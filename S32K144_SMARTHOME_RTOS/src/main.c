/*!
** Copyright 2020 NXP
** @file main.c
** @brief
**         Main module.
**         This module contains user's application code.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */


/* Including necessary configuration files. */
#include "sdk_project_config.h"

#include "sh_config.h"
#include "sh_tasks.h"

#include "shh_system.h"
#include "shh_uart.h"
#include "shh_led.h"
#include "shh_display.h"

int main(void)
{
    /* 모든 하드웨어 드라이버 초기화 */
	SHH_Init();
    SHH_LEDs_Init();
    SHH_FND_Init();
    // SHH_OLED_Init();

    /* RTOS 객체 생성 */
    g_command_queue = xQueueCreate(10, sizeof(command_msg_t));
    g_sensor_data_queue = xQueueCreate(10, sizeof(sensor_data_t));

    g_system_status_mutex = xSemaphoreCreateMutex();
    g_display_data_mutex = xSemaphoreCreateMutex();
    g_uart_mutex = xSemaphoreCreateMutex();

    g_button_interrupt_semaphore = xSemaphoreCreateBinary();
    g_uWave_semaphore = xSemaphoreCreateBinary();

    g_security_event_group = xEventGroupCreate();

    /* RTOS 태스크 생성 */
    xTaskCreate(SH_MainControl_Task, "MainCtrl", 2048, NULL, 5, NULL);
    xTaskCreate(SH_Sensor_Task, "Sensor", 512, NULL, 4, NULL);
    xTaskCreate(SH_ButtonInput_Task, "Button", 512, NULL, 6, NULL);
    xTaskCreate(SH_Display_Task, "Display", 1024, NULL, 3, NULL);
    // xTaskCreate(SH_SecurityEvent_Task, "Security", 256, NULL, 7, NULL);
    // xTaskCreate(SH_CanComm_Task, "CAN", 256, NULL, 4, NULL);

    /* RTOS 스케줄러 시작 */
    vTaskStartScheduler();

    for(;;);
    return 0;
}

/* END main */
/*!
** @}
*/
