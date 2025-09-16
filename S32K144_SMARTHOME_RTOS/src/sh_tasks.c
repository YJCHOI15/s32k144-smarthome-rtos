#include "sh_tasks.h"
#include "sh_config.h"

#include "shh_led.h"
#include "shh_actuator.h"
#include "shh_display.h"
#include "shh_sensor.h"
#include "shh_sound.h"
#include "shh_system.h"
#include "shh_uart.h"

#include "drivers/lpit_driver.h"
#include "drivers/gpio_driver.h"
#include "drivers/flexcan_driver.h"

#include "timers.h"


/************************ RTOS 객체 핸들러 정의 *******************/
QueueHandle_t g_command_queue;
QueueHandle_t g_sensor_data_queue;

SemaphoreHandle_t g_system_status_mutex;
SemaphoreHandle_t g_display_data_mutex;
SemaphoreHandle_t g_uart_mutex;

SemaphoreHandle_t g_button_interrupt_semaphore;
SemaphoreHandle_t g_uWave_semaphore;

EventGroupHandle_t g_security_event_group;

TimerHandle_t g_fnd_timer;

/************************** 전역 변수 정의 ************************/
// 시스템의 현재 상태를 저장
static volatile system_status_t g_system_status;

// 가장 최근에 수신된 센서 데이터를 저장
static volatile sensor_data_t g_latest_sensor_data;

// 최근 데이터만 디스플레이에 반영
volatile display_data_t g_display_data;

// 수동 제어 모드에서 선택된 장치를 추적
typedef enum {
    DEVICE_SERVO,
    DEVICE_STEP,
    DEVICE_RELAY
} manual_device_t;
static volatile manual_device_t g_selected_device = DEVICE_SERVO;

/*************** 수신 이벤트 처리 내부 헬퍼 함수 선언 ***************/
static void _handle_command(command_msg_t* cmd);
static void _handle_sensor_data(sensor_data_t* data);
static void _handle_security_event(void);
static void _run_monitoring_mode_logic(const sensor_data_t* data);
static void _update_display(void);
static void _security_led_timer_callback(void);
static void _can_rx_callback(uint32_t id, uint8_t* data, uint8_t dlc);
static void vFndTimerCallback(TimerHandle_t xTimer);

/********************************************************************/
/************************ MainControlTask ***************************/
/********************************************************************/
void SH_MainControl_Task(void *pvParameters) {

    (void)pvParameters;

    QueueSetHandle_t event_queue_set;
    QueueSetMemberHandle_t activated_member;

    command_msg_t received_cmd;
    sensor_data_t received_sensor_data;

    // 1. 여러 큐를 한번에 기다리기 위한 Queue Set 생성
    event_queue_set = xQueueCreateSet(
        10 + 1 // Command Queue 크기 + Sensor Data Queue 크기
    );
    xQueueAddToSet(g_command_queue, event_queue_set);
    xQueueAddToSet(g_sensor_data_queue, event_queue_set);

    // 2. 초기 상태 설정 (모니터링 모드)
    xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
    g_system_status.current_mode = MODE_MONITORING;
    g_system_status.is_alarm_active = false;
    xSemaphoreGive(g_system_status_mutex);
    SHH_ModeLED_Set(MODE_MONITORING);

    for (;;) {

        // 3. Command 또는 Sensor 데이터가 큐에 들어오거나, 보안 이벤트가 발생할 때까지 Blocked
        activated_member = xQueueSelectFromSet(event_queue_set, pdMS_TO_TICKS(10)); // 10ms 타임아웃

        if (activated_member != NULL) {

            // 4a. Command Queue 이벤트 발생 (버튼, CAN 명령)
            if (activated_member == g_command_queue) {
                xQueueReceive(g_command_queue, &received_cmd, 0);
                _handle_command(&received_cmd);
            } 
            // 4b. Sensor Data Queue 이벤트 발생 (온습도 센서, 조도 센서)
            else if (activated_member == g_sensor_data_queue) {
                xQueueReceive(g_sensor_data_queue, &received_sensor_data, 0);
                _handle_sensor_data(&received_sensor_data);
            }
        }

        // 5. 보안 이벤트가 발생했는지 확인 (항상 체크)
        EventBits_t security_bits = xEventGroupGetBits(g_security_event_group);
        if (security_bits & 0x01) {
            _handle_security_event();
            xEventGroupClearBits(g_security_event_group, 0x01); // 이벤트 처리 후 플래그 클리어
        }
    }
}

