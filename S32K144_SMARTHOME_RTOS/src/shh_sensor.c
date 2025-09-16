#include "shh_sensor.h"
#include "shh_uart.h"
#include "sh_config.h"

#include "drivers/gpio_driver.h"
#include "drivers/adc_driver.h"
#include "drivers/lpit_driver.h"



/* 온습도 센서 통신을 위한 내부 함수 선언 */
static bool _SHH_Read_DHT11_40bit_Data(uint8_t* data);

static uint8_t g_last_temperature = 0;
static uint8_t g_last_humidity = 0;

uint8_t SHH_ReadTemperature(void) {

    uint8_t buffer[5] = {0,};
    if (_SHH_Read_DHT11_40bit_Data(buffer)) {
        g_last_humidity = buffer[0];
        g_last_temperature = buffer[2];
    }
    // 실패해도 마지막 성공값 반환
    return g_last_temperature;
}

uint8_t SHH_ReadHumidity(void) {
    // SHH_ReadTemperature가 먼저 호출되어 값을 업데이트했다고 가정
    // 이렇게 하면 DHT11과의 통신은 한 번만 수행된다. 
    return g_last_humidity;
}

/* DHT11 센서로부터 40비트 데이터를 읽어오는 내부 함수 */
static bool _SHH_Read_DHT11_40bit_Data(uint8_t* data) {

    uint8_t bit_count = 0;
    uint8_t current_byte = 0;
    uint32_t timeout_counter;

    /**
     * DHT11 온습도 센서 프로토콜
     * 1. 통신개시 알림: mcu가 센서로 18ms 동안 LOW 유지하고 
     *                  HIGH로 전환해서 20-40ms 기다림
     *                  이 신호를 받으면 DHT11는 대기상태에서 깨어남
     * 2. 응답신호: DHT11가 대기상태에서 깨어나 80us LOW 신호 보내고
     *             다시 80us HIGH를 보냄으로써 준비가 됐음을 mcu에 알림
     * 3. 온습도 정보 송신: DHT11가 40bit의 데이터열을 보낸다.
     *                     LOW          : HIGH 보내기 전
     *                     HIGH 26~28us : 0
     *                     HIGH 70us    : 1    -> HIGH의 길이로 0, 1 정의
     * 4. 데이터 수신 및 파싱: 습도(16bit) + 온도(16bit) + 체크섬(8bit) 수신
     *                        각 16비트의 상위 8비트가 값을 의미한다.
     *                        하위 8비트는 소수점이나 DHT11은 지원하지 않는다.
     *                        온습도 값을 더한 값이 체크섬인데 
     *                        일치하지 않으면 오류가 발생했음을 의미한다.  
     */

    // 1. 통신 시작 신호
    SHD_GPIO_InitPin(PIN_TEMP_HUMI, GPIO_OUTPUT);
    SHD_GPIO_WritePin(PIN_TEMP_HUMI, 0);
    SHD_GPIO_WritePin(PIN_LED1, 0);       ////////////////////// LOW
    SHD_LPIT0_DelayUs(3, 18000);
    SHD_GPIO_WritePin(PIN_TEMP_HUMI, 1);  ////////////////////// HIGH
    SHD_GPIO_WritePin(PIN_LED1, 1);
    SHD_LPIT0_DelayUs(3, 5);
    SHD_GPIO_InitPin(PIN_TEMP_HUMI, GPIO_INPUT);


    // 2. DHT11 응답 신호 확인 (80us Low -> 80us High)
    timeout_counter = 100;
    while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 1) {  // DHT11 센서 주기에 따라 응답 안할 수도 있음
        if (timeout_counter-- == 0) {
            SHD_GPIO_WritePin(PIN_LED1, 1);
            return false;
        }   
        SHD_LPIT0_DelayUs(3, 1);
    }
    ////////////////////// LOW
    SHD_GPIO_WritePin(PIN_LED1, 0);

    while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 0); // 응답 HIGH 신호 
    ////////////////////// HIGH
    SHD_GPIO_WritePin(PIN_LED1, 1);

    // 첫 비트 시작 LOW
    while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 1);
    ////////////////////// LOW
    SHD_GPIO_WritePin(PIN_LED1, 0);

    // 3. 40비트 데이터 수신 (각 비트 50us Low -> 26~28us (0) / 70us High (1))
    for (uint8_t i = 0; i < 40; i++) {

        while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 0);
        ////////////////////// HIGH
        /////////////////////////////////////////// 여기서부터
        SHD_GPIO_WritePin(PIN_LED1, 1);

        uint8_t high_duration = 0;
        while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 1) {
            high_duration++;
            SHD_LPIT0_DelayUs(3, 10);
        }
        ////////////////////// LOW
        /////////////////////////////////////////// 여기까지 시간 재야 함
        SHD_GPIO_WritePin(PIN_LED1, 0);

        // 왼쪽 시프트로 비트 자리 마련
        current_byte <<= 1;
        
        // 26-28us보다 길면 1, 짧으면 0
        // 10us 실제 길이 약 15us
        if (high_duration > 3) {
            current_byte |= 1;
        }

        bit_count++;
        if (bit_count == 8) {
            data[i/8] = current_byte;
            bit_count = 0;
            current_byte = 0;
        }
    }

    while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 0);
    ////////////////////// HIGH
    SHD_GPIO_WritePin(PIN_LED1, 1);

    // 4. 체크섬 검증
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        return true;
    }

    return false;
}

