#include "ftm_driver.h"

/* 50Hz PWM 주기를 위한 MOD 값 (SPLLDIV1 80MHz 기준) */
#define FTM0_MOD_VALUE (12499)

/**
 * FTM0 모듈의 클럭을 활성화한다.
 */
void SHD_FTM0_Init(void) {

    /* FTM0 모듈에 Bus Clock을 클럭 소스로 하여 활성화 */
    PCC->PCCn[PCC_FTM0_INDEX] = PCC_PCCn_PCS(0b110)  /* PCS=6: SPLLDIV1_CLK (80MHz) */
                              | PCC_PCCn_CGC_MASK;   /* CGC=1: Clock enabled */
}

/**
 * FTM0의 특정 채널을 PWM 출력 모드로 초기화한다.
 * 서보 모터 제어를 위해 PWM 주기는 50Hz (20ms)로 설정된다.
 */
void SHD_FTM0_InitPwmChannel(uint8_t channel) {

    /* 1. FTM0 카운터 설정 (최초 한 번만 실행) */
    if ((FTM0->SC & FTM_SC_CLKS_MASK) == 0) {
        /* FTM0 Write Protection Disable */
        FTM0->MODE |= FTM_MODE_WPDIS_MASK;

        /* Edge-Aligned PWM (EPWM), High-true pulses 설정 */
        /* ELSB=1, ELSA=0 -> High-true pulses */
        FTM0->CONTROLS[channel].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSA_MASK;

        /* FTM 카운터 초기값 설정 */
        FTM0->CNTIN = 0;

        /* PWM 주기(Frequency) 설정: 50Hz (20ms)
         * FTM Clock = SPLLDIV1_CLK / Prescaler = 80MHz / 128 = 625,000 Hz
         * MOD = (FTM Clock / PWM Freq) - 1 = (625000 / 50) - 1 = 12499
         */
        FTM0->MOD = FTM0_MOD_VALUE;

        /* SC (Status and Control) 레지스터 설정 */
        FTM0->SC = FTM_SC_PS(0b111)      /* PS=7: Prescaler = 128 */
                   | FTM_SC_CLKS(0b01);  /* CLKS=1: System Clock (Bus Clock)을 소스로 카운터 시작 */

        /* FTM0 Write Protection Enable */
        FTM0->MODE &= ~FTM_MODE_WPDIS_MASK;
    } else {
        /* 이미 카운터가 동작 중이면, 해당 채널만 EPWM 모드로 설정 */
        FTM0->CONTROLS[channel].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSA_MASK;
    }
}

/**
 * 지정된 FTM0 채널의 PWM Duty Cycle을 설정한다.
 */
void SHD_FTM0_SetDutyCycle(uint8_t channel, uint8_t duty_cycle) {

    uint16_t new_cnv;

    if (duty_cycle > 100) {
        duty_cycle = 100; // 최대 100%로 제한
    }

    /* Duty Cycle(%)을 CnV(Channel Value) 값으로 변환 */
    /* CnV = (MOD * DutyCycle) / 100 */
    new_cnv = (FTM0_MOD_VALUE * duty_cycle) / 100;

    /* 계산된 CnV 값을 해당 채널 레지스터에 기록 */
    FTM0->CONTROLS[channel].CnV = new_cnv;
}