static void _handle_command(command_msg_t* cmd) {

    // 현재 시스템 상태 읽어옴
    xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
    system_mode_t current_mode = g_system_status.current_mode;
    bool is_alarm_active = g_system_status.is_alarm_active;
    xSemaphoreGive(g_system_status_mutex);

    switch (cmd->command_id) {

        case CMD_BTN1_CYCLE_MODE:
    
            // 보안 경고가 활성화 상태가 아닐 때만 모드 변경 가능
            if (!is_alarm_active) {
                xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
                if (current_mode == MODE_MONITORING) {
                    g_system_status.current_mode = MODE_MANUAL;
                    SHH_SecurityStandbyLED_Off();
                } else if (current_mode == MODE_MANUAL) {
                    g_system_status.current_mode = MODE_SECURITY;
                    SHH_SecurityStandbyLED_On(); 
                } else if (current_mode == MODE_SECURITY) {
                    g_system_status.current_mode = MODE_MONITORING;
                    SHH_SecurityStandbyLED_Off();
                }
                SHH_ModeLED_Set(g_system_status.current_mode);
                xSemaphoreGive(g_system_status_mutex);
                _update_display();
            }
            break;

        case CMD_BTN2_SELECT_DEVICE:
            if (current_mode == MODE_MANUAL) {
                g_selected_device = (manual_device_t)(((int)g_selected_device + 1) % 3);
                _update_display();
            }
            break;

        case CMD_BTN3_DEVICE_ACTION_POSITIVE:
            if (current_mode == MODE_MANUAL) {
                switch(g_selected_device) {
                    case DEVICE_SERVO:
                        SHH_DoorLock_Open();
                        break;
                    case DEVICE_STEP:
                        SHH_Blinds_Move(20); // 정방향으로 20스텝
                        break;
                    case DEVICE_RELAY:
                        SHH_ExternalPower_On();
                        break;
                }
            }
            break;

        case CMD_BTN4_DEVICE_ACTION_NEGATIVE:
             if (current_mode == MODE_MANUAL) {
                 switch(g_selected_device) {
                     case DEVICE_SERVO:
                         SHH_DoorLock_Close();
                         break;
                     case DEVICE_STEP:
                         SHH_Blinds_Move(-20); // 역방향으로 20스텝
                         break;
                     case DEVICE_RELAY:
                         SHH_ExternalPower_Off();
                         break;
                 }
             }
            break;

        case CMD_CAN_SET_MODE:
            if (cmd->value >= MODE_MONITORING && cmd->value <= MODE_SECURITY) {
                xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
                g_system_status.current_mode = (system_mode_t)cmd->value;
                // CAN 명령으로 모드 변경 시 알람 상태도 해제
                g_system_status.is_alarm_active = false; 
                SHH_Buzzer_StopAlarm();
                SHH_SecurityWarningLED_Off();
                SHH_ModeLED_Set(g_system_status.current_mode);
                xSemaphoreGive(g_system_status_mutex);
            }
            break;

        case CMD_CAN_ALARM_OFF:
            xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
            g_system_status.is_alarm_active = false;
            // 알람만 끄고 모드는 보안 모드로 유지
            SHH_Buzzer_StopAlarm();
            SHD_LPIT0_Stop(2);  // 경고 LED4, 5, 6 점멸 타이머 중지
            SHH_SecurityWarningLED_Off();
            xSemaphoreGive(g_system_status_mutex);
            break;

        case CMD_CAN_CONTROL_DEVICE:
        {
            // cmd->value에서 장치 ID와 동작 값을 파싱
            uint8_t device_id = (cmd->value) & 0xFF;       
            int32_t action_value = (cmd->value) >> 8;   

            switch(device_id) {
                case 1: // DEVICE_SERVO
                    if (action_value == 1) SHH_DoorLock_Open();
                    else SHH_DoorLock_Close();
                    break;
                case 2: // DEVICE_STEP
                    SHH_Blinds_Move(action_value); 
                    break;
                case 3: // DEVICE_RELAY
                    if (action_value == 1) SHH_ExternalPower_On();
                    else SHH_ExternalPower_Off();
                    break;
            }
            break;
        }

        default:
            break;
    }
}

