#include "yongjin.h"

int wildlife_distance = 100000;
volatile uint32_t time_us = 0;
volatile uint32_t echo_time_us = 0;
volatile uint32_t sw5_count = 0;
volatile uint32_t led5_count = 0;

int previous_state1 = 0;
int previous_state2 = 0;
int LED_state1 = 0;
int LED_state2 = 0;

unsigned int i = 0;

WILDLIFE_TYPE current_state = STEADY_STATE;

void uwave_sensing(void) {

    if (time_us == 0)
        UW_TRIG_HIGH;
    else if (time_us >= 10) {
        UW_TRIG_LOW;
    } 

    time_us++;

    if (UW_ECHO_READ) {
        echo_time_us++;
    }
    if (time_us >= 60000) {
        wildlife_distance = (echo_time_us * 0.034);   // set wildlife distance (cm)
        echo_time_us = 0;
        time_us = 0;
    }

}

void wildlife_detect(void) {
    if (wildlife_distance <= WARNING_THRESHOLD) {
        current_state = WARNING_STATE;
    }

    if (current_state == WARNING_STATE) {
        led5_count++;
        if (led5_count >= 100) {               // Turn on LED5; T=100ms
            LED5_TOG;
            led5_count = 0;
        }

    }
    else {
        current_state = STEADY_STATE;
    }

}

void buzzer_control(int dist) {   
    if (dist <= WARNING_THRESHOLD)
        FTM2_CH5_PWM((dist) * 150);          // CNV modulation
    else if (dist > WARNING_THRESHOLD) 
        FTM2_CH5_PWM(3000);                          // out of WARNING STATE ; turn off
    else if (dist == 0)
        FTM2_CH5_PWM(0);  // duty = 0.9
}

void clear_warning(void) {
    if (SW5_ON && (current_state == WARNING_STATE)) {
        sw5_count++;
        if (sw5_count >= 3000) {     // 1ms * 3000 = 3s
            LED5_OFF;
            buzzer_control(3000);         // buzzer off
            current_state = STEADY_STATE;
            led5_count = 0;
            sw5_count = 0;
        }
    } else {
        sw5_count = 0;
    }
}

void print_cur_state(void) {
    if (current_state == STEADY_STATE) {
        LPUART1_transmit_string("DETECTED DISTANCE : ");
        send_number_as_string(wildlife_distance); 
        LPUART1_transmit_string("cm   >> STEADY-STATE\n\r");
    } else {
        LPUART1_transmit_string("DETECTED DISTANCE : ");
        send_number_as_string(wildlife_distance);
        LPUART1_transmit_string("cm   >> WARNING-STATE\n\r");
    }
}

void send_number_as_string(int num) {
    char buffer[5]; 
    sprintf(buffer, "%d", num);
    LPUART1_transmit_string(buffer);
}

void indoor_control(void)
{
	int current_state1 = !(PTC->PDIR & (1 << 1));

	if (current_state1 && !previous_state1 ) {
		LED_state1 = !LED_state1;
		if(LED_state1){
		    LED1_ON; /*turn ON*/
            Indoor_Light_on();
	    } else {
		    LED1_OFF; /*turn OFF*/
            Indoor_Light_off();
	    }

	}

	previous_state1 = current_state1;
}


/***************************************************lcd***********************************************************/
void AC_Heater_On(void) {

    char msg_array1[16]={0x20, 0x20, 0x20, 0x41, 0x43, 0x20, 0x48, 0x65, 0x61, 0x74, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20};  // 1-row text-char
    char msg_array2[16]={0x20,0x20,0x20,0x20,0x54, 0x75, 0x72, 0x6E, 0x20, 0x4F, 0x6E}; // 2-row text-char
    lcdinit();        /* Initialize LCD1602A module*/

    delay_ms(20);

    //text-char output
    while(msg_array1[i] != '\0'){
        lcdcharinput(msg_array1[i]); // 1(first) row text-char send to LCD module
        delay_ms(2);
        i++;

    }

    lcdinput(0x80+0x40);// second row
    delay_100ns(5);
    i=0;
    while(msg_array2[i] != '\0') {
        lcdcharinput(msg_array2[i]);// 2(second) row text-char send to LCD module
        delay_ms(2);
        i++;
    }
    i=0; 
    //Lcd off, LCD display clear
    delay_ms(200);
    lcdinput(0x08);   //lcd display off
    lcdinput(0x01);   //Clear display

}

