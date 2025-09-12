#include "drivers/sh_it_manager.h"
#include "S32K144.h"

 /** 
  * irq_num / 32는 ISER, ICER 배열의 인덱스를,
  * irq_num % 32는 비트 위치를 결정한다. 
  * S32K144는 4비트 우선순위를 사용하며, 상위 4비트에 값을 설정해야 한다.
  */

/**
 * 지정된 IRQ 번호의 인터럽트를 NVIC에서 활성화한다.
 */
void SHD_IT_EnableIRQ(IRQn_Type irq_num) {
    S32_NVIC->ISER[irq_num >> 5] = (1UL << (irq_num & 0x1F));
}

/**
 * 지정된 IRQ 번호의 인터럽트를 NVIC에서 비활성화한다.
 */
void SHD_IT_DisableIRQ(IRQn_Type irq_num) {
    S32_NVIC->ICER[irq_num >> 5] = (1UL << (irq_num & 0x1F));
}

/**
 * 지정된 IRQ 번호의 인터럽트 우선순위를 설정합니다.
 */
void SHD_IT_SetPriority(IRQn_Type irq_num, uint8_t priority) {
    S32_NVIC->IP[irq_num] = (uint8_t)((priority << 4) & 0xFF);
}
