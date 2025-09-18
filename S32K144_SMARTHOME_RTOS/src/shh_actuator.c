#include "shh_actuator.h"
#include "sh_config.h"
#include "drivers/gpio_driver.h"
#include "drivers/ftm_driver.h"

#include <FreeRTOS.h>
#include <task.h>

/****** Fan (DC Motor) ******/

void SHH_Fan_On(void) {
    SHD_GPIO_WritePin(PIN_DC_MOTOR, 1);
}

void SHH_Fan_Off(void) {
    SHD_GPIO_WritePin(PIN_DC_MOTOR, 0);
}


/****** Door Lock (Servo Motor) ******/

// 서보 모터 제어를 위한 Duty Cycle 값 정의 (50Hz 주기 기준)
// 0도   => 1ms 펄스폭 => (1ms / 20ms) * 100 = 5% 듀티 사이클
// 90도  => 1.5ms 펄스폭 => (1.5ms / 20ms) * 100 = 7.5% (8로 반올림)
// 1ms와 1.5ms는 서보 모터를 제어하는 표준화된 신호 길이다.

static uint8_t _angleToDuty(uint8_t angle);

void SHH_DoorLock_Open(void) {
    // 오른쪽 90도
    uint8_t duty = _angleToDuty(90);
    SHD_FTM0_SetDutyCycle(2, duty);  // duty = 2%
}

void SHH_DoorLock_Close(void) {
    // 왼쪽 0도 (제자리)
    uint8_t duty = _angleToDuty(0);
    SHD_FTM0_SetDutyCycle(2, duty);  // duty = 7%
}

static uint8_t _angleToDuty(uint8_t angle) {
    // angle: 0~90
    // duty: 7% (왼쪽) → 2% (오른쪽)
    return (uint8_t)(7 - ((angle * 5) / 90));
}


/****** Blind (Step Motor) ******/

// 스텝 모터 제어를 위한 step 시퀀스
static const uint8_t step_sequence[4][4] = {
    {1, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 0, 1}
};
static int32_t g_current_step = 0; // 현재 스텝 위치 추적

void SHH_Blinds_Move(int32_t steps) {

    int direction = (steps > 0) ? -1 : 1;
    uint32_t num_steps = (steps > 0) ? steps : -steps;

    for (uint32_t i = 0; i < num_steps; i++) {
        g_current_step += direction;

        // 시퀀스 인덱스가 0-3 범위를 벗어나지 않도록 처리
        if (g_current_step > 3) g_current_step = 0;
        if (g_current_step < 0) g_current_step = 3;

        SHD_GPIO_WritePin(PIN_STEP_MOTOR1, step_sequence[g_current_step][0]);
        SHD_GPIO_WritePin(PIN_STEP_MOTOR2, step_sequence[g_current_step][1]);
        SHD_GPIO_WritePin(PIN_STEP_MOTOR3, step_sequence[g_current_step][2]);
        SHD_GPIO_WritePin(PIN_STEP_MOTOR4, step_sequence[g_current_step][3]);

        /*
         * 스텝 모터가 물리적으로 회전할 시간을 확보하기 위해
         * 이 곳에 약 2ms ~ 10ms 정도의 delay 코드가 반드시 필요
         */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    SHD_GPIO_WritePin(PIN_STEP_MOTOR1, 0);
    SHD_GPIO_WritePin(PIN_STEP_MOTOR2, 0);
    SHD_GPIO_WritePin(PIN_STEP_MOTOR3, 0);
    SHD_GPIO_WritePin(PIN_STEP_MOTOR4, 0);
}


/****** External Power (Relay) ******/

void SHH_ExternalPower_On(void) {
    SHD_GPIO_WritePin(PIN_RELAY, 1);
}

void SHH_ExternalPower_Off(void) {
    SHD_GPIO_WritePin(PIN_RELAY, 0);
}
