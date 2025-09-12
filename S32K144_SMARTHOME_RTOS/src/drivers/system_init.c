#include "drivers/system_init.h"
#include "S32K144.h"

/* system_init.c에서만 사용 */
static void SOSC_init_8MHz(void);
static void SPLL_init_160MHz(void);
static void NormalRUNmode_80MHz(void);

/**
 * S32K144의 기본 시스템을 초기화한다.
 */
void SHD_System_Init(void) {
    /* 1. 워치독 타이머(WDOG) 비활성화
     * 리셋 후 워치독 타이머가 기본적으로 활성화 되어 있다. 
     * 원활한 디버깅을 위해 워치독 타이머를 비활성화 한다. 
     */
    WDOG->CNT = 0xD928C520;     /* Unlock sequence */
    WDOG->TOVAL = 0x0000FFFF;   /* Set timeout value to max */
    WDOG->CS = 0x00002100;      /* Disable WDOG */

    /* 2. 시스템 클럭(System Clock) 설정 (80MHz Core Clock) */
    SOSC_init_8MHz();       /* 외부 8MHz 크리스탈(SOSC) 활성화 */
    SPLL_init_160MHz();     /* SOSC를 사용하여 160MHz SPLL 클럭 생성 */
    NormalRUNmode_80MHz();  /* SPLL을 시스템 클럭 소스로 설정하고 80MHz로 분주 */
}

/**
 * 8MHz 외부 크리스탈 발진기(SOSC)를 초기화한다.
 */
static void SOSC_init_8MHz(void) {
    /* SOSC 분주비 설정 */
    SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(1) | /* SOSCDIV1 = 1: divide by 1 */
                   SCG_SOSCDIV_SOSCDIV2(1);   /* SOSCDIV2 = 1: divide by 1 */

    /* SOSC 설정: Medium frequency (1MHz-8MHz), 외부 크리스탈 사용 */
    SCG->SOSCCFG = SCG_SOSCCFG_RANGE(2)   |
                   SCG_SOSCCFG_EREFS_MASK;

    /* SOSC 활성화 전까지 레지스터 잠금 확인 및 해제 */
    while(SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK);
    SCG->SOSCCSR = SCG_SOSCCSR_SOSCEN_MASK;

    /* SOSC 클럭이 안정화될 때까지 대기 */
    while(!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK));
}

/**
 * 시스템 PLL(SPLL, 주파수 증폭기)을 160MHz로 초기화한다.
 */
static void SPLL_init_160MHz(void) {
    /* SPLL 비활성화 전까지 레지스터 잠금 확인 및 해제 */
    while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK);
    SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;

    /* SPLL 분주비 설정 (SPLLDIV1, SPLLDIV2는 보조 출력 클럭) */
    SCG->SPLLDIV = SCG_SPLLDIV_SPLLDIV1(2) | /* SPLLDIV1 divide by 2 */
                   SCG_SPLLDIV_SPLLDIV2(3);   /* SPLLDIV2 divide by 4 */

    /* SPLL 설정: 8MHz(SOSC) / 1(PREDIV) * 40(MULT) / 2 = 160 MHz */
    SCG->SPLLCFG = SCG_SPLLCFG_MULT(24);

    /* SPLL 활성화 전까지 레지스터 잠금 확인 및 해제 */
    while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK);
    SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;

    /* SPLL 클럭이 안정화될 때까지 대기 */
    while(!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK));
}

/**
 * 준비된 160MHz SPLL 클럭를 가지고, 
 * 시스템의 각 부분(CPU, Bus, Flash memory)에 최종적으로 전력을 공급한다.
 */
static void NormalRUNmode_80MHz(void) {
    /* 시스템 클럭 소스를 SPLL로 설정하고 각 버스의 분주비 설정
     * - Core Clock: 160MHz / 2 = 80MHz
     * - Bus Clock: 80MHz / 2 = 40MHz
     * - Slow Clock (Flash): 80MHz / 3 = 26.67MHz
     */
    SCG->RCCR = SCG_RCCR_SCS(6)
              | SCG_RCCR_DIVCORE(1)
              | SCG_RCCR_DIVBUS(1)
              | SCG_RCCR_DIVSLOW(2);

    /* 시스템 클럭 소스가 SPLL로 완전히 전환될 때까지 대기 */
    while (((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT ) != 6) {}
}