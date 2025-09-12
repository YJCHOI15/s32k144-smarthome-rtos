#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "drivers/port_driver.h"

typedef enum {
    GPIO_INPUT  = 0,
    GPIO_OUTPUT = 1
} sh_pin_direction_t;

void SHD_GPIO_InitPin(sh_port_pin_t pin_info, sh_pin_direction_t dir);
void SHD_GPIO_WritePin(sh_port_pin_t pin_info, uint8_t value);
uint8_t SHD_GPIO_ReadPin(sh_port_pin_t pin_info);
void SHD_GPIO_TogglePin(sh_port_pin_t pin_info);


#endif /* GPIO_DRIVER_H */
