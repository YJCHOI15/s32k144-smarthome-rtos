#ifndef LPUART_DRIVER_H
#define LPUART_DRIVER_H

#include "S32K144.h"
#include <stdint.h>


void SHD_LPUART1_Init(uint32_t baud_rate);
void SHD_LPUART1_WriteString(const char* str);

#endif /* LPUART_DRIVER_H */
