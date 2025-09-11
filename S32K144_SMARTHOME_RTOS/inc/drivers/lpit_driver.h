#ifndef LPIT_DRIVER_H
#define LPIT_DRIVER_H

#include "S32K144.h"
#include <stdint.h>


void SHD_LPIT0_Init(void);
void SHD_LPIT0_SetPeriodic(uint8_t timer_ch, uint32_t period_ms, void (*callback)(void));

#endif /* LPIT_DRIVER_H */
