#include "drivers/lpi2c_driver.h"

#define LPI2C_TIMEOUT 1000

/**
 * LPI2C0 모듈을 마스터 모드, 100kHz 속도로 초기화한다.
 */
void SHD_LPI2C0_Init(void) {
    /* 1. LPI2C0 모듈에 클럭 활성화 (SOSCDIV2_CLK, 8MHz) */
    PCC->PCCn[PCC_LPI2C0_INDEX] = PCC_PCCn_PCS(1)      /* PCS=1: Select SOSCDIV2_CLK */
                                | PCC_PCCn_CGC_MASK; /* CGC=1: Clock enabled */

    // Reset
    LPI2C0->MCR = LPI2C_MCR_RST_MASK;
    (void)LPI2C0->MCR;          // dummy read (권장)
    LPI2C0->MCR = 0;            // Reset 해제 (MEN=0 유지)

    // Baudrate, Prescaler 먼저 세팅
    LPI2C0->MCFGR1 = LPI2C_MCFGR1_PRESCALE(1);
    LPI2C0->MCCR0  = LPI2C_MCCR0_CLKLO(19) | LPI2C_MCCR0_CLKHI(19) |
                    LPI2C_MCCR0_SETHOLD(7) | LPI2C_MCCR0_DATAVD(7);

    // Status 클리어
    LPI2C0->MSR = 0xFFFFFFFF;

    // FIFO flush
    LPI2C0->MCR |= LPI2C_MCR_RTF_MASK | LPI2C_MCR_RRF_MASK;

    // Master Enable (맨 마지막)
    LPI2C0->MCR |= LPI2C_MCR_MEN_MASK;  
}

/**
 * I2C 버스를 통해 특정 주소의 슬레이브 장치에 데이터를 쓴다.
 */
bool SHD_LPI2C0_Write(uint8_t slave_addr, uint8_t* data, uint32_t len) {
    uint32_t i;
    uint32_t timeout = 0;

    /* 1. START 신호 및 슬레이브 주소(Write) 전송 */
    LPI2C0->MTDR = LPI2C_MTDR_CMD(4) | LPI2C_MTDR_DATA(slave_addr << 1);

    /* 2. 데이터 바이트 전송 */
    for (i = 0; i < len; ++i) {
        timeout = 0;
        /* 전송 데이터 레지스터가 비워질 때까지 대기 (TDRE 플래그) */
        while (!((LPI2C0->MSR) & LPI2C_MSR_TDF_MASK)) {
            timeout++;
            if (timeout > LPI2C_TIMEOUT) return false;
        }

        /* 데이터 전송 */
        LPI2C0->MTDR = LPI2C_MTDR_CMD(0) | LPI2C_MTDR_DATA(data[i]);

        /* NACK(Not Acknowledge) 수신 시 전송 중단 */
        if (LPI2C0->MSR & LPI2C_MSR_NDF_MASK) {
            LPI2C0->MSR |= LPI2C_MSR_NDF_MASK; // NACK 플래그 클리어
            return false;
        }
    }

    timeout = 0;
    while (!(LPI2C0->MSR & LPI2C_MSR_TDF_MASK)) {
        timeout++;
        if (timeout > LPI2C_TIMEOUT) return false;
    }

    /* 3. STOP 신호 전송 */
    LPI2C0->MTDR = LPI2C_MTDR_CMD(2);

    return true;
}
