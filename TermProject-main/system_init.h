#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "device_registers.h"
#include "ADC.h"
#include "clocks_and_modes.h"
#include "LPUART.h"
#include "lcd1602A.h"
#include "yongjin.h"

#define LED1_ON           (PTE->PCOR |= (1 << 2))
#define LED1_OFF          (PTE->PSOR |= (1 << 2))
#define LED2_ON           (PTE->PCOR |= (1 << 3))
#define LED2_OFF          (PTE->PSOR |= (1 << 3))
#define LED5_TOG          (PTE->PTOR |= (1 << 6))
#define LED5_OFF          (PTE->PSOR |= (1 << 6))
#define LED6_ON           (PTE->PCOR |= (1 << 7))
#define LED6_OFF          (PTE->PSOR |= (1 << 7))

#define BLUE_ON           (PTD->PCOR |= (1 << 0))
#define BLUE_OFF          (PTD->PSOR |= (1 << 0))
#define RED_ON            (PTD->PCOR |= (1 << 15))
#define RED_OFF           (PTD->PSOR |= (1 << 15))
#define GREEN_ON          (PTD->PCOR |= (1 << 16))
#define GREEN_OFF         (PTD->PSOR |= (1 << 16))
#define UW_TRIG_HIGH      (PTE->PSOR |= (1 << 0))
#define UW_TRIG_LOW       (PTE->PCOR |= (1 << 0))
#define UW_ECHO_READ      (PTE->PDIR & (1 << 1))
#define SW1_ON            ((PTC->PDIR & (1 << 1)) == 0)
// #define SW2_ON            ((PTC->PDIR & (1 << 2)) == 0)
#define SW3_ON            ((PTC->PDIR & (1 << 9)) == 0)
#define SW4_ON            ((PTC->PDIR & (1 << 17)) == 0)
#define SW5_ON            ((PTC->PDIR & (1 << 5)) == 0)

#define LCD_EN            (PTB->PSOR |= 1<<0)
#define LCD_NEN           (PTB->PCOR |= 1<<0)
#define LCD_RW            (PTB->PCOR |= 1<<1)
#define LCD_RS            (PTB->PSOR |= 1<<2)


void system_init(void);
void PORT_init(void);
void WDOG_disable(void);
void LPIT0_init(void);
void LPIT0_Ch2_init (uint32_t delay);
void delay_ms (volatile int ms);
void NVIC_init_IRQs(void);
void FTM_init (void);
void FTM2_CH5_PWM (int i);


void LPIT0_Ch3_init (uint32_t delay);
void delay_100ns (volatile int hns);


#endif
