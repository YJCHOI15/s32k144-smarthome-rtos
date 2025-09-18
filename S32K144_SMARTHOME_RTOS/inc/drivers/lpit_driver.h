#ifndef LPIT_DRIVER_H
#define LPIT_DRIVER_H

#include "S32K144.h"
#include <stdint.h>


void SHD_LPIT0_Init(void);
void SHD_LPIT0_SetPeriodic(uint8_t timer_ch, uint32_t period_ms, void (*callback)(void));
void SHD_LPIT0_Stop(uint8_t timer_ch);
void SHD_LPIT0_Us_Timer_Start(uint8_t channel);
uint32_t SHD_LPIT0_GetCurrentUs(uint8_t channel);
void SHD_LPIT0_DelayUs(uint8_t timer_ch, uint32_t us);

#endif /* LPIT_DRIVER_H */
