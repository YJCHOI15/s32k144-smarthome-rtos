#ifndef FTM_DRIVER_H
#define FTM_DRIVER_H

#include "S32K144.h"
#include <stdint.h>

void SHD_FTM0_Init(void);
void SHD_FTM0_InitPwmChannel(uint8_t channel);
void SHD_FTM0_SetDutyCycle(uint8_t channel, uint8_t duty_cycle);

#endif /* FTM_DRIVER_H */