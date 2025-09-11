#ifndef SHH_SOUND_H
#define SHH_SOUND_H

/* 보안 경고음(Buzzer) 제어 */
void SHH_Buzzer_StartAlarm(void);
void SHH_Buzzer_StopAlarm(void);

/* 버튼 클릭 피드백(Piezo) 제어 */
void SHH_Piezo_Beep(void);

#endif /* SHH_SOUND_H */
