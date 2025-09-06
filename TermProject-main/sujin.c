#include "sujin.h"

unsigned int FND_SEL[6]={0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000};
unsigned int SOS_FND_DATA[3] = {0xDA, 0x7E, 0xDA}; // 5, 0, 5

// 0, 1, 2, 3, 4, ... , 8, 9
unsigned int NUM_FND_DATA[10]={0x7E, 0x0C,0xB6,0x9E,0xCC,0xDA,0xFA,0x4E,0xFE,0xCE};

unsigned int FND_LOADING [6] = {0x7A, 0x76, 0x6E, 0x5E, 0x3E, 0x7C};

uint32_t adcResultInMv=0;	/*< ADC0 Result in miliVolts */
int targettemp = 18;
int curtemp = 24;
int prev_state = 0;
int cur_state = 0;

int cnt = 0;

int loading_anim_index = 0;
int cnt_blind = 0;
int D_Blind = 7600;
int blind_state = 0;   // 0: stop, 1: down, 2: up
int switch_released = 1; // Flag to check if the switch is in a released state

void Segment_display (int seg) {
    switch(seg)
    {   case 1:
        for (int j = 0; j < 1000; j++) {
            for (int i=0; i < 3; i++) {
                PTD->PSOR |= FND_SEL[i]; // find the specific pin (location)
                PTD->PCOR |= 0xFE;       // Clear previous data (A~G)
                PTD->PSOR |= SOS_FND_DATA[i]; // Print alphabet
                delay_ms(1);
               PTD->PCOR |= 0x1FFE;                // Clear previous data (FND_selected pins, A~G)
            }
        }
        
        case 2:
        PTD->PCOR |= 0x1FFE; //turn off
    }
}

void SOS_process(void) {
    LED6_ON;
    Segment_display(1);
    LPUART1_transmit_string("SOS report has been received.\n\r");
    // SOS(); /*Turn on LCE*/
    LED6_OFF;
    Segment_display(2);
}

void ac_temp_seg(int cur_temp, int target_temp)
{
    unsigned int j=0;
    unsigned int num0,num1,num2,num3 =0;

	int Dtime = 20;
	num3 = (cur_temp / 10) % 10;
	num2 = cur_temp % 10;
    

	num1 = (target_temp / 10) % 10;
	num0 = target_temp % 10;
	for(int i = 0; i<125; i++)
	{
		PTD->PSOR = FND_SEL[j];
		PTD->PCOR =0xfe;
		PTD->PSOR = NUM_FND_DATA[num3];
		delay_100ns(Dtime);
		PTD->PCOR = 0x1ffe;
		j++;

		PTD->PSOR = FND_SEL[j];
		PTD->PCOR =0xfe;
		PTD->PSOR = NUM_FND_DATA[num2];
		delay_100ns(Dtime);
		PTD->PCOR = 0x1ffe;
		j++;

		PTD->PSOR = FND_SEL[j];
		PTD->PCOR =0xfe;
		PTD->PSOR = NUM_FND_DATA[num1];
		delay_100ns(Dtime);
		PTD->PCOR = 0x1ffe;
		j++;

		PTD->PSOR = FND_SEL[j];
		PTD->PCOR =0xfe;
		PTD->PSOR = NUM_FND_DATA[num0];
		delay_100ns(Dtime);
		PTD->PCOR = 0x1ffe;
		j=0;

	}

}

int ac_cur_target(int curtemp,int target_temp)
{

	if(target_temp > curtemp)
	{
        cnt++;
        if (cnt == 15) {
            curtemp++;
            cnt = 0;
        }
	}
	else if (target_temp < curtemp)
	{
        cnt++;
        if (cnt == 15) {
            curtemp--;
            cnt = 0;
        }
	}
	return curtemp;
}

void ac_oper(int curtemp, int target_temp)
{
	if(target_temp > curtemp)
	{
        cur_state = ~prev_state;
		BLUE_OFF; GREEN_OFF;
        RED_ON;
        ac_temp_seg(curtemp, targettemp);
		//lcd : HEATER ON
	}
	else if(target_temp < curtemp)
	{
        cur_state = ~prev_state;
		RED_OFF; GREEN_OFF;
		BLUE_ON;
        ac_temp_seg(curtemp, targettemp);
	}
	else
	{
		BLUE_OFF; RED_OFF;
		GREEN_ON;
        if (prev_state != cur_state) {
            for (int i = 0; i < 10; i++) 
                ac_temp_seg(curtemp, targettemp);
            cur_state = prev_state;
        }
		//lcd : TARGET == TEMP
	}

}

