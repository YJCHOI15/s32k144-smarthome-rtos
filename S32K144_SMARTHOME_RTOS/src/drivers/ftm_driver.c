#include "drivers/ftm_driver.h"

/* LED8, 서보모터 둘다 50Hz PWM 주기 사용 (SPLLDIV1 80MHz 기준) */
#define FTM_MOD_VALUE (12499)

/**
 * FTM 모듈의 클럭을 활성화한다.
 */
void SHD_FTM_Init(void) {

    /* FTM 모듈에 SPLLDIV1_CLK를 클럭 소스로 하여 활성화 */
    PCC->PCCn[PCC_FTM0_INDEX] = PCC_PCCn_PCS(6)  /* PCS=6: SPLLDIV1_CLK (80MHz) */
                              | PCC_PCCn_CGC_MASK;   /* CGC=1: Clock enabled */

    PCC->PCCn[PCC_FTM1_INDEX] = PCC_PCCn_PCS(6)  /* PCS=6: SPLLDIV1_CLK (80MHz) */
                              | PCC_PCCn_CGC_MASK;   /* CGC=1: Clock enabled */                
}

/**
 * FTM의 특정 채널을 PWM 출력 모드로 초기화한다.
 */
void SHD_FTM_InitPwmChannel(FTM_Type *FTMx, uint8_t channel) {

    /* 1. FTMx 카운터 설정 (최초 한 번만 실행) */
    if ((FTMx->SC & FTM_SC_CLKS_MASK) == 0) {

        /* FTMx 쓰기 방지 비활성화 */
        FTMx->MODE |= FTM_MODE_WPDIS_MASK;

        /* FTM 카운터 초기값 설정 */
        FTMx->CNTIN = 0;

        /* PWM 주기(Frequency) 설정: 50Hz (20ms) */
        FTMx->MOD = FTM_MOD_VALUE;

        /* SC (Status and Control) 레지스터 설정 및 카운터 시작 */
        FTMx->SC = FTM_SC_PS(7)      /* PS=7: Prescaler = 128 */
                   | FTM_SC_CLKS(1);  /* CLKS=1: Clock as per PCS selection */

        /* FTMx 쓰기 방지 재활성화 */
        FTMx->MODE &= ~FTM_MODE_WPDIS_MASK;
    }

    /* 2. 채널 모드 설정 */
    if (FTMx == FTM0) {
        /* FTM0 → High-true pulses */
        FTMx->CONTROLS[channel].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    } else if (FTMx == FTM1) {
        /* FTM1 → Low-true pulses */
        FTMx->CONTROLS[channel].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSA_MASK;
    }
    
    /* 3. [수정] 해당 채널의 PWM 출력을 물리적 핀으로 활성화한다. */
    /* SC 레지스터는 쓰기 방지가 되어 있으므로, 일시적으로 해제해야 한다. */
    FTMx->MODE |= FTM_MODE_WPDIS_MASK;
    FTMx->SC |= (1UL << (FTM_SC_PWMEN0_SHIFT + channel));
    FTMx->MODE &= ~FTM_MODE_WPDIS_MASK;
}

/**
 * 지정된 FTMx 채널의 PWM Duty Cycle을 설정한다.
 */
void SHD_FTM_SetDutyCycle(FTM_Type *FTMx, uint8_t channel, uint8_t duty_cycle) {
    uint32_t new_cnv;

    if (duty_cycle > 100) {
        duty_cycle = 100;
    }

    // Duty Cycle → CnV 변환
    new_cnv = (uint32_t)(FTM_MOD_VALUE * duty_cycle) / 100;

    // 채널에 기록
    FTMx->CONTROLS[channel].CnV = (uint16_t)new_cnv;
}
