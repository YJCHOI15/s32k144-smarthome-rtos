#include "sh_config.h" 

#include "shh_system.h"
#include "shh_display.h"
#include "shh_led.h"
#include "shh_display.h"

#include "drivers/system_init.h"
#include "drivers/port_driver.h"
#include "drivers/gpio_driver.h"
#include "drivers/adc_driver.h"
#include "drivers/ftm_driver.h"
#include "drivers/lpi2c_driver.h"
#include "drivers/flexcan_driver.h"
#include "drivers/lpit_driver.h"
#include "drivers/lpuart_driver.h"
#include "drivers/sh_it_manager.h"

void SHH_Init(void)
{
    /* 1. 코어 및 시스템 클럭 초기화 */
    SHD_System_Init();

    /* 2. 각 PORT 모듈의 클럭 활성화 */
    SHD_PORT_Init();

    /* 3. 핀맵에 따른 핀 기능(Mux) 설정 */
    SHD_PORT_SetPinMux(PIN_LPUART1_RX, PORT_MUX_ALT_2);
    SHD_PORT_SetPinMux(PIN_LPUART1_TX, PORT_MUX_ALT_2);

    SHD_PORT_SetPinMux(PIN_CAN0_RX, PORT_MUX_ALT_4);
    SHD_PORT_SetPinMux(PIN_CAN0_TX, PORT_MUX_ALT_5);

    SHD_PORT_SetPinMux(PIN_LPI2C0_SDA, PORT_MUX_ALT_3);
    SHD_PORT_SetPinMux(PIN_LPI2C0_SCL, PORT_MUX_ALT_3);

    SHD_PORT_SetPinMux(PIN_ADC0_SE4_CDS, PORT_MUX_ANALOG);
    SHD_PORT_SetPinMux(PIN_ADC0_SE5_VR, PORT_MUX_ANALOG);

    SHD_PORT_SetPinMux(PIN_UWAVE_TRIG, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_UWAVE_ECHO, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_BTN1, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_BTN2, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_BTN3, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_BTN4, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_FTM0_CH6_LED8, PORT_MUX_ALT_2);
    SHD_PORT_SetPinMux(PIN_FTM0_CH2_SERVO, PORT_MUX_ALT_2);

    SHD_PORT_SetPinMux(PIN_LED_RED, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_LED_GREEN, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_LED_BLUE, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_LED1, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_LED2, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_LED4, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_LED5, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_LED6, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_FND_DATA_A, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_DATA_B, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_DATA_C, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_DATA_D, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_DATA_E, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_DATA_F, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_DATA_G, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_FND_SEL1, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_SEL2, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_SEL3, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_SEL4, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_SEL5, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_FND_SEL6, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_PIEZO, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_BUZZER, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_DC_MOTOR, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_STEP_MOTOR1, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_STEP_MOTOR2, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_STEP_MOTOR3, PORT_MUX_GPIO);
    SHD_PORT_SetPinMux(PIN_STEP_MOTOR4, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_RELAY, PORT_MUX_GPIO);

    SHD_PORT_SetPinMux(PIN_TEMP_HUMI, PORT_MUX_GPIO);

    /* 4. 각 주변장치 드라이버 초기화 및 GPIO 방향 설정 */

    // 온습도 센서는 입출력 번갈아가며 작동하므로 초기화 하지 않음
    // SHD_GPIO_InitPin(PIN_TEMP_HUMI, GPIO_INPUT);    
    // SHD_GPIO_InitPin(PIN_TEMP_HUMI, GPIO_OUTPUT);

    // GPIO 입력 핀 초기화
    SHD_GPIO_InitPin(PIN_UWAVE_ECHO, GPIO_INPUT);
    SHD_GPIO_InitPin(PIN_BTN1, GPIO_INPUT);
    SHD_GPIO_InitPin(PIN_BTN2, GPIO_INPUT);
    SHD_GPIO_InitPin(PIN_BTN3, GPIO_INPUT);
    SHD_GPIO_InitPin(PIN_BTN4, GPIO_INPUT);

    // GPIO 출력 핀 초기화
    SHD_GPIO_InitPin(PIN_UWAVE_TRIG, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_LED_RED, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_LED_GREEN, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_LED_BLUE, GPIO_OUTPUT);

    SHD_GPIO_InitPin(PIN_LED1, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_LED2, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_LED4, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_LED5, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_LED6, GPIO_OUTPUT);

    SHD_GPIO_InitPin(PIN_FND_DATA_A, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_DATA_B, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_DATA_C, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_DATA_D, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_DATA_E, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_DATA_F, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_DATA_G, GPIO_OUTPUT);

    SHD_GPIO_InitPin(PIN_FND_SEL1, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_SEL2, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_SEL3, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_SEL4, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_SEL5, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_FND_SEL6, GPIO_OUTPUT);

    SHD_GPIO_InitPin(PIN_PIEZO, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_BUZZER, GPIO_OUTPUT);

    SHD_GPIO_InitPin(PIN_DC_MOTOR, GPIO_OUTPUT);

    SHD_GPIO_InitPin(PIN_STEP_MOTOR1, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_STEP_MOTOR2, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_STEP_MOTOR3, GPIO_OUTPUT);
    SHD_GPIO_InitPin(PIN_STEP_MOTOR4, GPIO_OUTPUT);

    SHD_GPIO_InitPin(PIN_RELAY, GPIO_OUTPUT);

    // 나머지 드라이버 초기화
    SHD_ADC0_Init();
    SHD_FTM0_Init();
    SHD_FTM0_InitPwmChannel(6); // FTM0_CH6 (LED 8)
    SHD_FTM0_InitPwmChannel(2); // FTM0_CH2 (Servo)
    SHD_LPI2C0_Init();
    SHD_CAN0_Init();
    SHD_LPIT0_Init();
    SHD_LPUART1_Init(9600);

    /* 5. 인터럽트 설정 및 활성화 (+lpit0 타이머 주기 설정) */
    // 핀 인터럽트 설정
    SHD_PORT_SetPinIT(PIN_UWAVE_ECHO, PORT_IT_IRQ_RISING);
    SHD_PORT_SetPinIT(PIN_BTN1, PORT_IT_IRQ_FALLING);
    SHD_PORT_SetPinIT(PIN_BTN2, PORT_IT_IRQ_FALLING);
    SHD_PORT_SetPinIT(PIN_BTN3, PORT_IT_IRQ_FALLING);
    SHD_PORT_SetPinIT(PIN_BTN4, PORT_IT_IRQ_FALLING);

    // NVIC 인터럽트 활성화 및 우선순위 설정
    SHD_IT_EnableIRQ(PORTC_IRQn);             // uWave-echo(PTC13)
    SHD_IT_SetPriority(PORTC_IRQn, 6);

    SHD_IT_EnableIRQ(PORTE_IRQn);             // 버튼(PTE13-16)
    SHD_IT_SetPriority(PORTE_IRQn, 8);

    SHD_IT_EnableIRQ(CAN0_ORed_0_15_MB_IRQn); // CAN0 수신
    SHD_IT_SetPriority(CAN0_ORed_0_15_MB_IRQn, 6);

    // SHD_IT_EnableIRQ(LPIT0_Ch0_IRQn);         
    // SHD_IT_SetPriority(LPIT0_Ch0_IRQn, 10);

    SHD_IT_EnableIRQ(LPIT0_Ch1_IRQn);         // CAN 500ms Broadcast
    SHD_IT_SetPriority(LPIT0_Ch1_IRQn, 10);

    SHD_IT_EnableIRQ(LPIT0_Ch2_IRQn);         // 1초 보안 경고 상태 LED
    SHD_IT_SetPriority(LPIT0_Ch2_IRQn, 10);

    // SHD_IT_EnableIRQ(LPIT0_Ch3_IRQn);         // us 딜레이 함수
    // SHD_IT_SetPriority(LPIT0_Ch3_IRQn, 12);

}
