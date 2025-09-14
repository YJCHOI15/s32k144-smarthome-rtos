#include "shh_uart.h"
#include "sh_config.h"
#include "drivers/lpuart_driver.h"
#include <stdio.h>  // vsnprintf
#include <stdarg.h> // va_list

void SHH_Uart_Printf(const char* format, ...) {

    char buffer[128]; // 로그 메시지를 담을 버퍼
    va_list args;

    // 뮤텍스를 사용하여 여러 태스크의 동시 접근을 막음
    if (xSemaphoreTake(g_uart_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        SHD_LPUART1_WriteString(buffer);
        
        xSemaphoreGive(g_uart_mutex);
    }
}