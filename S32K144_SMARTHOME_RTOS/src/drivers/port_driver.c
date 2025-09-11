#include "port_driver.h"

/**
 * 모든 PORT 모듈(A-E)의 클럭을 활성화한다.
 * PCC(Peripheral Clock Controller)를 통해 각 PORT 모듈에 클럭을 공급해야
 * 해당 모듈의 레지스터에 접근하고 설정을 변경할 수 있다.
 */
void SHD_PORT_Init(void) {

    /* PCC 레지스터의 CGC(Clock Gate Control) 비트를 1로 설정하여 클럭 활성화 */
    PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK;
}

/**
 * 지정된 핀의 기능을 설정한다.
 * 각 핀의 PCR(Pin Control Register)의 MUX 필드를 변경하여 핀의 역할을 결정한다.
 */
void SHD_PORT_SetPinMux(PORT_Type *port, uint32_t pin, port_mux_t mux) {

    /* MUX 필드를 먼저 0으로 클리어한 후, 원하는 MUX 값으로 설정 */
    port->PCR[pin] = (port->PCR[pin] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(mux);
}

/**
 * 지정된 핀의 인터럽트 조건을 설정한다.
 * 각 핀의 PCR(Pin Control Register)의 IRQC 필드를 변경하여
 * 인터럽트 발생 조건을 결정한다.
 */
void SHD_PORT_SetPinIT(PORT_Type *port, uint32_t pin, port_it_t it_config) {

    /* IRQC 필드를 먼저 0으로 클리어한 후, 원하는 인터럽트 조건으로 설정 */
    port->PCR[pin] = (port->PCR[pin] & ~PORT_PCR_IRQC_MASK) | PORT_PCR_IRQC(it_config);
}
