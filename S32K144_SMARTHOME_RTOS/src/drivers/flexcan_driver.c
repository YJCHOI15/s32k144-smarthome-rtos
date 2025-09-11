#include "flexcan_driver.h"
#include <stddef.h>

/* Message Buffer - CS (Control/Status) Word */
#define CAN_CS_CODE_SHIFT   (24)
#define CAN_CS_CODE_MASK    (0x0F000000U)
#define CAN_CS_DLC_SHIFT    (16)
#define CAN_CS_DLC_MASK     (0x000F0000U)

/* Message Buffer - ID Word */
#define CAN_ID_STD_SHIFT    (18)
#define CAN_ID_STD_MASK     (0x1FFC0000U)

#define CAN_CS_CODE(x)      (((uint32_t)(((uint32_t)(x)) << CAN_CS_CODE_SHIFT)) & CAN_CS_CODE_MASK)
#define CAN_CS_DLC(x)       (((uint32_t)(((uint32_t)(x)) << CAN_CS_DLC_SHIFT)) & CAN_CS_DLC_MASK)
#define CAN_ID_STD(x)       (((uint32_t)(((uint32_t)(x)) << CAN_ID_STD_SHIFT)) & CAN_ID_STD_MASK)

/*
 * S32K144의 CAN Message Buffer는 CAN0->RAMn 메모리 영역에 위치한다.
 */
typedef volatile struct {
    uint32_t CS;
    uint32_t ID;
    uint8_t  DATA_BYTE[8];
    uint32_t WORD2;
    uint32_t WORD3;
} CAN_MB_t;

/* CAN0의 RAMn 영역을 Message Buffer 구조체 배열 포인터로 캐스팅 */
#define CAN0_MB ((CAN_MB_t*) &CAN0->RAMn[0])


/* 수신 콜백 함수를 저장하기 위한 정적 변수 */
static can_rx_callback_t g_can_rx_callback = NULL;

/**
 * FlexCAN0 모듈을 500kbps 속도로 초기화한다.
 */
void SHD_CAN0_Init(void) {
    /* 1. FlexCAN0 모듈에 클럭 활성화 (SOSCDIV2_CLK, 8MHz) */
    PCC->PCCn[PCC_FlexCAN0_INDEX] = PCC_PCCn_PCS(1) | PCC_PCCn_CGC_MASK;

    /* 2. FlexCAN 모듈 비활성화 및 클럭 소스 선택 */
    CAN0->MCR |= CAN_MCR_MDIS_MASK;
    CAN0->CTRL1 &= ~CAN_CTRL1_CLKSRC_MASK; /* CLKSRC=0: SOSCDIV2_CLK 선택 */
    CAN0->MCR &= ~CAN_MCR_MDIS_MASK;

    /* 3. Freeze Mode 진입 */
    CAN0->MCR |= CAN_MCR_HALT_MASK;
    while (!((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT));

    /* 4. Bit Timing 설정 (500kbps) */
    /* Baud Rate = fCANCLK / ( (PRESDIV+1) * (1 + PSEG1 + PSEG2 + PROPSEG) )
     * 500,000 = 8,000,000 / ( (0+1) * (1 + 7 + 3 + 5) ) = 8,000,000 / 16
     */
    CAN0->CTRL1 = CAN_CTRL1_PRESDIV(0) | CAN_CTRL1_PSEG1(7) | CAN_CTRL1_PSEG2(3) | CAN_CTRL1_PROPSEG(5) | CAN_CTRL1_RJW(3);

    /* 5. Message Buffer (MB) 설정 */
    /* MB0: 수신용, 모든 ID 수신 */
    CAN0_MB[0].CS = CAN_CS_CODE(0x4); // CODE=4: Rx, Empty

    CAN0->RXMGMASK = 0x00000000;         // 모든 ID를 수신하도록 마스크 해제

    /* MB1: 송신용 */
    CAN0_MB[1].CS = CAN_CS_CODE(0x8); // CODE=8: Tx, Inactive

    /* 6. 수신 인터럽트 활성화 */
    CAN0->IMASK1 |= (1UL << 0); // MB0에 대한 인터럽트 활성화

    /* 7. Freeze Mode 해제 */
    CAN0->MCR &= ~CAN_MCR_HALT_MASK;
    while ((CAN0->MCR & CAN_MCR_FRZACK_MASK) >> CAN_MCR_FRZACK_SHIFT);
}

/**
 * CAN 메시지를 전송한다.
 */
bool SHD_CAN0_Transmit(uint32_t id, uint8_t* data, uint8_t dlc) {
    uint32_t i;
    if (dlc > 8) return false;

    /* 1. 송신 MB(MB1)가 비어있는지 확인 (Polling 방식) */
    while(((CAN0_MB[1].CS & CAN_CS_CODE_MASK) >> CAN_CS_CODE_SHIFT) != 0x8);

    /* 2. 데이터 및 ID 설정 */
    CAN0_MB[1].ID = CAN_ID_STD(id);
    for (i = 0; i < dlc; i++) {
        CAN0_MB[1].DATA_BYTE[i] = data[i];
    }

    /* 3. 전송 시작 */
    /* DLC와 CODE를 설정하여 메시지 전송 활성화 */
    CAN0_MB[1].CS = CAN_CS_CODE(0xC) | CAN_CS_DLC(dlc);

    return true;
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
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
    uint32_t i;

    /* 수신 MB(MB0)에서 인터럽트가 발생했는지 확인 */
    if (CAN0->IFLAG1 & (1UL << 0)) {
        /* 1. 데이터 추출 */
        id = (CAN0_MB[0].ID & CAN_ID_STD_MASK) >> CAN_ID_STD_SHIFT;
        dlc = (CAN0_MB[0].CS & CAN_CS_DLC_MASK) >> CAN_CS_DLC_SHIFT;
        for (i = 0; i < dlc; i++) {
            data[i] = CAN0_MB[0].DATA_BYTE[i];
        }

        /* 2. 등록된 콜백 함수 호출 */
        if (g_can_rx_callback != NULL) {
            g_can_rx_callback(id, data, dlc);
        }

        /* 3. 인터럽트 플래그 클리어 */
        CAN0->IFLAG1 = (1UL << 0);
    }

    /* 4. MB 잠금 해제 */
    (void)CAN0->TIMER;
}