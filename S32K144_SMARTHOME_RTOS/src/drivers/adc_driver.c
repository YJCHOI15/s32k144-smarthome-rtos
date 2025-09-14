#include "drivers/adc_driver.h"
#include "shh_uart.h"

/**
 * ADC0 모듈을 초기화하고 calibration을 수행한다.
 */
void SHD_ADC0_Init(void) {

    PCC->PCCn[PCC_ADC0_INDEX] &= ~PCC_PCCn_CGC_MASK; /* Disable clock to change PCS */
    PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_PCS(1);    /* PCS=1: Select SOSCDIV2_CLK */
    PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_CGC_MASK;  /* Enable clock for ADC0 */

	/* ADC0 Initialization: */  
    ADC0->SC1[0] |= ADC_SC1_ADCH_MASK;	/* ADCH=1F: Module is disabled for conversions	*/
                                        /* AIEN=0: Interrupts are disabled 			*/
    ADC0->CFG1 |= ADC_CFG1_ADIV_MASK
                |ADC_CFG1_MODE(1);	/* ADICLK=0: Input clk=ALTCLK1=SOSCDIV2 	*/
                                    /* ADIV=0: Prescaler=1 					*/
                                    /* MODE=1: 12-bit conversion 				*/

    ADC0->CFG2 = ADC_CFG2_SMPLTS(12);	/* SMPLTS=12(default): sample time is 13 ADC clks 	*/
    ADC0->SC2 = 0x00000000;         	/* ADTRG=0: SW trigger 							*/
                                    /* ACFE,ACFGT,ACREN=0: Compare func disabled		*/
                                    /* DMAEN=0: DMA disabled 							*/
                                    /* REFSEL=0: Voltage reference pins= VREFH, VREEFL */
    ADC0->SC3 = 0x00000000;       	/* CAL=0: Do not start calibration sequence 		*/
                                    /* ADCO=0: One conversion performed 				*/
                                    /* AVGE,AVGS=0: HW average function disabled 		*/
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
