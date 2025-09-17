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

#include "ssd1306_fonts.h"

#define OLED_I2C_ADDR 0x3C

static void _OLED_SendCommand(uint8_t command);
static void _OLED_SendData(uint8_t data);

void SHH_OLED_Init(void) { 

    _OLED_SendCommand(0xA8); // Set MUX Ratio 
    _OLED_SendCommand(0x3F); 

    _OLED_SendCommand(0xD3); // Set Display Offset 
    _OLED_SendCommand(0x00); 

    _OLED_SendCommand(0x40); // Set Display Start Line 

    // _OLED_SendCommand(0xA0); // 좌우반전
    _OLED_SendCommand(0xA1); // 좌우반전 

    // _OLED_SendCommand(0xC0); // 상하반전
    _OLED_SendCommand(0xC8); // 상하반전 
    
    _OLED_SendCommand(0xDA); // Set COM Pins hardware configuration 
    _OLED_SendCommand(0x02); 

    _OLED_SendCommand(0x81); // Set Contrast Control 
    _OLED_SendCommand(0x7F); 
    
    _OLED_SendCommand(0xA4); // Disable Entire Display On 
    
    _OLED_SendCommand(0xA6); // Set Normal Display; 
    
    _OLED_SendCommand(0xD5); // Set Osc Frequency
    _OLED_SendCommand(0x80); 
    
    _OLED_SendCommand(0x8D); // Enable charge pump regulator 
    _OLED_SendCommand(0x14); 
    
    _OLED_SendCommand(0xAF); // Display On 
}

void SHH_OLED_Clear(void) {
    uint8_t buffer[129];  // Control byte + 128 data
    buffer[0] = 0x40;     // Data mode
    for (uint32_t i = 1; i < 129; i++) {
        buffer[i] = 0x00;  // Clear: all pixels OFF (수정: 0xFF -> 0x00)
    }

    for (uint32_t page = 0; page < 8; page++) {
        _OLED_SendCommand(0xB0 + page); // Set Page Start Address (0-7)
        _OLED_SendCommand(0x00);        // Set Lower Column Start Address
        _OLED_SendCommand(0x10);        // Set Higher Column Start Address

        // 한 페이지(128바이트) 한 번에 전송 (효율화)
        SHD_LPI2C0_Write(OLED_I2C_ADDR, buffer, 129);
    }
}

void SHH_OLED_SetCursor(uint8_t x, uint8_t y) {
    _OLED_SendCommand(0xB0 + y);                 // Page start address
    _OLED_SendCommand(x & 0x0F);                 // Lower nibble of column address
    _OLED_SendCommand(0x10 + ((x >> 4) & 0x0F)); // Higher nibble of column address
}

void SHH_OLED_PrintChar(char c) {

    // 폰트 배열에 없는 문자는 공백으로 처리
    if (c < ' ' || c > '~') {
        c = ' ';
    }
    
    // 폰트 데이터(5바이트)를 OLED로 전송합니다.
    // 'c - ' ' 계산으로 해당 문자의 인덱스를 찾습니다.
    for (uint8_t i = 0; i < 5; i++) {
        _OLED_SendData(Font5x8[c - ' '][i]);
    }
    
    // 문자와 문자 사이를 구분하기 위해 공백 1픽셀을 추가합니다.
    _OLED_SendData(0x00);
}

void SHH_OLED_PrintString_5x8(const char* str) {

    while (*str) {
        SHH_OLED_PrintChar(*str++);
    }
}

static void _OLED_SendCommand(uint8_t command) {
    uint8_t buffer[2] = {0x00, command}; // Control Byte(0x00) + Command
    SHD_LPI2C0_Write(OLED_I2C_ADDR, buffer, 2);
}

static void _OLED_SendData(uint8_t data) {
    // [0]: Control Byte (0x40 = 데이터 전송)
    // [1]: 실제 데이터
    uint8_t buffer[2] = {0x40, data};
    
    // I2C 드라이버를 통해 2바이트 전송
    SHD_LPI2C0_Write(OLED_I2C_ADDR, buffer, 2);
}