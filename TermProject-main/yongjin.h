#ifndef YONGJIN_H
#define YONGJIN_H

#include <stdio.h>
#include "system_init.h"

#define WARNING_THRESHOLD 20 // cm

void uwave_sensing(void);
void wildlife_detect(void);
void buzzer_control(int dist);
void clear_warning(void);
void print_cur_state(void);
void send_number_as_string(int num);
void indoor_control(void);

void AC_Heater_On(void);
void AC_Heater_OFF(void);
void Indoor_Light_on(void);
void Indoor_Light_off(void);
void Blind_Operating(void);
void Wildlife_Detected_Turn_ON(void);
void Wildlife_Detected_Turn_OFF(void);


typedef enum
{
    STEADY_STATE,
    WARNING_STATE
} WILDLIFE_TYPE;


#endif
