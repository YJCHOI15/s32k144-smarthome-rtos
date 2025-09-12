#ifndef SH_TASKS_H
#define SH_TASKS_H

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>

extern QueueHandle_t g_command_queue;
extern QueueHandle_t g_sensor_data_queue;
extern QueueHandle_t g_display_data_queue;
extern SemaphoreHandle_t g_system_status_mutex;
extern EventGroupHandle_t g_security_event_group;

#endif /* SH_TASKS_H */