#include "lpit_driver.h"
#include <stddef.h> 

/* 각 타이머 채널의 콜백 함수를 저장하기 위한 정적 배열 */
static void (*g_lpit_callbacks[4])(void) = {NULL, NULL, NULL, NULL};

/**
 * LPIT 모듈을 초기화한다.
 */
void SHD_LPIT0_Init(void) {
    /* 1. LPIT 모듈에 클럭 활성화 (SOSCDIV2_CLK, 8MHz) */
    PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(1)      /* PCS=1: Select SOSCDIV2_CLK */
                              | PCC_PCCn_CGC_MASK;   /* CGC=1: Clock enabled */

    /* 2. LPIT0 모듈 리셋 */
    LPIT0->MCR |= LPIT_MCR_SW_RST_MASK;
    LPIT0->MCR &= ~LPIT_MCR_SW_RST_MASK;

    /* 3. LPIT0 모듈 활성화 */
    /* M_CEN=1: Module Clock Enable. DBG_EN=1: Debug mode에서도 동작 */
    LPIT0->MCR = LPIT_MCR_M_CEN_MASK | LPIT_MCR_DBG_EN_MASK;
}

/**
 * 지정된 LPIT 채널에 주기적인 타이머를 설정한다.
 */
void SHD_LPIT0_SetPeriodic(uint8_t timer_ch, uint32_t period_ms, void (*callback)(void)) {

    /* 1. 콜백 함수 등록 */
    g_lpit_callbacks[timer_ch] = callback;

    /* 2. 타이머 주기(ms)를 클럭 카운트 값으로 변환 */
    /* LPIT 클럭 소스가 8MHz 이므로, 1ms = 8000 카운트 */
    uint32_t timer_val = 8000UL * period_ms;

    /* 3. 해당 채널의 타이머 값(TVAL) 설정 */
    LPIT0->TMR[timer_ch].TVAL = timer_val;

    /* 4. 해당 채널의 타이머 활성화 */
    /* T_EN=1: Timer Enable. MODE=00 (Default): 32-bit periodic counter */
    LPIT0->TMR[timer_ch].TCTRL = LPIT_TMR_TCTRL_T_EN_MASK;

    /* 5. 해당 채널의 인터럽트 활성화 */
    LPIT0->MIER |= (1UL << timer_ch);
}

/* shh_sensor - 온습도 통신용 delay 함수 */
void SHD_LPIT0_CH3_DelayUs(uint32_t us) {

    /* 1. 타이머 주기를 클럭 카운트 값으로 변환 (8MHz 기준, 1us = 8 카운트) */
    uint32_t timer_val = 8 * us;
    LPIT0->TMR[3].TVAL = timer_val;

    /* 2. 타이머 시작 */
    LPIT0->SETTEN |= (1UL << 3);

    /* 3. 타이머 만료 플래그(TIF)가 1이 될 때까지 대기 (Polling) */
    while ((LPIT0->MSR & (1UL << 3)) == 0);

    /* 4. 타이머 비활성화 및 플래그 클리어 */
    LPIT0->CLRTEN |= (1UL << 3);
    LPIT0->MSR = (1UL << 3); // 플래그는 쓰기로 클리어
}

/* 이 함수들은 startup.s 파일의 벡터 테이블에 각각 등록되어 있다. */

// Sensor_Task를 1초마다 깨워서 온/습도, 밝기 값 읽음
void LPIT0_Ch0_IRQHandler(void) {
    LPIT0->MSR = LPIT_MSR_TIF0_MASK;

    if (g_lpit_callbacks[0] != NULL) {
        g_lpit_callbacks[0]();
    }
}

// CAN_Comm_Task가 0.5초마다 시스템의 현재 상태를 외부에 알림
void LPIT0_Ch1_IRQHandler(void) {
    LPIT0->MSR = LPIT_MSR_TIF1_MASK;
    if (g_lpit_callbacks[1] != NULL) {
        g_lpit_callbacks[1]();
    }
}

// 보안 모드일 때 1초마다 인터럽트를 발생시켜 LED 상태를 반전시킴
void LPIT0_Ch2_IRQHandler(void) {
    LPIT0->MSR = LPIT_MSR_TIF2_MASK;
    if (g_lpit_callbacks[2] != NULL) {
        g_lpit_callbacks[2]();
    }
}

// 사용 안함
void LPIT0_Ch3_IRQHandler(void) {
    LPIT0->MSR = LPIT_MSR_TIF3_MASK;
    if (g_lpit_callbacks[3] != NULL) {
        g_lpit_callbacks[3]();
    }
}
