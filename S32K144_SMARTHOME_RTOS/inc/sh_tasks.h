#ifndef SH_TASKS_H
#define SH_TASKS_H

void SH_MainControl_Task(void *pvParameters);
void SH_Sensor_Task(void *pvParameters);
void SH_ButtonInput_Task(void *pvParameters);
void SH_CanComm_Task(void *pvParameters);
void SH_Display_Task(void *pvParameters);
void SH_SecurityEvent_Task(void *pvParameters);

#endif /* SH_TASKS_H */