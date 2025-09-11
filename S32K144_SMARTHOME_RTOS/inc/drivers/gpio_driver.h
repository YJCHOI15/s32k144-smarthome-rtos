#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "S32K144.h"
#include <stdint.h>  

typedef enum {
    GPIO_INPUT  = 0,
    GPIO_OUTPUT = 1
} pin_direction_t;

void SHD_GPIO_InitPin(GPIO_Type *gpio, uint32_t pin, pin_direction_t dir);
void SHD_GPIO_WritePin(GPIO_Type *gpio, uint32_t pin, uint8_t value);
uint8_t SHD_GPIO_ReadPin(GPIO_Type *gpio, uint32_t pin);
void SHD_GPIO_TogglePin(GPIO_Type *gpio, uint32_t pin);


#endif /* GPIO_DRIVER_H */
