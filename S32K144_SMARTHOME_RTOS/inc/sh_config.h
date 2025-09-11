#ifndef SH_CONFIG_H
#define SH_CONFIG_H

#include "drivers/port_driver.h"

/********************** Pin Mapping *************************/

#define PIN_LPUART1_RX                   ((port_pin_t){PORT_C, 6})
#define PIN_LPUART1_TX                   ((port_pin_t){PORT_C, 7})

#define PIN_CAN0_RX                      ((port_pin_t){PORT_E, 4})
#define PIN_CAN0_TX                      ((port_pin_t){PORT_E, 5})

#define PIN_LPI2C0_SDA                   ((port_pin_t){PORT_A, 2})
#define PIN_LPI2C0_SCL                   ((port_pin_t){PORT_A, 3})

#define PIN_ADC0_SE4_CDS                 ((port_pin_t){PORT_B, 0})
#define PIN_ADC0_SE5_VR                  ((port_pin_t){PORT_B, 1})

#define PIN_UWAVE_TRIG                   ((port_pin_t){PORT_C, 12})
#define PIN_UWAVE_ECHO                   ((port_pin_t){PORT_C, 13})

#define PIN_BTN1                         ((port_pin_t){PORT_E, 13})
#define PIN_BTN2                         ((port_pin_t){PORT_E, 14})
#define PIN_BTN3                         ((port_pin_t){PORT_E, 15})
#define PIN_BTN4                         ((port_pin_t){PORT_E, 16})

#define PIN_FTM0_CH1_LED8                ((port_pin_t){PORT_C, 1})
#define PIN_FTM0_CH2_SERVO               ((port_pin_t){PORT_C, 2})

#define PIN_LED_RED                      ((port_pin_t){PORT_D, 15})
#define PIN_LED_GREEN                    ((port_pin_t){PORT_D, 16})
#define PIN_LED_BLUE                     ((port_pin_t){PORT_D, 0})

#define PIN_LED1                         ((port_pin_t){PORT_D, 3})
#define PIN_LED2                         ((port_pin_t){PORT_D, 5})
#define PIN_LED4                         ((port_pin_t){PORT_D, 10})
#define PIN_LED5                         ((port_pin_t){PORT_D, 11})
#define PIN_LED6                         ((port_pin_t){PORT_D, 12})

#define PIN_FND_DATA1                    ((port_pin_t){PORT_B, 8})
#define PIN_FND_DATA2                    ((port_pin_t){PORT_B, 9})
#define PIN_FND_DATA3                    ((port_pin_t){PORT_B, 10})
#define PIN_FND_DATA4                    ((port_pin_t){PORT_B, 11})
#define PIN_FND_DATA5                    ((port_pin_t){PORT_C, 3})
#define PIN_FND_DATA6                    ((port_pin_t){PORT_C, 10})
#define PIN_FND_DATA7                    ((port_pin_t){PORT_C, 11})

#define PIN_FND_SEL1                     ((port_pin_t){PORT_B, 2})
#define PIN_FND_SEL2                     ((port_pin_t){PORT_B, 3})
#define PIN_FND_SEL3                     ((port_pin_t){PORT_B, 4})
#define PIN_FND_SEL4                     ((port_pin_t){PORT_B, 5})
#define PIN_FND_SEL5                     ((port_pin_t){PORT_D, 13})
#define PIN_FND_SEL6                     ((port_pin_t){PORT_D, 14})

#define PIN_PIEZO                        ((port_pin_t){PORT_C, 8})
#define PIN_BUZZER                       ((port_pin_t){PORT_C, 9})

#define PIN_DC_MOTOR                     ((port_pin_t){PORT_D, 8})

#define PIN_STEP_MOTOR1                  ((port_pin_t){PORT_B, 14})
#define PIN_STEP_MOTOR2                  ((port_pin_t){PORT_B, 15})
#define PIN_STEP_MOTOR3                  ((port_pin_t){PORT_B, 16})
#define PIN_STEP_MOTOR4                  ((port_pin_t){PORT_B, 17})

#define PIN_RELAY                        ((port_pin_t){PORT_D, 9})

#define PIN_TEMP_HUMI                    ((port_pin_t){PORT_D, 6})


/**************** SmartHome Data Structure *****************/

/* 시스템 모드를 나타내는 열거형 */
typedef enum {
    MODE_MONITORING,
    MODE_MANUAL,
    MODE_SECURITY
} system_mode_t;

/* Command Queue를 통해 전달될 메시지 구조체 */
typedef struct {
    uint8_t command_id; // 예: CMD_CYCLE_MODE, CMD_SELECT_DEVICE
    int32_t value;      // 추가 데이터
} command_msg_t;

/* Sensor Data Queue를 통해 전달될 메시지 구조체 */
typedef struct {
    float temperature;
    float humidity;
    uint16_t cds_raw; // 0-4095
    uint16_t vr_raw;  // 0-4095
} sensor_data_t;

/* Display Data Queue를 통해 전달될 메시지 구조체 */
typedef struct {
    char fnd_string[7]; // "TT:HH:BB" 형식
    char oled_line1[20];
    char oled_line2[20];
} display_data_t;

/* 공유 데이터: 여러 태스크가 접근하는 시스템의 현재 상태 */
typedef struct {
    system_mode_t current_mode;
    // ... 기타 공유가 필요한 상태 변수들
} system_status_t;


/********************* I2C Slave Adddress ******************/

#define SENSOR_OLED_ADDR 0x3C    // 또는 0x3D

/**************** RTOS & Application Configuration ****************/
// (이곳에 큐 사이즈, 태스크 우선순위 등 다른 설정값들도 정의할 수 있습니다)


#endif /* SH_CONFIG_H */