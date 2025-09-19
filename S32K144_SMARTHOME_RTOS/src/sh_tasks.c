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

#include <stdio.h>
#include <string.h>


/************************ RTOS 객체 핸들러 정의 *******************/
QueueHandle_t g_command_queue;
QueueHandle_t g_sensor_data_queue;

SemaphoreHandle_t g_system_status_mutex;
SemaphoreHandle_t g_display_data_mutex;
SemaphoreHandle_t g_uart_mutex;

SemaphoreHandle_t g_button_interrupt_semaphore;

EventGroupHandle_t g_security_event_group;

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
static void __security_led_timer_callback(void);
static void __buzzer_timer_callback(TimerHandle_t xTimer);
static void __can_rx_callback(uint32_t id, uint8_t* data, uint8_t dlc);

/********************************************************************/
/************************ MainControlTask ***************************/
/********************************************************************/

static volatile bool g_is_buzzer_started;

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
                    g_is_buzzer_started = false;
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
                        SHH_Blinds_Move(200); // 정방향으로 20스텝
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
                         SHH_Blinds_Move(-200); // 역방향으로 20스텝
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
                g_is_buzzer_started = false;
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

static TimerHandle_t g_buzzer_timer = NULL;

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
    SHD_LPIT0_SetPeriodic(2, 200, __security_led_timer_callback);
    
    if (!g_is_buzzer_started) {
        SHH_Buzzer_StartAlarm();            // 부저 울림 시작 

        // 부저 타이머 시작 (2초 원샷)
        if (g_buzzer_timer == NULL) {
            g_buzzer_timer = xTimerCreate("BuzzerTimer",
                                        pdMS_TO_TICKS(3000), // 실제 약 2초
                                        pdFALSE,             // one-shot
                                        NULL,
                                        __buzzer_timer_callback);
        }

        if (g_buzzer_timer != NULL) {
            xTimerReset(g_buzzer_timer, 0);
        }
        g_is_buzzer_started = true;
    }
}

static void __security_led_timer_callback(void) {
    SHH_SecurityWarningLED_Toggle();
}

static void __buzzer_timer_callback(TimerHandle_t xTimer) {
    (void)xTimer;
    SHH_Buzzer_StopAlarm();   // 2초 뒤에 부저 끄기
}

static void _run_monitoring_mode_logic(const sensor_data_t* data) {

    // 자동 팬 제어 (온도 기반)
    if (data->temperature >= TEMP_THRESHOLD) {
        SHH_Fan_On();
    } else {
        SHH_Fan_Off();
    }

    // 자동 조명 제어 (밝기 기반)
    int8_t brightness_percent = (int8_t)((data->cds_raw * 100) / 4095);
    SHH_MainLight_SetBrightness(65 - brightness_percent);
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

    // 2. OLED 포맷팅 (현재 모드 기준)
    xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
    system_mode_t current_mode = g_system_status.current_mode;
    xSemaphoreGive(g_system_status_mutex);

    char temp_mode_str[20];
    char temp_device_str[20];

    switch(current_mode) {
        case MODE_MONITORING:
            snprintf(temp_mode_str, 20, "Mode: Monitoring");
            snprintf(temp_device_str, 20, "Selected:         ");
            break;
            case MODE_MANUAL:
            snprintf(temp_mode_str, 20, "Mode: Manual    ");
            switch(g_selected_device) {
                case DEVICE_SERVO: snprintf(temp_device_str, 20, "Selected: DoorLock"); break;
                case DEVICE_STEP:  snprintf(temp_device_str, 20, "Selected: Blinds  ");   break;
                case DEVICE_RELAY: snprintf(temp_device_str, 20, "Selected: ExtPower"); break;
            }
            break;
        case MODE_SECURITY:
            snprintf(temp_mode_str, 20, "Mode: Security  ");
            snprintf(temp_device_str, 20, "Selected:         ");
            break;
    } 

    xSemaphoreTake(g_display_data_mutex, portMAX_DELAY); // Mutex 획득
    for (int i = 0; i < 20; ++i) {
        g_display_data.oled_mode_str[i] = temp_mode_str[i];
        if (temp_mode_str[i] == '\0') {
            break;
        }
    }
    for (int i = 0; i < 20; ++i) {
        g_display_data.oled_device_str[i] = temp_device_str[i];
        if (temp_device_str[i] == '\0') {
            break;
        }
    }
    xSemaphoreGive(g_display_data_mutex); // Mutex 반환

}