void AC_Heater_OFF(void) {
    char msg_array1[16]={0x20, 0x20, 0x20, 0x41, 0x43, 0x20, 0x48, 0x65, 0x61, 0x74, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20};  // 1-row text-char
    char msg_array2[16]={0x20, 0x20, 0x20, 0x20,0x54, 0x75, 0x72, 0x6E, 0x20, 0x4F, 0x46, 0x46, 0x20, 0x20, 0x20, 0x20}; // 2-row text-char

    lcdinit();        /* Initialize LCD1602A module*/

    delay_ms(20);

    //text-char output
    while(msg_array1[i] != '\0'){
        lcdcharinput(msg_array1[i]); // 1(first) row text-char send to LCD module
        delay_ms(2);
        i++;

    }

    lcdinput(0x80+0x40);// second row
    delay_100ns(5);
    i=0;
    while(msg_array2[i] != '\0'){
        lcdcharinput(msg_array2[i]);// 2(second) row text-char send to LCD module
        delay_ms(2);
        i++;
    }
	   i=0; //�������� �����ų���� 0���� �ʱ�ȭ ������Ѵ�.
    //Lcd off, LCD display clear
    delay_ms(200);
    lcdinput(0x08);   //lcd display off
    lcdinput(0x01);   //Clear display

}

void Indoor_Light_on(void) {

    char msg_array1[16]={0x20,0x20, 0x49, 0x6E, 0x64, 0x6F, 0x6F, 0x72, 0x20, 0x4C, 0x69, 0x67, 0x68, 0x74};  // 1-row text-char
    char msg_array2[16]={0x20,0x20,0x54, 0x75, 0x72, 0x6E, 0x20, 0x6F, 0x6E}; // 2-row text-char

    lcdinit();        /* Initialize LCD1602A module*/

    delay_ms(20);

    //text-char output
    while(msg_array1[i] != '\0'){
        lcdcharinput(msg_array1[i]); // 1(first) row text-char send to LCD module
        delay_ms(2);
        i++;
    }

    lcdinput(0x80+0x40);// second row
    delay_ms(2);
    i=0;
    while(msg_array2[i] != '\0') {
        lcdcharinput(msg_array2[i]);// 2(second) row text-char send to LCD module
        delay_ms(2);
        i++;
    }
    i=0; 
    //Lcd off, LCD display clear
    delay_ms(200);
    lcdinput(0x08);   //lcd display off
    lcdinput(0x01);   //Clear display

}
void Indoor_Light_off(void) {

    char msg_array1[16]={0x20,0x20, 0x49, 0x6E, 0x64, 0x6F, 0x6F, 0x72, 0x20, 0x4C, 0x69, 0x67, 0x68, 0x74};  // 1-row text-char
    char msg_array2[16]={0x20,0x20,0x54, 0x75, 0x72, 0x6E, 0x20, 0x4F, 0x46, 0x46}; // 2-row text-char
    lcdinit();        /* Initialize LCD1602A module*/

    delay_ms(20);

    //text-char output
    while(msg_array1[i] != '\0'){
        lcdcharinput(msg_array1[i]); // 1(first) row text-char send to LCD module
        delay_ms(2);
        i++;

    }

    lcdinput(0x80+0x40);// second row
    delay_ms(2);
    i=0;
    while(msg_array2[i] != '\0'){
    lcdcharinput(msg_array2[i]);// 2(second) row text-char send to LCD module
    delay_ms(2);
    i++;
    }
    i=0; 
    //Lcd off, LCD display clear
    delay_ms(200);
    lcdinput(0x08);   //lcd display off
    lcdinput(0x01);   //Clear display

}

