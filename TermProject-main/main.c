#include "system_init.h"
#include "yongjin.h"
#include "sujin.h"

volatile int uart_print_counter = 0; 
volatile int sw4_counter = 0;
volatile int lpit0_ch1_flag_counter = 0; 
volatile bool sw4_flag = false;
volatile bool uw_on_flag = false;
volatile int lcd_cnt = 0;
volatile int start = 0;   // For NO Wildlife lcd at first

extern WILDLIFE_TYPE current_state;
extern int wildlife_distance;

extern unsigned int FND_SEL[3]; // FND Select pins
extern unsigned int FND_DATA[3];

void PORTC_IRQHandler(void) {
    PORTC->PCR[2] &= ~(0x01000000);
    PORTC->PCR[17] &= ~(0x01000000);

   //PORTC_Interrupt State Flag Register Read
   if((PORTC->ISFR & (1<<17)) != 0){
        sw4_flag = true;
    }

   if((PORTC->ISFR & (1<<2)) != 0){
        start = 1;
        uw_on_flag = !uw_on_flag;
        if (uw_on_flag) {
            LED2_ON; 
        } else {
            LED2_OFF;
            LED5_OFF;
            current_state = STEADY_STATE;
        }
        lcd_cnt = 0;
    }
    
    PORTC->PCR[2] |= 0x01000000;
    PORTC->PCR[17] |= 0x01000000;
}

// every 1ms
void LPIT0_Ch0_IRQHandler(void)
{
    uart_print_counter++;

    if (uw_on_flag) {
        wildlife_detect();
        clear_warning();
    }

    if (sw4_flag && SW4_ON) {
        sw4_counter++;
        if (sw4_counter >= 3000) {
            SOS_process();
            sw4_counter = 0;
        }
    } else {
        sw4_counter = 0;
        sw4_flag = false;
    }

    if ((uart_print_counter >= 1000) && uw_on_flag) {
        print_cur_state();
        uart_print_counter = 0;
    }
    LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag*/
}

// every 1us
void LPIT0_Ch1_IRQHandler(void)
{
    uwave_sensing();
    LPIT0->MSR |= LPIT_MSR_TIF1_MASK; /* Clear LPIT0 timer flag*/
}



int main() {
    system_init();

    LED1_OFF;
    LED2_OFF;
    LED5_OFF;
    LED6_OFF;
    BLUE_OFF;
    GREEN_OFF;
    RED_OFF;
    
    while(1) {
        if (current_state == WARNING_STATE)
            buzzer_control(wildlife_distance);

        indoor_control();
        ac_control();
        blind_control();

        if (!lcd_cnt && start) {
            if (uw_on_flag) {
                Wildlife_Detected_Turn_ON();
            } else {
                Wildlife_Detected_Turn_OFF();
            }
            lcd_cnt = 1;
        }

    }
    return 0;
}
