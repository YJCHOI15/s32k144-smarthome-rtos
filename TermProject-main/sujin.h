#ifndef SUJIN_H
#define SUJIN_H

#include "system_init.h"

void Segment_display (int seg);
void SOS_process(void);
void ac_control(void);
void ac_temp_seg(int cur_temp, int target_temp);
int ac_cur_target(int curtemp,int target_temp);
void ac_oper(int curtemp, int target_temp);
void Loading_seg(void);
void Inver_Loading_seg(void);
void blind_control(void);

#endif