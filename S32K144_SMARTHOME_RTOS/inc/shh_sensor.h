#ifndef SHH_SENSOR_H
#define SHH_SENSOR_H

#include <stdint.h>

/* 온습도 센서로부터 값 read */
uint8_t SHH_ReadTemperature(void);
uint8_t SHH_ReadHumidity(void);

/* CDS, VR 값 읽고 0~100% 범위 반환 */
uint8_t SHH_ReadBrightnessSensor(void);
uint8_t SHH_ReadManualControlVr(void);

/* 초음파 센서를 사용하여 거리를 측정 (cm) */
void SHH_uWave_StartMeasurement(void);
uint16_t SHH_uWave_GetDistanceCm(void);
void SHH_uWave_Echo_ISR_Handler(void);

#endif /* SHH_SENSOR_H */
