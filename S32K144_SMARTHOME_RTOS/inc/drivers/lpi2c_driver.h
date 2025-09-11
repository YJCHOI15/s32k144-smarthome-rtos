#ifndef LPI2C_DRIVER_H
#define LPI2C_DRIVER_H

#include "S32K144.h"
#include <stdbool.h>
#include <stdint.h>


void SHD_LPI2C0_Init(void);
bool SHD_LPI2C0_Write(uint8_t slave_addr, uint8_t* data, uint32_t len);
bool SHD_LPI2C0_Read(uint8_t slave_addr, uint8_t* buffer, uint32_t len);

#endif /* LPI2C_DRIVER_H */
