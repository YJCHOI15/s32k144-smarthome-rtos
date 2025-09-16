#include "shh_display.h"
#include "drivers/gpio_driver.h"
#include "drivers/lpi2c_driver.h"
#include "drivers/lpit_driver.h"
#include "sh_config.h"
#include <string.h>


/** 
 *********************** FND ************************
 * Common Cathod -> 1써서 led 켬
*/

static uint8_t g_fnd_buffer[6] = {0,};
static uint8_t g_fnd_scan_digit = 0;

static const uint8_t g_fnd_patterns[10] = {
    // gfedcba
    0x3F, // 0 (0b00111111)
    0x06, // 1 (0b00000110)
    0x5B, // 2 (0b01011011)
    0x4F, // 3 (0b01001111)
    0x66, // 4 (0b01100110)
    0x6D, // 5 (0b01101101)
    0x7D, // 6 (0b01111101)
    0x07, // 7 (0b00000111)
    0x7F, // 8 (0b01111111)
    0x6F  // 9 (0b01101111)
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
static const sh_port_pin_t g_fnd_data_pins[7] = {
    PIN_FND_DATA_A, PIN_FND_DATA_B, PIN_FND_DATA_C, PIN_FND_DATA_D, PIN_FND_DATA_E, PIN_FND_DATA_F, PIN_FND_DATA_G
};
static const sh_port_pin_t g_fnd_sel_pins[6] = {
    PIN_FND_SEL1, PIN_FND_SEL2, PIN_FND_SEL3, PIN_FND_SEL4, PIN_FND_SEL5, PIN_FND_SEL6
};
#pragma GCC diagnostic pop


void SHH_FND_Init(void) {

    // FND 전부 끄기
    for(int i = 0; i < 6; i++) {
        SHD_GPIO_WritePin(g_fnd_sel_pins[i], 0); // Common Cathod
    }
}

void SHH_FND_BufferUpdate(uint32_t number) {

    // 밝기 저장: PIN_FND_SEL5, 6
    g_fnd_buffer[5] = number % 10;
    g_fnd_buffer[4] = (number / 10) % 10;
    
    // 습도 저장: PIN_FND_SEL3, 4
    g_fnd_buffer[3] = (number / 100) % 10;
    g_fnd_buffer[2] = (number / 1000) % 10;

    // 온도 저장: PIN_FND_SEL1, 2
    g_fnd_buffer[1] = (number / 10000) % 10;
    g_fnd_buffer[0] = (number / 100000) % 10;
}

void SHH_FND_Display(void) {

    // 1. 모든 자릿수 끄기
    for(int i = 0; i < 6; i++) {
        SHD_GPIO_WritePin(g_fnd_sel_pins[i], 0); // Common Cathod
    }

    // 2. 현재 자릿수에 해당하는 숫자 패턴 출력
    uint8_t pattern = g_fnd_patterns[g_fnd_buffer[g_fnd_scan_digit]];
    for(int i = 0; i < 7; i++) {
        if((pattern >> i) & 1) {
            SHD_GPIO_WritePin(g_fnd_data_pins[i], 1);
        } else {
            SHD_GPIO_WritePin(g_fnd_data_pins[i], 0);
        }
    }

    // 3. 현재 자릿수만 켜기
    SHD_GPIO_WritePin(g_fnd_sel_pins[g_fnd_scan_digit], 1);

    // 4. 다음 스캔을 위해 자릿수 이동
    g_fnd_scan_digit++;
    if (g_fnd_scan_digit >= 6) {
        g_fnd_scan_digit = 0;
    }
}


/** 
 *********************** OLED ************************
*/