static void _handle_sensor_data(sensor_data_t* data) {

    // 최신 센서 데이터 업데이트
    g_latest_sensor_data = *data;

    /************************ debug uart **************************/
    // uint8_t brightness_percent = (uint8_t)((data->cds_raw * 100) / 4095);
    // uint8_t vr_percent = (uint8_t)((data->vr_raw * 100) / 4095);
    // SHH_Uart_Printf("[_handle_sensor_data] Temp: %dC, Humi: %d%%, Bright: %d%%, VR: %d%%\r\n",
    //                 data->temperature,
    //                 data->humidity,
    //                 brightness_percent,
    //                 vr_percent);
    /***************************************************************/

    xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
    system_mode_t current_mode = g_system_status.current_mode;
    xSemaphoreGive(g_system_status_mutex);

    // 현재 모드에 따라 로직 수행
    if (current_mode == MODE_MONITORING) {
        _run_monitoring_mode_logic(data);
    }
    else if (current_mode == MODE_MANUAL) {
        uint8_t brightness_percent = (uint8_t)((data->vr_raw * 100) / 4095);
        SHH_MainLight_SetBrightness(brightness_percent);
    }

    // 센서 데이터 업데이트 완료 -> FND, OLED 업데이트
    _update_display();
}

static void _handle_security_event(void) {

    xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
    system_mode_t current_mode = g_system_status.current_mode;
    bool is_alarm_active = g_system_status.is_alarm_active;
    xSemaphoreGive(g_system_status_mutex);

    // 보안 모드가 아니거나, 이미 알람이 울리고 있다면 아무것도 하지 않음
    if ((current_mode != MODE_SECURITY) || is_alarm_active) {
        return;
    }

    // 알람 상태로 전환
    xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
    g_system_status.is_alarm_active = true;
    xSemaphoreGive(g_system_status_mutex);

    // 시청각적 경고 시작
    SHH_SecurityStandbyLED_Off();       // 대기 LED2 끄기
    SHD_LPIT0_SetPeriodic(2, 1000, _security_led_timer_callback);
    SHH_Buzzer_StartAlarm();            // 부저 울림 시작 

}

static void _security_led_timer_callback(void) {
    SHH_SecurityWarningLED_Toggle();
}

static void _run_monitoring_mode_logic(const sensor_data_t* data) {

    // 자동 팬 제어 (온도 기반)
    if (data->temperature > TEMP_THRESHOLD) {
        SHH_Fan_On();
    } else {
        SHH_Fan_Off();
    }

    // 자동 조명 제어 (밝기 기반)
    uint8_t brightness_percent = (uint8_t)((data->cds_raw * 100) / 4095);
    SHH_MainLight_SetBrightness(100 - brightness_percent);
}