void ac_control(void) {
    static int ach_cnt = 0;

    convertAdcChan(13);                   /* Convert Channel AD13 to pot on EVB 	*/
    while(adc_complete()==0){}            /* Wait for conversion complete flag 	*/
    adcResultInMv = read_adc_chx();       /* Get channel's conversion results in mv */

    if (adcResultInMv > 4682)
    {
        targettemp = 30;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 4370)
    {
        targettemp = 29;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 4058)
    {
        targettemp = 28;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 3746)
    {
        targettemp = 27;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 3434)
    {
        targettemp = 26;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 3122)
    {
        targettemp = 25;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 2810)
    {
        targettemp = 24;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 2498)
    {
        targettemp = 23;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 2186)
    {
        targettemp = 22;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 1874)
    {
        targettemp = 21;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 1562)
    {
        targettemp = 20;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv >1250)
    {
        targettemp = 19;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else if (adcResultInMv > 938) // turn on an air conditioner
    {
        if (!ach_cnt) {
            AC_Heater_On();
            ach_cnt = 1;
        }
        targettemp = 18;
        curtemp = ac_cur_target(curtemp, targettemp);
        ac_oper(curtemp,targettemp);
    }
    else
    {
        if (ach_cnt) {
            AC_Heater_OFF();
            ach_cnt = 0;
        }
        RED_OFF; BLUE_OFF; GREEN_OFF;
    }
}

void Loading_seg(void)
{
	int Dtime = 20;
	int index = (loading_anim_index / 350);

    for (int i = 0; i < 5; i++) {
        PTD->PSOR = FND_SEL[4];
        PTD->PCOR = 0xffe;
        PTD->PSOR = FND_LOADING[5 - index];
        delay_100ns(Dtime);
        PTD->PCOR = 0x1ffe;
    }

	loading_anim_index++;
	if ( (loading_anim_index / 350) > 5)
		loading_anim_index = 0;

}

void Inver_Loading_seg(void)
{
	int Dtime = 20;
	int index = (loading_anim_index / 350);

    for (int i = 0; i < 5; i++) {
        PTD->PSOR = FND_SEL[4];
        PTD->PCOR = 0xffe;
        PTD->PSOR = FND_LOADING[index];
        delay_100ns(Dtime);
        PTD->PCOR = 0x1ffe;
    }


	loading_anim_index++;
	if ( (loading_anim_index / 350) > 5)
		loading_anim_index = 0;

}

void blind_control(void) {
    if (SW3_ON) { // When SW3 is pressed
        if (switch_released) { // Operates only if the switch was released and pressed again
            if ((cnt_blind == 0) || (cnt_blind == 3500))
                Blind_Operating();
            switch (blind_state) {
                case 0: // Starting from the stopped state
                    if (cnt_blind == 0) {
                        blind_state = 1; // Change to lowering state
                    } else if (cnt_blind == 3500) {
                        blind_state = 2; // Change to raising state
                    }
                    break;

                case 1: // Lowering
                    break;

                case 2: // Raising
                    break;
            }
            switch_released = 0; // Record that the switch is pressed
        }

        // Blind operation control
        if (blind_state == 1) { // Lowering
            Loading_seg();
            FTM3->CONTROLS[1].CnV = FTM_CnV_VAL(0);
            FTM3->CONTROLS[0].CnV = FTM_CnV_VAL(D_Blind);

            cnt_blind++;
            if (cnt_blind == 3500) {
                blind_state = 0; // Change to stopped state
            }
        } else if (blind_state == 2) { // Raising
            Inver_Loading_seg();
            FTM3->CONTROLS[0].CnV = FTM_CnV_VAL(0);
            FTM3->CONTROLS[1].CnV = FTM_CnV_VAL(D_Blind);

            cnt_blind++;
            if (cnt_blind == 7000) {
                blind_state = 0; // Change to stopped state
                cnt_blind = 0;
            }
        } else { // Stopped state
            FTM3->CONTROLS[0].CnV = FTM_CnV_VAL(0);
            FTM3->CONTROLS[1].CnV = FTM_CnV_VAL(0);
            loading_anim_index = 0;
        }
    } else { // When the switch is released, stop the blind (allows user to control how far the blind lowers)
        switch_released = 1; // Record that the switch is released
        FTM3->CONTROLS[0].CnV = FTM_CnV_VAL(0);
        FTM3->CONTROLS[1].CnV = FTM_CnV_VAL(0);
        loading_anim_index = 0;
    }
}