/**
 ************************ CDS & VR ***************************
 */

uint8_t SHH_ReadBrightnessSensor(void) {

    uint16_t raw_value = SHD_ADC0_ReadChannel(4);

    // ADC는 12비트(0-4095) 해상도를 가짐
    // 주변이 밝을수록 CDS 저항이 낮아져 ADC 값이 높아진다고 가정
    // 0-4095 범위를 0-100% 범위로 변환
    uint8_t percentage = (uint8_t)((raw_value * 100) / 4095);

    return percentage;
}

uint8_t SHH_ReadManualControlVr(void) {

    uint16_t raw_value = SHD_ADC0_ReadChannel(5);
    uint8_t percentage = (uint8_t)((raw_value * 100) / 4095);

    return percentage;
}

/**
 ************************ uWave ***************************
 */
static volatile uint16_t g_ftm1_start_time = 0;
static volatile uint16_t g_ftm1_end_time = 0;
static volatile bool g_is_measurement_done = false;

/**
 * 초음파 센서용 FTM1 타이머를 프리러닝 카운터로 초기화한다.
 */
void SHH_uWave_Init(void) {

    // 1. FTM1 모듈에 클럭 활성화 (SOSCDIV2_CLK, 8MHz)
    PCC->PCCn[PCC_FTM1_INDEX] = PCC_PCCn_PCS(1) | PCC_PCCn_CGC_MASK;

    // 2. FTM1 카운터 설정
    FTM1->MODE |= FTM_MODE_WPDIS_MASK; // 쓰기 방지 해제
    FTM1->SC = 0; // 카운터 정지 상태에서 설정
    FTM1->CNTIN = 0;
    FTM1->MOD = 0xFFFF; // 최대값까지 카운트
    FTM1->SC = FTM_SC_PS(3) | FTM_SC_CLKS(1); // Prescaler=8, 1MHz 카운터 (1us당 1카운트)
    FTM1->MODE &= ~FTM_MODE_WPDIS_MASK; // 쓰기 방지 재활성화
}

/**
 * 초음파 거리 측정을 시작한다.
 */
void SHH_uWave_StartMeasurement(void) {
    g_is_measurement_done = false;

    // 1. Echo 핀을 상승 에지 인터럽트로 설정
    SHD_PORT_SetPinIT(PIN_UWAVE_ECHO, PORT_IT_IRQ_RISING);

    // 2. Trig 핀에 10us High 펄스 전송
    SHD_GPIO_WritePin(PIN_UWAVE_TRIG, 1);
    SHD_LPIT0_DelayUs(3, 10); // LPIT 채널 3을 이용한 10us 정밀 지연
    SHD_GPIO_WritePin(PIN_UWAVE_TRIG, 0);
}

/**
 * 가장 최근 측정된 거리 값을 반환한다.
 */
uint16_t SHH_uWave_GetDistanceCm(void) {

    if (!g_is_measurement_done) {
        return 0xFFFF; // 아직 측정 완료 전
    }

    uint32_t pulse_duration_us = 0;
    if (g_ftm1_end_time >= g_ftm1_start_time) {
        pulse_duration_us = g_ftm1_end_time - g_ftm1_start_time;
    } else {
        // 카운터 오버플로우 처리
        pulse_duration_us = (0xFFFF - g_ftm1_start_time) + g_ftm1_end_time;
    }

    // 1us 당 1카운트이므로, pulse_duration_us는 시간과 같음
    return (uint16_t)(pulse_duration_us / 58);
}


/**
 * 초음파 Echo 핀의 상태 변화를 처리하는 함수 (PORTC_IRQHandler에서 호출됨)
 */
void SHH_uWave_Echo_ISR_Handler(void) {

    // 상승 에지 감지 (펄스 시작)
    if (SHD_GPIO_ReadPin(PIN_UWAVE_ECHO) == 1) {
        g_ftm1_start_time = FTM1->CNT; // 타이머 시작 시간 기록
        // 다음 인터럽트는 하강 에지에서 발생하도록 변경
        SHD_PORT_SetPinIT(PIN_UWAVE_ECHO, PORT_IT_IRQ_FALLING);
    }
    // 하강 에지 감지 (펄스 종료)
    else {
        g_ftm1_end_time = FTM1->CNT; // 타이머 종료 시간 기록
        g_is_measurement_done = true; // 측정 완료 플래그 설정
        SHD_PORT_SetPinIT(PIN_UWAVE_ECHO, PORT_IT_DISABLED); // 다음 측정을 위해 인터럽트 비활성화

        // SecurityEvent_Task에 측정 완료 신호 전송
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(g_uWave_semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}