void Blind_Operating(void) {


	    char msg_array1[16]={0x20,0x20,0x20,0x20,0x42, 0x7C, 0x69, 0x6E, 0x64};  // 1-row text-char
	    char msg_array2[16]={0x20,0x20,0x20,0x4F, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6E, 0x67, 0x2E, 0x2E, 0x2E}; // 2-row text-char

	   lcdinit();        /* Initialize LCD1602A module*/

	   delay_ms(20);

	      //text-char output
	      while(msg_array1[i] != '\0'){
	         lcdcharinput(msg_array1[i]); // 1(first) row text-char send to LCD module
	         delay_ms(2);
	         i++;

	      }

	      lcdinput(0x80+0x40);// second row
	         delay_ms(2);
	         i=0;
	         while(msg_array2[i] != '\0'){
	            lcdcharinput(msg_array2[i]);// 2(second) row text-char send to LCD module
	            delay_ms(2);
	            i++;
	         }
	   i=0; 
    //Lcd off, LCD display clear
    delay_ms(200);
    lcdinput(0x08);   //lcd display off
    lcdinput(0x01);   //Clear display

}

void Wildlife_Detected_Turn_ON(void) {
    char msg_array1[16]={0x20, 0x20, 0x20, 0x20, 0x57, 0x69, 0x6C, 0x64, 0x7C, 0x69, 0x66, 0x65, 0x20, 0x20, 0x20, 0x20,};  // 1-row text-char
    char msg_array2[16]={0x20,0x20,0x20,0x20,0x54, 0x75, 0x72, 0x6E, 0x20, 0x4F, 0x6E}; 
    lcdinit();        /* Initialize LCD1602A module*/

    delay_ms(20);

    //text-char output
    while(msg_array1[i] != '\0'){
        lcdcharinput(msg_array1[i]); // 1(first) row text-char send to LCD module
        delay_ms(2);
        i++;

    }

    lcdinput(0x80+0x40);// second row
    delay_ms(2);
    i=0;
    while(msg_array2[i] != '\0'){
        lcdcharinput(msg_array2[i]);// 2(second) row text-char send to LCD module
        delay_ms(2);
        i++;
    }
    i=0; 
    
    //Lcd off, LCD display clear
    delay_ms(200);
    lcdinput(0x08);   //lcd display off
    lcdinput(0x01);   //Clear display

}

void Wildlife_Detected_Turn_OFF(void) {
    char msg_array1[16]={0x20, 0x20, 0x20, 0x20, 0x57, 0x69, 0x6c, 0x64, 0x7C, 0x69, 0x66, 0x65, 0x20, 0x20, 0x20, 0x20,};  // 1-row text-char
    char msg_array2[16]={0x20, 0x20, 0x20, 0x20,0x54, 0x75, 0x72, 0x6E, 0x20, 0x4F, 0x46, 0x46, 0x20, 0x20, 0x20, 0x20}; 
    lcdinit();        /* Initialize LCD1602A module*/

    delay_ms(20);

    //text-char output
    while(msg_array1[i] != '\0'){
        lcdcharinput(msg_array1[i]); // 1(first) row text-char send to LCD module
        delay_ms(2);
        i++;

    }

    lcdinput(0x80+0x40);// second row
    delay_ms(2);
    i=0;
    while(msg_array2[i] != '\0'){
        lcdcharinput(msg_array2[i]);// 2(second) row text-char send to LCD module
        delay_ms(2);
        i++;
    }
    i=0; 
    
    //Lcd off, LCD display clear
    delay_ms(200);
    lcdinput(0x08);   //lcd display off
    lcdinput(0x01);   //Clear display

}