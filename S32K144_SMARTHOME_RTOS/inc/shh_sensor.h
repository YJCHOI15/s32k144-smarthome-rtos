#ifndef SHH_SENSOR_H
#define SHH_SENSOR_H

#include <stdint.h>

/* 온습도 센서로부터 온도 값(°C)을 읽어온다. */
float SHH_ReadTemperature(void);

/* 온습도 센서로부터 습도 값(%)을 읽어온다. */
float SHH_ReadHumidity(void);

/* CDS의 값을 읽어 0-100% 범위로 반환한다. */
uint8_t SHH_ReadBrightnessSensor(void);

/* VR의 값을 읽어 0-100% 범위로 반환한다. */
uint8_t SHH_ReadManualControlVr(void);

#endif /* SHH_SENSOR_H */
