#include "drivers/lpuart_driver.h"

#define LPUART1_CLK_FREQ 8000000UL // LPUART1에 공급되는 클럭 소스 (SOSCDIV2_CLK @ 8MHz)

/**
 * LPUART1 모듈을 지정된 통신 속도로 초기화한다.
 */
void SHD_LPUART1_Init(uint32_t baud_rate) {
    uint16_t sbr; // Baud Rate Modulo Divisor

    /* 1. LPUART1 모듈에 클럭 활성화 (SOSCDIV2_CLK, 8MHz) */
    PCC->PCCn[PCC_LPUART1_INDEX] = PCC_PCCn_PCS(1)      /* PCS=1: Select SOSCDIV2_CLK */
                                 | PCC_PCCn_CGC_MASK; /* CGC=1: Clock enabled */

    /* 2. 송신(Transmitter) 및 수신(Receiver) 비활성화 */
    LPUART1->CTRL &= ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);

    /* 3. Baud Rate 설정 */
    /* Baud Rate = LPUART_CLK / ((OSR + 1) * SBR)
     * OSR(Oversampling Ratio)는 16배(기본값 15+1)로 가정
     * SBR = LPUART_CLK / (Baud Rate * 16)
     */
    sbr = (uint16_t)(LPUART1_CLK_FREQ / (baud_rate * 16));

    /* BAUD 레지스터에 SBR 값 설정 */
    LPUART1->BAUD = LPUART_BAUD_SBR(sbr);

    /* 4. 컨트롤 레지스터 설정: 8-N-1 */
    /* 8 데이터 비트, 패리티 없음, 1 스톱 비트, 인터럽트 비활성화 */
    LPUART1->CTRL = 0x00000000;

    /* 5. 송신(Transmitter) 및 수신(Receiver) 재활성화 */
    LPUART1->CTRL |= LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK;
}

/**
 * LPUART1을 통해 문자열을 전송한다.
 */
void SHD_LPUART1_WriteString(const char* str) {
    while (*str != '\0') {
        /* 1. Transmit Data Register Empty (TDRE) 플래그 대기 */
        while (!((LPUART1->STAT) & LPUART_STAT_TDRE_MASK));

        /* 2. 데이터 레지스터에 한 글자를 전송 */
        LPUART1->DATA = *str;

        /* 3. 다음 글자로 포인터를 이동 */
        str++;
    }
}
