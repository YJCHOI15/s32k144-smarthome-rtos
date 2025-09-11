#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "S32K144.h"
#include "port_driver.h"
#include <stdint.h>  

typedef enum {
    GPIO_INPUT  = 0,
    GPIO_OUTPUT = 1
} pin_direction_t;

void SHD_GPIO_InitPin(port_pin_t pin_info, pin_direction_t dir);
void SHD_GPIO_WritePin(port_pin_t pin_info, uint8_t value);
uint8_t SHD_GPIO_ReadPin(port_pin_t pin_info);
void SHD_GPIO_TogglePin(port_pin_t pin_info);


#endif /* GPIO_DRIVER_H */
