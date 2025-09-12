#ifndef SHH_DISPLAY_H
#define SHH_DISPLAY_H

#include <stdint.h>


void SHH_Display_Init(void);
void SHH_FND_NumberParsing(uint32_t number);
void SHH_FND_Display(void);
void SHH_OLED_Clear(void);
void SHH_OLED_PrintString(uint8_t line, uint8_t col, const char* str);


#endif /* SHH_DISPLAY_H */
