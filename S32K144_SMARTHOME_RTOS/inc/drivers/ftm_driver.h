#ifndef FTM_DRIVER_H
#define FTM_DRIVER_H

#include "S32K144.h"
#include <stdint.h>

void SHD_FTM_Init(void);
void SHD_FTM_InitPwmChannel(FTM_Type *FTMx, uint8_t channel);
void SHD_FTM_SetDutyCycle(FTM_Type *FTMx, uint8_t channel, uint8_t duty_cycle);

#endif /* FTM_DRIVER_H */