#ifndef PORT_DRIVER_H
#define PORT_DRIVER_H

#include "S32K144.h"
#include <stdint.h>  

typedef enum {
    PORT_MUX_ANALOG = 0,
    PORT_MUX_GPIO   = 1,
    PORT_MUX_ALT2   = 2,
    PORT_MUX_ALT3   = 3,
    PORT_MUX_ALT4   = 4,
    PORT_MUX_ALT5   = 5,
    PORT_MUX_ALT6   = 6,
    PORT_MUX_ALT7   = 7
} port_mux_t;

typedef enum {
    PORT_IT_DISABLED    = 0,  // Interrupt/DMA disabled
    PORT_IT_DMA_RISING  = 1,  // DMA Request on rising edge
    PORT_IT_DMA_FALLING = 2,  // DMA Request on falling edge
    PORT_IT_DMA_EITHER  = 3,  // DMA Request on either edge
    PORT_IT_IRQ_LOGIC_0 = 8,  // Interrupt when logic 0
    PORT_IT_IRQ_RISING  = 9,  // Interrupt on rising edge
    PORT_IT_IRQ_FALLING = 10, // Interrupt on falling edge
    PORT_IT_IRQ_EITHER  = 11, // Interrupt on either edge
    PORT_IT_IRQ_LOGIC_1 = 12  // Interrupt when logic 1
} port_it_t;


void SHD_PORT_Init(void);
void SHD_PORT_SetPinMux(PORT_Type *port, uint32_t pin, port_mux_t mux);
void SHD_PORT_SetPinIT(PORT_Type *port, uint32_t pin, port_it_t it_config);


#endif /* PORT_DRIVER_H */