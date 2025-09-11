#include "gpio_driver.h"

/**
 * 지정된 GPIO 핀의 데이터 방향을 초기화한다.
 * PDDR(Port Data Direction Register)의 해당 핀 비트를 설정하여
 * 핀을 입력(0) 또는 출력(1)으로 구성한다.
 */
void SHD_GPIO_InitPin(GPIO_Type *gpio, uint32_t pin, pin_direction_t dir) {

    if (dir == GPIO_OUTPUT) {
        gpio->PDDR |= (1UL << pin);
    } else {
        gpio->PDDR &= ~(1UL << pin);
    }
}

/**
 * 지정된 출력 핀에 1 또는 0 값을 쓴다.
 * PSOR(Set)/PCOR(Clear) 레지스터를 사용한다.
 */
void SHD_GPIO_WritePin(GPIO_Type *gpio, uint32_t pin, uint8_t value) {

    if (value)
    {
        gpio->PSOR = (1UL << pin);
    } else {
        gpio->PCOR = (1UL << pin);
    }
}

/**
 * 지정된 입력 핀의 현재 논리 상태를 읽는다.
 * PDIR(Port Data Input Register)에서 해당 핀의 비트 값을 읽어온다.
 */
uint8_t SHD_GPIO_ReadPin(GPIO_Type *gpio, uint32_t pin) {

    return (uint8_t)((gpio->PDIR >> pin) & 1UL);
}

/**
 * 지정된 출력 핀의 상태를 Toggle시킨다.
 * PTOR(Port Toggle Output Register)에 해당 핀의 비트를 쓴다.
 */
void SHD_GPIO_TogglePin(GPIO_Type *gpio, uint32_t pin) {
    gpio->PTOR = (1UL << pin);
}
