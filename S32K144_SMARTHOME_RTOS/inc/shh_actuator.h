#ifndef SHH_ACTUATOR_H
#define SHH_ACTUATOR_H

#include <stdint.h>


/* 팬(DC 모터) 제어 */
void SHH_Fan_On(void);
void SHH_Fan_Off(void);

/* 도어락(서보 모터) 제어 */
void SHH_DoorLock_Open(void);
void SHH_DoorLock_Close(void);

/* 블라인드(스텝 모터) 제어 */
void SHH_Blinds_Move(int32_t steps);

/* 외부 전원(릴레이) 제어 */
void SHH_ExternalPower_On(void);
void SHH_ExternalPower_Off(void);

#endif /* SHH_ACTUATOR_H */
