#include "shh_sensor.h"
#include "drivers/gpio_driver.h"
#include "drivers/adc_driver.h"
#include "drivers/lpit_driver.h"
#include "sh_config.h"
#include <stdbool.h>


/**
 * DHT11 온습도 센서 통신과정 
 * 1. 통신개시 알림: mcu가 센서로 18ms 동안 LOW 유지하고 
 *                  HIGH로 전환해서 20-40ms 기다림
 *                  이 신호를 받으면 DHT11는 대기상태에서 깨어남
 * 2. 응답신호: DHT11가 대기상태에서 깨어나 80us LOW 신호 보내고
 *             다시 80us HIGH를 보냄으로써 준비가 됐음을 mcu에 알림
 * 3. 온습도 정보 송신: DHT11가 40bit의 데이터열을 보낸다.
 *                     HIGH 26~28us : 0
 *                     LOW  70us    : 1    -> HIGH의 길이로 0, 1 정의
 * 4. 데이터 수신 및 파싱: 습도(16bit) + 온도(16bit) + 체크섬(8bit) 수신
 *                        각 16비트의 상위 8비트가 값을 의미한다.
 *                        하위 8비트는 소수점이나 DHT11은 지원하지 않는다.
 *                        온습도 값을 더한 값이 체크섬인데 
 *                        일치하지 않으면 오류가 발생했음을 의미한다.  
 */

/* 온습도 센서 통신을 위한 내부 함수 선언 */
static bool _SHH_Read_DHT11_40bit_Data(uint8_t* data);

static uint8_t g_last_temperature = -99;
static uint8_t g_last_humidity = -99;

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

    // 1. 통신 시작 신호 (MCU -> DHT11)
    SHD_GPIO_InitPin(PIN_TEMP_HUMI, GPIO_OUTPUT);
    SHD_GPIO_WritePin(PIN_TEMP_HUMI, 0);
    SHD_LPIT0_CH3_DelayUs(18000);
    SHD_GPIO_WritePin(PIN_TEMP_HUMI, 1);
    SHD_LPIT0_CH3_DelayUs(30);
    SHD_GPIO_InitPin(PIN_TEMP_HUMI, GPIO_INPUT);


    // 2. DHT11 응답 신호 확인 (DHT11 -> MCU)
    timeout_counter = 100;
    while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 1) {
        if (timeout_counter-- == 0) return false; 
        SHD_LPIT0_CH3_DelayUs(1);
    }
    timeout_counter = 100;
    while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 0) {
        if (timeout_counter-- == 0) return false; 
        SHD_LPIT0_CH3_DelayUs(1);
    }
    // 데이터 전송 시작을 위해 핀이 다시 Low가 될 때까지 대기
    timeout_counter = 100;
    while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 1) {
        if (timeout_counter-- == 0) return false; 
        SHD_LPIT0_CH3_DelayUs(1);
    }

    // 3. 40비트 데이터 수신 (DHT11 -> MCU)
    for (int i = 0; i < 40; i++) {
        // 비트 시작 (Low)
        timeout_counter = 100;
        while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 0) {
            if (timeout_counter-- == 0) return false; 
            SHD_LPIT0_CH3_DelayUs(1);
        }

        // 비트 길이 측정 (High)
        uint32_t high_duration = 0;
        while(SHD_GPIO_ReadPin(PIN_TEMP_HUMI) == 1) {
            high_duration++;
            SHD_LPIT0_CH3_DelayUs(1);
            if (high_duration > 200) return false; 
        }

        // 왼쪽 시프트로 비트 자리 마련
        current_byte <<= 1;
        
        // 26-28us보다 길면 1, 짧으면 0
        if (high_duration > 40) {
            current_byte |= 1;
        }

        bit_count++;
        if (bit_count == 8) {
            data[i/8] = current_byte;
            bit_count = 0;
            current_byte = 0;
        }
    }

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
