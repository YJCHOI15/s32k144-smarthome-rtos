#include "drivers/adc_driver.h"

/**
 * ADC0 모듈을 초기화하고 calibration을 수행한다.
 */
void SHD_ADC0_Init(void) {

    /* 1. ADC0 모듈에 클럭(SOSCDIV2_CLK) 활성화 */
    PCC->PCCn[PCC_ADC0_INDEX] &= ~PCC_PCCn_CGC_MASK; /* Disable clock to change PCS */
    PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_PCS(1);    /* PCS=1: Select SOSCDIV2_CLK */
    PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_CGC_MASK;  /* Enable clock for ADC0 */

    /* 2. ADC Calibration 사전 설정: 32 샘플 평균 활성화 */
    ADC0->SC3 = ADC_SC3_AVGE_MASK     // AVGE=1: 하드웨어 평균 기능 활성화 
              | ADC_SC3_AVGS(2);      // AVGS=2: 32개 샘플 평균으로 설정

    /* 3. ADC Calibration */
    ADC0->SC3 |= ADC_SC3_CAL_MASK;    // CAL=1: Start calibration 
    /* 보정 실패 플래그(CALF)가 0이고, 보정 활성 플래그(CAL)가 1인 동안 대기 */
    while (ADC0->SC3 & ADC_SC3_CAL_MASK) {}

    /* 4. 보정이 끝난 후, 일반 변환을 위해 평균 기능 비활성화 */
    ADC0->SC3 &= ~ADC_SC3_AVGE_MASK;

    /* 5. ADC0 기본 설정 */
    ADC0->CFG1 = ADC_CFG1_MODE(1) |     // 12비트 해상도 설정
                ADC_CFG1_ADICLK(1) |    // 변환 속도를 결정할 기준 클럭을 선택
                ADC_CFG1_ADIV(0);       // ADICLK에서 선택한 클럭 분주비 설정

    /* SC1A: 변환 완료 인터럽트 비활성화 */
    ADC0->SC1[0] = ADC_SC1_AIEN(0);
}

/**
 * 지정된 ADC 채널의 아날로그 값을 읽어 디지털 값으로 변환한다.
 */
uint16_t SHD_ADC0_ReadChannel(uint8_t channel) {

    /* 1. 변환할 채널 선택 (채널 선택과 동시에 변환 시작) */
    /* ADCH 필드에 채널 번호를 쓰고, 인터럽트는 비활성화 상태로 유지 */
    ADC0->SC1[0] = ADC_SC1_ADCH(channel);

    /* 2. 변환 완료 플래그(COCO - Conversion Complete)가 1이 될 때까지 대기 */
    while ((ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0) {}

    /* 3. 변환 결과 레지스터(R[0])에서 값을 읽어 반환 */
    return (uint16_t)ADC0->R[0];
}