/********************************************************************/
/**************************** SensorTask ****************************/
/********************************************************************/
void SH_Sensor_Task(void *pvParameters) {

    (void)pvParameters;
    sensor_data_t sensor_data_to_send;

    for (;;) {
        // 1. 주기적으로 실행
        vTaskDelay(pdMS_TO_TICKS(500));

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
    PORTE->ISFR = (1UL << 13) | (1UL << 14) | (1UL << 15) | (1UL << 16);

    // 만약 세마포어 전달로 인해 더 높은 우선순위의 태스크가 깨어났다면,
    // 즉시 컨텍스트 스위칭을 요청한다.
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/********************************************************************/
/**************************** CanCommTask ***************************/
/********************************************************************/
void SH_Can_Init(void) {
    // SHD_CAN0_RegisterRxCallback(__can_rx_callback);
    CAN0->IMASK1 |= (1 << 4);
}

void SH_CanComm_Task(void *pvParameters) {

    (void)pvParameters;
    SH_Can_Init();
    uint8_t can_data_buffer[8];

    // uint8_t test_buf[8] = {0};
    // uint32_t cnt = 0;

    for (;;) {

        // 1. 주기적으로 실행
        vTaskDelay(pdMS_TO_TICKS(1000));

        // // 송신 테스트
        // test_buf[0] = (uint8_t)(cnt & 0xFF);
        // test_buf[1] = (uint8_t)((cnt >> 8) & 0xFF);
        // SHD_CAN0_Transmit(0x123, test_buf, 2);
        // cnt++;

        // 수신 테스트
        SHD_CAN0_CheckRx();

        // // 2. 뮤텍스로 보호하며 현재 시스템 상태와 센서 데이터를 읽어옴
        // xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
        // system_status_t current_status = g_system_status;
        // sensor_data_t current_sensor_data = g_latest_sensor_data;
        // xSemaphoreGive(g_system_status_mutex);

        // // 3. 환경 데이터 CAN 메시지 전송
        // can_data_buffer[0] = current_sensor_data.temperature;
        // can_data_buffer[1] = current_sensor_data.humidity;
        // can_data_buffer[2] = (uint8_t)((current_sensor_data.cds_raw * 100) / 4095);
        // SHD_CAN0_Transmit(CAN_ID_STATUS_ENV, can_data_buffer, 3);

        // // 4. 시스템 상태 CAN 메시지 전송
        // can_data_buffer[0] = (uint8_t)current_status.current_mode;
        // can_data_buffer[1] = (uint8_t)current_status.is_alarm_active;
        // SHD_CAN0_Transmit(CAN_ID_STATUS_SYSTEM, can_data_buffer, 2);
    }
}

// static void __can_rx_callback(uint32_t id, uint8_t* data, uint8_t dlc) {

//     command_msg_t cmd_msg;
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//     switch(id) {

//         case CAN_ID_CMD_SET_MODE:
//             if (dlc > 0) {
//                 cmd_msg.command_id = CAN_ID_CMD_SET_MODE;
//                 cmd_msg.value = data[0]; // 모드 값
//                 xQueueSendFromISR(g_command_queue, &cmd_msg, &xHigherPriorityTaskWoken);
//             }
//             break;

//         case CAN_ID_CMD_ALARM_OFF:
//             cmd_msg.command_id = CAN_ID_CMD_ALARM_OFF;
//             cmd_msg.value = 0;
//             xQueueSendFromISR(g_command_queue, &cmd_msg, &xHigherPriorityTaskWoken);
//             break;
            
//         case CAN_ID_CMD_DEVICE_CTRL:
//             if (dlc > 1) {
//                 cmd_msg.command_id = CAN_ID_CMD_DEVICE_CTRL;
//                 // value에 장치 ID와 동작 값을 압축하여 전달
//                 cmd_msg.value = (int32_t)((data[1] << 8) | data[0]);
//                 xQueueSendFromISR(g_command_queue, &cmd_msg, &xHigherPriorityTaskWoken);
//             }
//             break;
//     }
    
//     // 만약 큐 전송으로 인해 더 높은 우선순위의 태스크가 깨어났다면,
//     // ISR 종료 후 즉시 컨텍스트 스위칭을 수행합니다.
//     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// }

/********************************************************************/
/**************************** DisplayTask & FNDScanTask**************/
/********************************************************************/
void SH_Display_Task(void *pvParameters) {

    (void)pvParameters;

    SHH_OLED_Clear();

    for (;;) {

        vTaskDelay(pdMS_TO_TICKS(1000));
        
        uint32_t temp_fnd_number;
        char temp_mode_str[20];
        char temp_device_str[20];

        xSemaphoreTake(g_display_data_mutex, portMAX_DELAY);  
        temp_fnd_number = g_display_data.fnd_number;
        for (int i = 0; i < 20; ++i) {
            temp_mode_str[i] = g_display_data.oled_mode_str[i];
            if (temp_mode_str[i] == '\0') {
                break;
            }
        }
        for (int i = 0; i < 20; ++i) {
            temp_device_str[i] = g_display_data.oled_device_str[i];
            if (temp_device_str[i] == '\0') {
                break;
            }
        }
        xSemaphoreGive(g_display_data_mutex); 

        // 최신 값 반영만 수행
        SHH_FND_BufferUpdate(temp_fnd_number);     
        
        SHH_OLED_SetCursor(0, 0);
        SHH_OLED_PrintString_5x8(temp_mode_str);
        SHH_OLED_SetCursor(0, 2); 
        SHH_OLED_PrintString_5x8(temp_device_str);

    }
}

void FND_Scan_Task(void *pvParameters) {
    (void)pvParameters;
    const TickType_t xPeriod = pdMS_TO_TICKS(2);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // 1. Mutex 없이 바로 버퍼에서 읽어서 스캔
        SHH_FND_Display();  

        // 2. 1ms 정확하게 대기
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}
/********************************************************************/
/************************ SecurityEventTask *************************/
/********************************************************************/
void SH_SecurityEvent_Task(void *pvParameters) {

    (void)pvParameters;

    for (;;) {
        // 데이터시트 권장사항(60ms 이상)에 따라 100ms마다 측정 사이클 실행
        vTaskDelay(pdMS_TO_TICKS(100));

        // 현재 시스템이 보안 모드일 때만 거리 측정 수행
        xSemaphoreTake(g_system_status_mutex, portMAX_DELAY);
        bool is_security_mode = (g_system_status.current_mode == MODE_SECURITY);
        xSemaphoreGive(g_system_status_mutex);

        if (is_security_mode) {
            SHH_uWave_StartMeasurement();

            uint16_t distance_cm = SHH_uWave_GetDistanceCm();
            SHH_Uart_Printf("distance: %d\r\n", distance_cm);

            if ((distance_cm != 0xFFFF) && (distance_cm < SECURITY_DISTANCE_THRESHOLD_CM)) {
                xEventGroupSetBits(g_security_event_group, 0x01);
            }
        }
    }
}

void PORTC_IRQHandler(void) {

    // Trig 핀이 초음파를 보내면 Echo 핀이 HIGH 상태가 되어 인터럽트 핸들러가 실행됨
    if ((PORTC->ISFR & (1UL << 13))) {
        SHH_uWave_Echo_ISR_Handler();

        // 발생한 모든 핀의 인터럽트 플래그를 클리어
        // 주의: 다른 PORTC 인터럽트 소스가 있다면, 해당 플래그만 선택적으로 클리어해야 함
        PORTC->ISFR = (1UL << 13);
    }

}