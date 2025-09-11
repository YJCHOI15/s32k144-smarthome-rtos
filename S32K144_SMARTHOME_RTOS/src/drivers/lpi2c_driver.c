#include "lpi2c_driver.h"

#define LPI2C_TIMEOUT 1000

/**
 * LPI2C0 모듈을 마스터 모드, 100kHz 속도로 초기화한다.
 */
void SHD_LPI2C0_Init(void) {
    /* 1. LPI2C0 모듈에 클럭 활성화 (SOSCDIV2_CLK, 8MHz) */
    PCC->PCCn[PCC_LPI2C0_INDEX] = PCC_PCCn_PCS(1)      /* PCS=1: Select SOSCDIV2_CLK */
                                | PCC_PCCn_CGC_MASK; /* CGC=1: Clock enabled */

    /* 2. LPI2C0 마스터 모듈 리셋 */
    LPI2C0->MCR |= LPI2C_MCR_RST_MASK; // 모듈 리셋
    LPI2C0->MCR = 0x00000000;          // 리셋 해제 및 기본 설정

    /* 3. I2C 100kHz (Standard Mode)를 위한 클럭 설정 */
    /* Baud Rate = (Source Clock / (2^PRESCALE)) / (CLKLO + CLKHI + 2)
     * 100,000 = (8,000,000 / (2^1)) / (19 + 19 + 2)
     */
    LPI2C0->MCCR0 = LPI2C_MCCR0_CLKLO(19) | LPI2C_MCCR0_CLKHI(19);

    /* Prescaler 설정 */
    LPI2C0->MCFGR1 = LPI2C_MCFGR1_PRESCALE(1); // Prescaler = 2

    /* 4. LPI2C0 마스터 모드 활성화 */
    LPI2C0->MCR |= LPI2C_MCR_MEN_MASK;
}

/**
 * I2C 버스를 통해 특정 주소의 슬레이브 장치에 데이터를 쓴다.
 */
bool SHD_LPI2C0_Write(uint8_t slave_addr, uint8_t* data, uint32_t len) {
    uint32_t i;
    uint32_t timeout = 0;

    /* 1. START 신호 및 슬레이브 주소(Write) 전송 */
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b0100) | LPI2C_MTDR_DATA(slave_addr << 1);

    /* 2. 데이터 바이트 전송 */
    for (i = 0; i < len; ++i) {
        timeout = 0;
        /* 전송 데이터 레지스터가 비워질 때까지 대기 (TDRE 플래그) */
        while (!((LPI2C0->MSR) & LPI2C_MSR_TDF_MASK)) {
            timeout++;
            if (timeout > LPI2C_TIMEOUT) return false;
        }

        /* 데이터 전송 */
        LPI2C0->MTDR = LPI2C_MTDR_CMD(0b0001) | LPI2C_MTDR_DATA(data[i]);

        /* NACK(Not Acknowledge) 수신 시 전송 중단 */
        if (LPI2C0->MSR & LPI2C_MSR_NDF_MASK) {
            LPI2C0->MSR |= LPI2C_MSR_NDF_MASK; // NACK 플래그 클리어
            return false;
        }
    }

    /* 3. STOP 신호 전송 */
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b0010);

    return true;
}

/**
 * I2C 버스를 통해 특정 주소의 슬레이브 장치로부터 데이터를 읽는다.
 */
bool SHD_LPI2C0_Read(uint8_t slave_addr, uint8_t* buffer, uint32_t len) {
    uint32_t i;
    uint32_t timeout = 0;

    /* 1. START 신호 및 슬레이브 주소(Read) 전송 */
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b0100) | LPI2C_MTDR_DATA((slave_addr << 1) | 1);

    /* 2. 읽을 데이터 길이 전송 */
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b0001) | LPI2C_MTDR_DATA(len - 1);

    /* 3. 데이터 수신 */
    for (i = 0; i < len; ++i) {
        timeout = 0;
        /* 데이터 수신 레지스터에 데이터가 들어올 때까지 대기 (RDRF 플래그) */
        while (!((LPI2C0->MSR) & LPI2C_MSR_RDF_MASK))
        {
            timeout++;
            if (timeout > LPI2C_TIMEOUT) return false;
        }
        buffer[i] = (uint8_t)LPI2C0->MRDR;
    }

    /* 4. STOP 신호 전송 */
    LPI2C0->MTDR = LPI2C_MTDR_CMD(0b0010);

    return true;
}
