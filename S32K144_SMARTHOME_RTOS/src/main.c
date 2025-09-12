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
#include "shh_system.h"
#include "sh_tasks.h"

int main(void)
{
    /* 모든 하드웨어 드라이버 초기화 */
    SHH_Init();

    /* RTOS 객체 생성 */
    g_command_queue = xQueueCreate(10, sizeof(command_msg_t));
    g_sensor_data_queue = xQueueCreate(1, sizeof(sensor_data_t));
    g_display_data_queue = xQueueCreate(1, sizeof(display_data_t));
    g_system_status_mutex = xSemaphoreCreateMutex();
    g_security_event_group = xEventGroupCreate();
    g_button_interrupt_semaphore = xSemaphoreCreateBinary();

    /* RTOS 태스크 생성 */
    xTaskCreate(SH_MainControl_Task, "MainCtrl", 512, NULL, 5, NULL);
    xTaskCreate(SH_Sensor_Task, "Sensor", 256, NULL, 4, NULL);
    xTaskCreate(SH_ButtonInput_Task, "Button", 256, NULL, 6, NULL);

    /* RTOS 스케줄러 시작 */
    vTaskStartScheduler();

    for(;;);
    return 0;
}

/* END main */
/*!
** @}
*/
