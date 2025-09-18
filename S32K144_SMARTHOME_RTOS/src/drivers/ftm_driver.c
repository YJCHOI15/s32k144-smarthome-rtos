#include "drivers/ftm_driver.h"

/* 50Hz PWM 주기를 위한 MOD 값 (SPLLDIV1 80MHz 기준) */
#define FTM0_MOD_VALUE (12499)

/**
 * FTM0 모듈의 클럭을 활성화한다.
 */
void SHD_FTM0_Init(void) {

    /* FTM0 모듈에 SPLLDIV1_CLK를 클럭 소스로 하여 활성화 */
    PCC->PCCn[PCC_FTM0_INDEX] = PCC_PCCn_PCS(6)  /* PCS=6: SPLLDIV1_CLK (80MHz) */
                              | PCC_PCCn_CGC_MASK;   /* CGC=1: Clock enabled */
}

/**
 * FTM0의 특정 채널을 PWM 출력 모드로 초기화한다.
 */
void SHD_FTM0_InitPwmChannel(uint8_t channel) {

    /* 1. FTM0 카운터 설정 (최초 한 번만 실행) */
    if ((FTM0->SC & FTM_SC_CLKS_MASK) == 0) {

        /* FTM0 쓰기 방지 비활성화 */
        FTM0->MODE |= FTM_MODE_WPDIS_MASK;

        /* FTM 카운터 초기값 설정 */
        FTM0->CNTIN = 0;

        /* PWM 주기(Frequency) 설정: 50Hz (20ms) */
        FTM0->MOD = FTM0_MOD_VALUE;

        /* SC (Status and Control) 레지스터 설정 및 카운터 시작 */
        FTM0->SC = FTM_SC_PS(7)      /* PS=7: Prescaler = 128 */
                   | FTM_SC_CLKS(1);  /* CLKS=1: Clock as per PCS selection */

        /* FTM0 쓰기 방지 재활성화 */
        FTM0->MODE &= ~FTM_MODE_WPDIS_MASK;
    }

    /* 2. 해당 채널을 Edge-Aligned PWM, Low-true pulses 모드로 설정 */
    FTM0->CONTROLS[channel].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    
    /* 3. [수정] 해당 채널의 PWM 출력을 물리적 핀으로 활성화한다. */
    /* SC 레지스터는 쓰기 방지가 되어 있으므로, 일시적으로 해제해야 한다. */
    FTM0->MODE |= FTM_MODE_WPDIS_MASK;
    FTM0->SC |= (1UL << (FTM_SC_PWMEN0_SHIFT + channel));
    FTM0->MODE &= ~FTM_MODE_WPDIS_MASK;
}

/**
 * 지정된 FTM0 채널의 PWM Duty Cycle을 설정한다.
 */
void SHD_FTM0_SetDutyCycle(uint8_t channel, uint8_t duty_cycle) {

    uint32_t new_cnv;

    if (duty_cycle > 100){
        duty_cycle = 100; // 최대 100%로 제한
    }

    /* Duty Cycle(%)을 CnV(Channel Value) 값으로 변환 */
    new_cnv = (uint32_t)(FTM0_MOD_VALUE * duty_cycle) / 100;

    /* 계산된 CnV 값을 해당 채널 레지스터에 기록 */
    FTM0->CONTROLS[channel].CnV = (uint16_t)new_cnv;

}