static void _update_display(void) {

    // 1. FND 문자열 포맷팅
    uint8_t brightness = (g_latest_sensor_data.cds_raw * 100) / 4095;
    if (brightness == 100) brightness = 99;

    xSemaphoreTake(g_display_data_mutex, portMAX_DELAY);   
    g_display_data.fnd_number = (g_latest_sensor_data.temperature * 10000) 
        + (g_latest_sensor_data.humidity * 100) 
        + brightness;
    xSemaphoreGive(g_display_data_mutex);  

    // // 2. OLED 포맷팅 (현재 모드 기준)
    // xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
    // system_mode_t current_mode = g_system_status.current_mode;
    // xSemaphoreGive(g_system_status_mutex);

    // if (current_mode == MODE_MANUAL) {

    //     switch(g_selected_device) {
    //         case DEVICE_SERVO: snprintf(msg.oled_string, 20, "Selected: DoorLock"); break;
    //         case DEVICE_STEP:  snprintf(msg.oled_string, 20, "Selected: Blinds");   break;
    //         case DEVICE_RELAY: snprintf(msg.oled_string, 20, "Selected: ExtPower"); break;
    //     }
    // } else {
    //     snprintf(msg.oled_string, 20, "Mode: %s", (current_mode == MODE_MONITORING) ? "Monitoring" : "Security");
    // }

}

/********************************************************************/
/**************************** SensorTask ****************************/
/********************************************************************/
void SH_Sensor_Task(void *pvParameters) {

    (void)pvParameters;
    sensor_data_t sensor_data_to_send;

    for (;;) {
        // 1. 주기적으로 실행
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 2. HAL 함수를 호출하여 모든 센서 값을 읽어온다.
        sensor_data_to_send.temperature = SHH_ReadTemperature();
        sensor_data_to_send.humidity    = SHH_ReadHumidity();

        // HAL 함수가 0-100% 값을 반환하므로, 0-4095 범위의 raw 값으로 역산한다.
        uint8_t cds_percent = SHH_ReadBrightnessSensor();
        sensor_data_to_send.cds_raw = (uint16_t)((cds_percent * 4095) / 100);

        uint8_t vr_percent = SHH_ReadManualControlVr();
        sensor_data_to_send.vr_raw = (uint16_t)((vr_percent * 4095) / 100);
        
        // 3. 수집된 데이터를 Sensor Data Queue로 전송한다.
        // xQueueSend는 큐에 공간이 생길 때까지 기다리지 않고 즉시 반환될 수 있다.
        // 이 경우 큐가 꽉 차지 않았다고 가정한다.
        xQueueSend(g_sensor_data_queue, &sensor_data_to_send, 0);
    }
}

/********************************************************************/
/************************ ButtonInputTask ***************************/
/********************************************************************/
void SH_ButtonInput_Task(void *pvParameters) {

    (void)pvParameters;
    command_msg_t cmd_msg;
    
    for (;;) {
        // 1. 버튼 인터럽트가 발생할 때까지 Blocked
        if (xSemaphoreTake(g_button_interrupt_semaphore, portMAX_DELAY) == pdTRUE) {
            // 2. 채터링 방지를 위한 대기
            vTaskDelay(pdMS_TO_TICKS(50));

            // 3. 어떤 버튼이 눌렸는지 확인하고, 해당하는 명령을 생성합니다.
            if (SHD_GPIO_ReadPin(PIN_BTN1) == 0) {
                cmd_msg.command_id = CMD_BTN1_CYCLE_MODE;
                xQueueSend(g_command_queue, &cmd_msg, 0);
                SHH_Piezo_Beep();
                // 버튼이 떼어질 때까지 기다려 다중 입력을 방지
                while(SHD_GPIO_ReadPin(PIN_BTN1) == 0) { vTaskDelay(pdMS_TO_TICKS(20)); }
            } else if (SHD_GPIO_ReadPin(PIN_BTN2) == 0) {
                cmd_msg.command_id = CMD_BTN2_SELECT_DEVICE;
                xQueueSend(g_command_queue, &cmd_msg, 0);
                SHH_Piezo_Beep();
                while(SHD_GPIO_ReadPin(PIN_BTN2) == 0) { vTaskDelay(pdMS_TO_TICKS(20)); }
            } else if (SHD_GPIO_ReadPin(PIN_BTN3) == 0) {
                cmd_msg.command_id = CMD_BTN3_DEVICE_ACTION_POSITIVE;
                xQueueSend(g_command_queue, &cmd_msg, 0);
                SHH_Piezo_Beep();
                while(SHD_GPIO_ReadPin(PIN_BTN3) == 0) { vTaskDelay(pdMS_TO_TICKS(20)); }
            } else if (SHD_GPIO_ReadPin(PIN_BTN4) == 0) {
                cmd_msg.command_id = CMD_BTN4_DEVICE_ACTION_NEGATIVE;
                xQueueSend(g_command_queue, &cmd_msg, 0);
                SHH_Piezo_Beep();
                while(SHD_GPIO_ReadPin(PIN_BTN4) == 0) { vTaskDelay(pdMS_TO_TICKS(20)); }
            }
        }
    }
}

