#include "sh_tasks.h"

QueueHandle_t g_command_queue;
QueueHandle_t g_sensor_data_queue;
QueueHandle_t g_display_data_queue;
SemaphoreHandle_t g_system_status_mutex;
EventGroupHandle_t g_security_event_group;