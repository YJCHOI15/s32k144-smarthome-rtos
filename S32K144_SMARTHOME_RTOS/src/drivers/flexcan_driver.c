#include "drivers/flexcan_driver.h"
#include "shh_uart.h"
#include <stddef.h>



/* 수신 콜백 함수를 저장하기 위한 정적 변수 */
static can_rx_callback_t g_can_rx_callback = NULL;

void SHD_CAN0_Init(void) {
    /* 1. PCC 클럭 활성화 */
    PCC->PCCn[PCC_FlexCAN0_INDEX] =
        PCC_PCCn_PCS(1) |   // PCS=1 → SOSCDIV2_CLK (8 MHz)
        PCC_PCCn_CGC_MASK;  // 클럭 게이트 ON

    /* 2. Freeze 모드 진입 */
    CAN0->MCR |= CAN_MCR_MDIS_MASK;   // 모듈 디스에이블
    CAN0->MCR &= ~CAN_MCR_MDIS_MASK;  // 재활성화
    CAN0->MCR |= (CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK); // Freeze 요청
    while(!(CAN0->MCR & CAN_MCR_FRZACK_MASK));           // Freeze 진입 확인

    /* 3. 비트 타이밍: 500 kbps @ 8 MHz */
    CAN0->CTRL1 = CAN_CTRL1_PRESDIV(0)   // 분주 ×1 → 8 MHz
                | CAN_CTRL1_PROPSEG(1)   // 2 Tq
                | CAN_CTRL1_PSEG1(5)     // 6 Tq
                | CAN_CTRL1_PSEG2(6)     // 7 Tq
                | CAN_CTRL1_RJW(3);      // 재동기 점프 폭

    /* 4. 메시지 버퍼 초기화 */
    for(int i=0; i<32; i++) {
        CAN0->RAMn[i*4 + 0] = 0; // CS
        CAN0->RAMn[i*4 + 1] = 0; // ID
        CAN0->RAMn[i*4 + 2] = 0; // DATA[0..3]
        CAN0->RAMn[i*4 + 3] = 0; // DATA[4..7]
    }

    /* 5. 수신 MB 설정 (MB4를 Rx 전용) */
    CAN0->RAMn[4*4 + 1] = (0x123 << 18); // 수신 ID
    CAN0->RAMn[4*4 + 0] = (4 << 24);     // CODE=4 (Rx Empty)

    /* 6. 송신 MB 설정 (MB0를 Tx 전용) */
    CAN0->RAMn[0*4 + 0] = (8 << 24); // CODE=8 (Tx Inactive)

    /* 7. Normal 모드 진입 */
    CAN0->MCR &= ~CAN_MCR_HALT_MASK;  
    while(CAN0->MCR & CAN_MCR_FRZACK_MASK); // Freeze 해제 확인
}

void SHD_CAN0_Transmit(uint32_t id, uint8_t* data, uint8_t dlc) {
    /* ID 세팅 */
    CAN0->RAMn[0*4 + 1] = (id << 18);

    /* 데이터 세팅 */
    CAN0->RAMn[0*4 + 2] = (data[0] << 24) | (data[1] << 16) |
                          (data[2] << 8)  | (data[3]);
    CAN0->RAMn[0*4 + 3] = (data[4] << 24) | (data[5] << 16) |
                          (data[6] << 8)  | (data[7]);

    /* CS(DLC + CODE=Transmit) */
    CAN0->RAMn[0*4 + 0] = (0xC << 24) | (dlc << 16); // CODE=0xC (Tx Data Frame)
}

/**
 * CAN 메시지 수신 시 호출될 콜백 함수를 등록한다.
 */
void SHD_CAN0_RegisterRxCallback(can_rx_callback_t callback) {
    g_can_rx_callback = callback;
}


/**
 * FlexCAN0 Message Buffer 0-15번 그룹의 인터럽트 핸들러
 * 이 함수는 startup.s 파일의 벡터 테이블에 등록되어 있다.
 */
void CAN0_ORed_0_15_MB_IRQHandler(void) {

}