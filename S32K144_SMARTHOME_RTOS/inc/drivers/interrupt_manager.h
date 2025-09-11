#ifndef INTERRUPT_MANAGER_H
#define INTERRUPT_MANAGER_H

#include "S32K144.h"
#include <stdint.h>


void SHD_IT_EnableIRQ(IRQn_Type irq_num);
void SHD_IT_DisableIRQ(IRQn_Type irq_num);
void SHD_IT_SetPriority(IRQn_Type irq_num, uint8_t priority);

#endif /* INTERRUPT_MANAGER_H */
