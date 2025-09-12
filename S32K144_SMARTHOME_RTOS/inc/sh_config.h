#ifndef SH_CONFIG_H
#define SH_CONFIG_H

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>
#include <stdint.h>
#include <stdbool.h>

#include "drivers/port_driver.h"


/********************** Pin Mapping *************************/
#define PIN_LPUART1_RX                   ((sh_port_pin_t){PORT_C, 6})
#define PIN_LPUART1_TX                   ((sh_port_pin_t){PORT_C, 7})

#define PIN_CAN0_RX                      ((sh_port_pin_t){PORT_E, 4})
#define PIN_CAN0_TX                      ((sh_port_pin_t){PORT_E, 5})

#define PIN_LPI2C0_SDA                   ((sh_port_pin_t){PORT_A, 2})
#define PIN_LPI2C0_SCL                   ((sh_port_pin_t){PORT_A, 3})

#define PIN_ADC0_SE4_CDS                 ((sh_port_pin_t){PORT_B, 0})
#define PIN_ADC0_SE5_VR                  ((sh_port_pin_t){PORT_B, 1})

#define PIN_UWAVE_TRIG                   ((sh_port_pin_t){PORT_C, 12})
#define PIN_UWAVE_ECHO                   ((sh_port_pin_t){PORT_C, 13})

#define PIN_BTN1                         ((sh_port_pin_t){PORT_E, 13})
#define PIN_BTN2                         ((sh_port_pin_t){PORT_E, 14})
#define PIN_BTN3                         ((sh_port_pin_t){PORT_E, 15})
#define PIN_BTN4                         ((sh_port_pin_t){PORT_E, 16})

#define PIN_FTM0_CH1_LED8                ((sh_port_pin_t){PORT_C, 1})
#define PIN_FTM0_CH2_SERVO               ((sh_port_pin_t){PORT_C, 2})

#define PIN_LED_RED                      ((sh_port_pin_t){PORT_D, 15})
#define PIN_LED_GREEN                    ((sh_port_pin_t){PORT_D, 16})
#define PIN_LED_BLUE                     ((sh_port_pin_t){PORT_D, 0})

#define PIN_LED1                         ((sh_port_pin_t){PORT_D, 3})
#define PIN_LED2                         ((sh_port_pin_t){PORT_D, 5})
#define PIN_LED4                         ((sh_port_pin_t){PORT_D, 10})
#define PIN_LED5                         ((sh_port_pin_t){PORT_D, 11})
#define PIN_LED6                         ((sh_port_pin_t){PORT_D, 12})

#define PIN_FND_DATA_A                   ((sh_port_pin_t){PORT_B, 8})
#define PIN_FND_DATA_B                   ((sh_port_pin_t){PORT_B, 9})
#define PIN_FND_DATA_C                   ((sh_port_pin_t){PORT_B, 10})
#define PIN_FND_DATA_D                   ((sh_port_pin_t){PORT_B, 11})
#define PIN_FND_DATA_E                   ((sh_port_pin_t){PORT_C, 3})
#define PIN_FND_DATA_F                   ((sh_port_pin_t){PORT_C, 10})
#define PIN_FND_DATA_G                   ((sh_port_pin_t){PORT_C, 11})

#define PIN_FND_SEL1                     ((sh_port_pin_t){PORT_B, 2})
#define PIN_FND_SEL2                     ((sh_port_pin_t){PORT_B, 3})
#define PIN_FND_SEL3                     ((sh_port_pin_t){PORT_B, 4})
#define PIN_FND_SEL4                     ((sh_port_pin_t){PORT_B, 5})
#define PIN_FND_SEL5                     ((sh_port_pin_t){PORT_D, 13})
#define PIN_FND_SEL6                     ((sh_port_pin_t){PORT_D, 14})

#define PIN_PIEZO                        ((sh_port_pin_t){PORT_C, 8})
#define PIN_BUZZER                       ((sh_port_pin_t){PORT_C, 9})

#define PIN_DC_MOTOR                     ((sh_port_pin_t){PORT_D, 8})

#define PIN_STEP_MOTOR1                  ((sh_port_pin_t){PORT_B, 14})
#define PIN_STEP_MOTOR2                  ((sh_port_pin_t){PORT_B, 15})
#define PIN_STEP_MOTOR3                  ((sh_port_pin_t){PORT_B, 16})
#define PIN_STEP_MOTOR4                  ((sh_port_pin_t){PORT_B, 17})

#define PIN_RELAY                        ((sh_port_pin_t){PORT_D, 9})

#define PIN_TEMP_HUMI                    ((sh_port_pin_t){PORT_D, 6})


/**************** SmartHome Data Structure *****************/
/* 시스템 모드를 나타내는 열거형 */
typedef enum {
    MODE_MONITORING,
    MODE_MANUAL,
    MODE_SECURITY
} system_mode_t;

/* Command Queue를 통해 전달될 명령 ID */
typedef enum {
    CMD_NULL = 0,
    /* 버튼 입력 명령 */
    CMD_BTN1_CYCLE_MODE,
    CMD_BTN2_SELECT_DEVICE,
    CMD_BTN3_DEVICE_ACTION_POSITIVE,
    CMD_BTN4_DEVICE_ACTION_NEGATIVE,
    /* CAN 원격 명령 */
    CMD_CAN_SET_MODE,
    CMD_CAN_CONTROL_DEVICE,
    CMD_CAN_ALARM_OFF
} command_id_t;

/* Command Queue를 통해 전달될 메시지 구조체 */
typedef struct {
    command_id_t command_id; 
    int32_t value;      
        /**
         * CMD_CAN_SET_MODE:
         * 0: MODE_MONITORING, 1: MODE_MANUAL, 2: MODE_SECURITY
         */
        /**
         * CMD_CAN_CONTROL_DEVICE:
         * 하위 8비트 (0-7): 제어할 장치 ID (1=서보, 2=스텝, 3=릴레이)
         * 상위 24비트 (8-31): 동작 값 (0=OFF/닫힘, 1=ON/열림, 스텝 모터의 경우 이동할 스텝 수)
         */
} command_msg_t;

/* Sensor Data Queue를 통해 전달될 메시지 구조체 */
#define TEMP_THRESHOLD   28  // FAN 작동 온도
typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    uint16_t cds_raw; // 0-4095
    uint16_t vr_raw;  // 0-4095
} sensor_data_t;

/* 초음파 감지 거리 */
#define SECURITY_DISTANCE_THRESHOLD_CM 50

/* Display Data Queue를 통해 전달될 메시지 구조체 */
typedef struct {
    char fnd_string[7]; // "TT:HH:BB" 형식
    char oled_string[20]; // 수동 -> 현재 장치, 그외 -> 모드 표시
} display_data_t;

/* 공유 데이터: 여러 태스크가 접근하는 시스템의 현재 상태 */
typedef struct {
    system_mode_t current_mode;
    bool is_alarm_active; // 보안 경고 활성화 여부
} system_status_t;


/********************* I2C Slave Adddress ******************/
#define SSD1306_OLED_ADDR 0x3C    // 또는 0x3D

/********************* RTOS Object Handler ******************/
extern QueueHandle_t g_command_queue;
extern QueueHandle_t g_sensor_data_queue;
extern QueueHandle_t g_display_data_queue;
extern SemaphoreHandle_t g_system_status_mutex;
extern SemaphoreHandle_t g_button_interrupt_semaphore;
extern SemaphoreHandle_t g_uWave_semaphore;
extern EventGroupHandle_t g_security_event_group;


#endif /* SH_CONFIG_H */