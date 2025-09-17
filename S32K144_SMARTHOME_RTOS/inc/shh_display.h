#ifndef SHH_DISPLAY_H
#define SHH_DISPLAY_H

#include <stdint.h>


void SHH_FND_Init(void);
void SHH_FND_BufferUpdate(uint32_t number);
void SHH_FND_Display(void);

void SHH_OLED_Init(void);
void SHH_OLED_Clear(void);
void SHH_OLED_SetCursor(uint8_t x, uint8_t y);
void SHH_OLED_PrintChar(char c);
void SHH_OLED_PrintString_5x8(const char* str);

#endif /* SHH_DISPLAY_H */