void PORTE_IRQHandler(void) {

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 인터럽트가 발생했음을 ButtonInput_Task에 알린다 (세마포어 전달).
    xSemaphoreGiveFromISR(g_button_interrupt_semaphore, &xHigherPriorityTaskWoken);

    // 버튼 핀만 클리어
    PORTE->ISFR = (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16);

    // 만약 세마포어 전달로 인해 더 높은 우선순위의 태스크가 깨어났다면,
    // 즉시 컨텍스트 스위칭을 요청한다.
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/********************************************************************/
/**************************** CanCommTask ***************************/
/********************************************************************/
void SH_Can_Init(void) {
    SHD_CAN0_RegisterRxCallback(_can_rx_callback);
}

void SH_CanComm_Task(void *pvParameters) {

    (void)pvParameters;
    uint8_t can_data_buffer[8];

    for (;;) {

        // 1. 500ms마다 주기적으로 실행
        vTaskDelay(pdMS_TO_TICKS(500));

        // 2. 뮤텍스로 보호하며 현재 시스템 상태와 센서 데이터를 읽어옴
        xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
        system_status_t current_status = g_system_status;
        sensor_data_t current_sensor_data = g_latest_sensor_data;
        xSemaphoreGive(g_system_status_mutex);

        // 아래 코드 주석 -> FND 됨, 버튼 안됨 
        // 아래 코드 주석 해제 -> FND 안됨, 버튼 잘됨
        // 3. 환경 데이터 CAN 메시지 전송
        can_data_buffer[0] = current_sensor_data.temperature;
        can_data_buffer[1] = current_sensor_data.humidity;
        can_data_buffer[2] = (uint8_t)(current_sensor_data.cds_raw >> 8);
        can_data_buffer[3] = (uint8_t)(current_sensor_data.cds_raw & 0xFF);
        SHD_CAN0_Transmit(CAN_ID_STATUS_ENV, can_data_buffer, 4);

        // 4. 시스템 상태 CAN 메시지 전송
        can_data_buffer[0] = (uint8_t)current_status.current_mode;
        can_data_buffer[1] = (uint8_t)current_status.is_alarm_active;
        SHD_CAN0_Transmit(CAN_ID_STATUS_SYSTEM, can_data_buffer, 2);
    }
}

static void _can_rx_callback(uint32_t id, uint8_t* data, uint8_t dlc) {

    command_msg_t cmd_msg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    switch(id) {

        case CAN_ID_CMD_SET_MODE:
            if (dlc > 0) {
                cmd_msg.command_id = CAN_ID_CMD_SET_MODE;
                cmd_msg.value = data[0]; // 모드 값
                xQueueSendFromISR(g_command_queue, &cmd_msg, &xHigherPriorityTaskWoken);
            }
            break;

        case CAN_ID_CMD_ALARM_OFF:
            cmd_msg.command_id = CAN_ID_CMD_ALARM_OFF;
            cmd_msg.value = 0;
            xQueueSendFromISR(g_command_queue, &cmd_msg, &xHigherPriorityTaskWoken);
            break;
            
        case CAN_ID_CMD_DEVICE_CTRL:
            if (dlc > 1) {
                cmd_msg.command_id = CAN_ID_CMD_DEVICE_CTRL;
                // value에 장치 ID와 동작 값을 압축하여 전달
                cmd_msg.value = (int32_t)((data[1] << 8) | data[0]);
                xQueueSendFromISR(g_command_queue, &cmd_msg, &xHigherPriorityTaskWoken);
            }
            break;
    }
    
    // 만약 큐 전송으로 인해 더 높은 우선순위의 태스크가 깨어났다면,
    // ISR 종료 후 즉시 컨텍스트 스위칭을 수행합니다.
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/********************************************************************/
/**************************** DisplayTask ***************************/
/********************************************************************/
void SH_Display_Task(void *pvParameters) {

    (void)pvParameters;

    // FND 스캔용 소프트웨어 타이머 생성 (주기: 1ms) -> OLED도 쓸지 미정
    g_fnd_timer = xTimerCreate(
        "FndTimer",                   // 타이머 이름
        pdMS_TO_TICKS(1),             // 주기 (1ms)
        pdTRUE,                       // 자동 재로드 (계속 반복)
        (void*)0,                     // 타이머 ID (사용 안함)
        vFndTimerCallback             // 콜백 함수
    );

    if (g_fnd_timer != NULL) {
        xTimerStart(g_fnd_timer, 0);
    }

    for (;;) {
        // 최신 값 반영만 수행
        xSemaphoreTake(g_display_data_mutex, portMAX_DELAY);  
        SHH_FND_BufferUpdate(g_display_data.fnd_number);     
        xSemaphoreGive(g_display_data_mutex);              

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void vFndTimerCallback(TimerHandle_t xTimer) {
    (void)xTimer;
    SHH_FND_Display();   // 1자릿수 스캔
}

/********************************************************************/
/************************ SecurityEventTask *************************/
/********************************************************************/
void SH_SecurityEvent_Task(void *pvParameters) {

    (void)pvParameters;

    for (;;) {
        // 1. 데이터시트 권장사항(60ms 이상)에 따라 100ms마다 측정 사이클 실행
        vTaskDelay(pdMS_TO_TICKS(100));

        // 2. 현재 시스템이 보안 모드일 때만 거리 측정 수행
        xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
        bool is_security_mode = (g_system_status.current_mode == MODE_SECURITY);
        xSemaphoreGive(g_system_status_mutex);

        if (is_security_mode)
        {
            // 2. 거리 측정 시작 (이 함수는 즉시 반환됨)
            SHH_uWave_StartMeasurement();

            // 3. 측정 완료 인터럽트를 기다림 (최대 50ms 타임아웃)
            if (xSemaphoreTake(g_uWave_semaphore, pdMS_TO_TICKS(50)) == pdTRUE) {

                // 4. 신호 수신 시 거리 값을 가져와서 판단
                uint16_t distance_cm = SHH_uWave_GetDistanceCm();

                if ((distance_cm != 0xFFFF) && (distance_cm < SECURITY_DISTANCE_THRESHOLD_CM)) {
                    xEventGroupSetBits(g_security_event_group, 0x01);
                }
            }
        }
    }
}

void PORTC_IRQHandler(void) {

    // Echo 핀에서 인터럽트가 발생했는지 확인
    if ((PORTC->ISFR & (1UL << 13))) {
        
        // HAL에 있는 핸들러 호출
        SHH_uWave_Echo_ISR_Handler();
    }

    // 발생한 모든 핀의 인터럽트 플래그를 클리어
    // 주의: 다른 PORTC 인터럽트 소스가 있다면, 해당 플래그만 선택적으로 클리어해야 함
    PORTC->ISFR = 0xFFFFFFFF;
}