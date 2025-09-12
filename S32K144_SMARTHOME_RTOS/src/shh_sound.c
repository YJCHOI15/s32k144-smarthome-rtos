#include "shh_sound.h"
#include "sh_config.h"
#include "drivers/gpio_driver.h"

void SHH_Buzzer_StartAlarm(void) {
    SHD_GPIO_WritePin(PIN_BUZZER, 1);
}

void SHH_Buzzer_StopAlarm(void) {
    SHD_GPIO_WritePin(PIN_BUZZER, 0);
}

void SHH_Piezo_Beep(void) {
    SHD_GPIO_WritePin(PIN_PIEZO, 1);
    for (volatile uint32_t i = 0; i < 500; i++);
    SHD_GPIO_WritePin(PIN_PIEZO, 0);
}
