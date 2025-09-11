#include "gpio_driver.h"

static GPIO_Type * const g_gpio_bases[] = {PTA, PTB, PTC, PTD, PTE};

/**
 * 지정된 GPIO 핀의 데이터 방향을 초기화한다.
 * PDDR(Port Data Direction Register)의 해당 핀 비트를 설정하여
 * 핀을 입력(0) 또는 출력(1)으로 구성한다.
 */
void SHD_GPIO_InitPin(port_pin_t pin_info, pin_direction_t dir) {

    GPIO_Type* gpio = g_gpio_bases[pin_info.port];
    uint32_t pin = pin_info.pin;

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
void SHD_GPIO_WritePin(port_pin_t pin_info, uint8_t value) {

    GPIO_Type* gpio = g_gpio_bases[pin_info.port];
    uint32_t pin = pin_info.pin;

    if (value) {
        gpio->PSOR = (1UL << pin);
    } else {
        gpio->PCOR = (1UL << pin);
    }
}

/**
 * 지정된 입력 핀의 현재 논리 상태를 읽는다.
 * PDIR(Port Data Input Register)에서 해당 핀의 비트 값을 읽어온다.
 */
uint8_t SHD_GPIO_ReadPin(port_pin_t pin_info) {

    GPIO_Type* gpio = g_gpio_bases[pin_info.port];
    uint32_t pin = pin_info.pin;

    return (uint8_t)((gpio->PDIR >> pin) & 1UL);
}

/**
 * 지정된 출력 핀의 상태를 Toggle시킨다.
 * PTOR(Port Toggle Output Register)에 해당 핀의 비트를 쓴다.
 */
void SHD_GPIO_TogglePin(port_pin_t pin_info) {

    GPIO_Type* gpio = g_gpio_bases[pin_info.port];
    uint32_t pin = pin_info.pin;

    gpio->PTOR = (1UL << pin);
}
