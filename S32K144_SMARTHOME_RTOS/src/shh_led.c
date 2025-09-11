#include "shh_led.h"
#include "drivers/gpio_driver.h"
#include "drivers/ftm_driver.h"
#include "sh_config.h"

void SHH_PowerLED_On(void) {
    SHD_GPIO_WritePin(PIN_LED1, 0); 
}

void SHH_PowerLED_Off(void) {
    SHD_GPIO_WritePin(PIN_LED1, 1); 
}

void SHH_ModeLED_Set(system_mode_t mode) {

    SHD_GPIO_WritePin(PIN_LED_RED, 1);
    SHD_GPIO_WritePin(PIN_LED_GREEN, 1);
    SHD_GPIO_WritePin(PIN_LED_BLUE, 1);

    switch (mode) {
        case MODE_MONITORING:
            SHD_GPIO_WritePin(PIN_LED_GREEN, 0); // 모니터링 모드: 녹색
            break;
        case MODE_MANUAL:
            SHD_GPIO_WritePin(PIN_LED_BLUE, 0); // 수동 제어 모드: 파란색
            break;
        case MODE_SECURITY:
            SHD_GPIO_WritePin(PIN_LED_RED, 0); // 보안 모드: 빨간색
            break;
        default:
            break;
    }
}

void SHH_SecurityStandbyLED_On(void) {
    SHD_GPIO_WritePin(PIN_LED2, 0);
}

void SHH_SecurityStandbyLED_Off(void) {
    SHD_GPIO_WritePin(PIN_LED2, 1);
}

void SHH_SecurityStandbyLED_Toggle(void) {
    SHD_GPIO_TogglePin(PIN_LED2);
}

void SHH_SecurityWarningLED_On(void) {
    SHD_GPIO_WritePin(PIN_LED4, 0);
    SHD_GPIO_WritePin(PIN_LED5, 0);
    SHD_GPIO_WritePin(PIN_LED6, 0);
}

void SHH_SecurityWarningLED_Off(void) {
    SHD_GPIO_WritePin(PIN_LED4, 1);
    SHD_GPIO_WritePin(PIN_LED5, 1);
    SHD_GPIO_WritePin(PIN_LED6, 1);
}

void SHH_MainLight_SetBrightness(uint8_t brightness_percent) {
    // FTM0의 1번 채널(LED 8)에 Duty Cycle 설정
    SHD_FTM0_SetDutyCycle(1, brightness_percent);
}
