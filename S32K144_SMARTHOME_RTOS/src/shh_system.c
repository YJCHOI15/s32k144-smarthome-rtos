#include "shh_system.h"
#include "drivers/system_init.h"
#include "drivers/port_driver.h"
#include "drivers/gpio_driver.h"
#include "drivers/adc_driver.h"
#include "drivers/ftm_driver.h"
#include "drivers/lpi2c_driver.h"
#include "drivers/flexcan_driver.h"
#include "drivers/lpit_driver.h"
#include "drivers/lpuart_driver.h"
#include "drivers/interrupt_manager.h"
#include "sh_config.h" 

void SHH_Init(void)
{
    // /* 1. 코어 및 시스템 클럭 초기화 ) */
    // SHD_System_Init();

    // /* 2. 각 PORT 모듈의 클럭 활성화 */
    // SHD_PORT_Init();

    // /* 3. 핀맵에 따른 핀 기능(Mux) 및 인터럽트 설정 */
    // // LPUART1 Pins
    // SHD_PORT_SetPinMux(PIN_LPUART1_RX, PORT_MUX_ALT2);
    // SHD_PORT_SetPinMux(PIN_LPUART1_TX, PORT_MUX_ALT2);

    // // CAN0 Pins
    // SHD_PORT_SetPinMux(PIN_CAN0_RX, PORT_MUX_ALT5);
    // SHD_PORT_SetPinMux(PIN_CAN0_TX, PORT_MUX_ALT5);

    // // I2C0 Pins
    // SHD_PORT_SetPinMux(PIN_LPI2C0_SCL, PORT_MUX_ALT2);
    // SHD_PORT_SetPinMux(PIN_LPI2C0_SDA, PORT_MUX_ALT2);

    // // ADC Pins (아날로그 기능으로 설정)
    // SHD_PORT_SetPinMux(PIN_ADC_CDS, PORT_MUX_ANALOG);
    // SHD_PORT_SetPinMux(PIN_ADC_VR, PORT_MUX_ANALOG);

    // // FTM0 (PWM) Pins
    // SHD_PORT_SetPinMux(PIN_FTM0_CH1_LED8, PORT_MUX_ALT2);
    // SHD_PORT_SetPinMux(PIN_FTM0_CH2_SERVO, PORT_MUX_ALT2);

    // // GPIO Pins (GPIO 기능으로 설정)
    // // ... (모든 GPIO 핀에 대해 SHD_PORT_SetPinMux(PIN_XXX, PORT_MUX_GPIO) 호출) ...

    // // Interrupt Pins
    // SHD_PORT_SetPinIT(PIN_BTN_1, PORT_IT_FALLING_EDGE);
    // // ... (모든 버튼 및 uWave 센서 핀에 대해 인터럽트 설정) ...


    // /* 4. 각 주변장치 드라이버 초기화 */
    // SHD_GPIO_InitPin(PIN_LED_POWER, GPIO_OUTPUT); // 예시: 전원 LED
    // // ... (모든 GPIO 핀에 대해 방향 설정) ...

    // SHD_ADC0_Init();
    // SHD_FTM0_Init();
    // SHD_FTM0_InitPwmChannel(1); // LED 8 (FTM0_CH1)
    // SHD_FTM0_InitPwmChannel(2); // Servo (FTM0_CH2)

    // SHD_LPI2C0_Init();
    // SHD_CAN0_Init();
    // SHD_LPIT_Init();
    // SHD_LPUART1_Init(115200); // 115200 Baudrate로 초기화

    // /* 5. 인터럽트 활성화 (모든 설정이 끝난 후 마지막에 수행) */
    // SHD_IT_EnableIRQ(PORTE_IRQn); // 버튼 인터럽트 예시
    // SHD_IT_SetPriority(PORTE_IRQn, 5); // 우선순위 5로 설정
    // // ... (사용할 모든 인터럽트에 대해 Enable 및 Priority 설정) ...
}
