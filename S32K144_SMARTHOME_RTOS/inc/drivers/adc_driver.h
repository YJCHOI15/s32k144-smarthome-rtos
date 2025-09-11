#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "S32K144.h"
#include <stdint.h>

void SHD_ADC0_Init(void);
uint16_t SHD_ADC0_ReadChannel(uint8_t channel);

#endif /* ADC_DRIVER_H */
