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
#include "shh_system.h"

volatile int exit_code = 0;
/* User includes */

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void)
{
    // 1. 모든 하드웨어 드라이버를 '사용 가능한 상태'로 만듭니다.
    SHH_Init();

    // // 2. 애플리케이션에서 사용할 RTOS 객체들을 생성합니다.
    // Create_Application_RTOS_Objects(); // 큐, 세마포어 등 생성

    // // 3. 애플리케이션 태스크들을 생성합니다.
    // Create_Application_Tasks(); // SH_Sensor_Task 등 생성

    // // 4. 애플리케이션의 '목적'에 맞게 타이머를 설정합니다.
    // SHD_LPIT_SetPeriodic(0, 1000, 초음파 수신 콜백);
    // SHD_LPIT_SetPeriodic(1, 500, CAN 수신 콜백);
    // SHD_LPIT_SetPeriodic(1, 2000, 보안대기 LED 콜백);

    // // 5. RTOS 스케줄러를 시작하여 모든 태스크를 동작시킵니다.
    // vTaskStartScheduler();

    for(;;);
    return 0;
}

/* END main */
/*!
** @}
*/
