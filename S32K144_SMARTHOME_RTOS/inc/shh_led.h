#ifndef SHH_LED_H
#define SHH_LED_H

#include "sh_config.h"


/* 전원 LED 제어 */
void SHH_PowerLED_On(void);
void SHH_PowerLED_Off(void);

/* 현재 시스템 모드에 따라 RGB LED 제어 */
void SHH_ModeLED_Set(system_mode_t mode);

/* 보안 대기 상태 LED 제어 */
void SHH_SecurityStandbyLED_On(void);
void SHH_SecurityStandbyLED_Off(void);

/* 보안 경고 LED 제어 */
void SHH_SecurityWarningLED_On(void);
void SHH_SecurityWarningLED_Off(void);
void SHH_SecurityWarningLED_Toggle(void);

/* 외부 조명(LED 8) 밝기 제어(PWM) */
void SHH_MainLight_SetBrightness(uint8_t brightness_percent);

#endif /* SHH_LED_H */