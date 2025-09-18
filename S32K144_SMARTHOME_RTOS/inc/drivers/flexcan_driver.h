#ifndef FLEXCAN_DRIVER_H
#define FLEXCAN_DRIVER_H

#include "S32K144.h"
#include <stdbool.h>
#include <stdint.h>

typedef void (*can_rx_callback_t)(uint32_t id, uint8_t* data, uint8_t dlc);

void SHD_CAN0_Init(void);
void SHD_CAN0_Transmit(uint32_t id, uint8_t* data, uint8_t dlc);
void SHD_CAN0_RegisterRxCallback(can_rx_callback_t callback);

#endif /* FLEXCAN_DRIVER_H */
