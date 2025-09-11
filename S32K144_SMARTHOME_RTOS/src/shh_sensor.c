#include "shh_sensor.h"
#include "drivers/adc_driver.h"
#include "drivers/lpi2c_driver.h"
#include "sh_config.h"


float SHH_ReadTemperature(void)
{
    uint8_t read_buffer[2];
    uint8_t cmd_measure[] = {0x2C, 0x06}; // 예시: 고정밀 측정 명령

    // 1. 측정 시작 명령 전송
    if (!SHD_LPI2C0_Write(SENSOR_TEMP_HUMI_ADDR, cmd_measure, 2))
    {
        return -1.0f; // 통신 실패
    }

    // 2. 센서가 측정할 동안 잠시 대기 (실제로는 RTOS 딜레이 사용 권장)
    for (volatile int i = 0; i < 20000; ++i);

    // 3. 측정된 데이터 읽기
    if (SHD_LPI2C0_Read(SENSOR_TEMP_HUMI_ADDR, read_buffer, 2))
    {
        int16_t temp_raw = (int16_t)((read_buffer[0] << 8) | read_buffer[1]);
        // 4. 원시(Raw) 데이터를 실제 온도 값으로 변환 (센서 데이터시트 참조)
        // 예시 공식: Temp (°C) = -45 + 175 * (temp_raw / 65535)
        float temperature = -45.0f + 175.0f * (float)temp_raw / 65535.0f;
        return temperature;
    }

    return -1.0f; // 통신 실패
}

float SHH_ReadHumidity(void)
{
    uint8_t read_buffer[2];
    uint8_t cmd_measure[] = {0x2C, 0x06}; // 예시: 고정밀 측정 명령

    // 실제로는 온도/습도가 함께 측정되므로, 이전 측정값을 활용하는 것이 효율적
    // 여기서는 설명을 위해 독립적으로 구현
    if (!SHD_LPI2C0_Write(SENSOR_TEMP_HUMI_ADDR, cmd_measure, 2))
    {
        return -1.0f;
    }

    for (volatile int i = 0; i < 20000; ++i);

    if (SHD_LPI2C0_Read(SENSOR_TEMP_HUMI_ADDR, read_buffer, 2))
    {
        uint16_t humi_raw = (uint16_t)((read_buffer[0] << 8) | read_buffer[1]);
        // 예시 공식: Humidity (%) = 100 * (humi_raw / 65535)
        float humidity = 100.0f * (float)humi_raw / 65535.0f;
        return humidity;
    }

    return -1.0f;
}

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
