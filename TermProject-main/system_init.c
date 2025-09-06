#include "system_init.h"
#include "clocks_and_modes.h"

void system_init(void) {
   WDOG_disable();
   SOSC_init_8MHz();
   SPLL_init_160MHz();
   NormalRUNmode_80MHz();
   PORT_init();  
   LPIT0_init();
   NVIC_init_IRQs();
   LPUART1_init(); 
   FTM_init();
   ADC_init();
}

void PORT_init(void) {
   /*=========================================== PORT_CLK_ENABLE ========================================*/
   PCC->PCCn[PCC_PORTB_INDEX] |= PCC_PCCn_CGC_MASK;
   PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;
   PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
   PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK;


   /*=============================================== PORTB ===============================================*/
   // DC MOTOR
   PORTB->PCR[8] |= PORT_PCR_MUX(2);
   PORTB->PCR[9] |= PORT_PCR_MUX(2);

   /* PORT B설정
   PTB0->EN
   PTB1->RW
   PTB2->RS

   PTB10->D4
   PTB11->D5
   PTB12->D6
   PTB13->D7
   */

   PORTB->PCR[0]= PORT_PCR_MUX(1); //EN
   PORTB->PCR[1]= PORT_PCR_MUX(1); //RW
   PORTB->PCR[2]= PORT_PCR_MUX(1); //RS
   PORTB->PCR[10]= PORT_PCR_MUX(1); //D4
   PORTB->PCR[11]= PORT_PCR_MUX(1); //D5
   PORTB->PCR[12]= PORT_PCR_MUX(1); //D6
   PORTB->PCR[13]= PORT_PCR_MUX(1); //D7

   PTB->PDDR |= 1<<0 | 1<<1 | 1<<2 | 1<<10 | 1<<11 | 1<<12 | 1<<13;

   /*=============================================== PORTC ===============================================*/
   // SW1 for INDOOR LED
   PTC->PDDR &= ~(1<<1); /* Port C1:  Data Direction = input */
   PORTC->PCR[1]  = PORT_PCR_MUX(1);	/* Port D10: MUX = GPIO  */

   // SW2 (PTC2) GPIO and Falling-edge interrupt setup
   PTC->PDDR &= ~(1 << 2); /* Set PTC2 as input */
   PORTC->PCR[2] |= PORT_PCR_MUX(1);  /* Port C2: GPIO */
   PORTC->PCR[2] |= (10 << 16); /* Falling-edge interrupt */

   // SW3 for blind
   PTC->PDDR &= ~(1<<9); /* Port C3:  Data Direction = input */
   PORTC->PCR[9] = PORT_PCR_MUX(1);	/* Port D10: MUX = GPIO  */

   // SW for WS->SS
   PTC->PDDR &= ~(1 << 5);
   PORTC->PCR[5] |= PORT_PCR_MUX(1);

   // UART Pin Configuration
   PORTC->PCR[6]|=PORT_PCR_MUX(2);           /* Port C6: MUX = ALT2,UART1 TX */
   PORTC->PCR[7]|=PORT_PCR_MUX(2);           /* Port C7: MUX = ALT2,UART1 RX */

   // SW5 (PTC17) GPIO and Falling-edge interrupt setup
   PTC->PDDR &= ~(1 << 17); /* Set PTC5 as input */
   PORTC->PCR[17] |= PORT_PCR_MUX(1);  /* Port C5: GPIO */
   PORTC->PCR[17] |= (10 << 16); /* Falling-edge interrupt */

   /*=============================================== PORTD ===============================================*/

   /* s32k144 RGB for Air Conditioner */
   PTD->PDDR |= (1<<0) | (1<<15) | (1<<16);   /* Port D0, 15, 16 : Data Direction= input (default) */
   PORTD->PCR[0] = PORT_PCR_MUX(1); /* Port D0 : MUX = GPIO */
   PORTD->PCR[15] = PORT_PCR_MUX(1); /* Port D15 : MUX = GPIO */
   PORTD->PCR[16] = PORT_PCR_MUX(1); /* Port D16 : MUX = GPIO */

   // PIEZO Pin Configuration
   PORTD->PCR[14] |= PORT_PCR_MUX(2);

   PTD->PDDR |= 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7; //FND_ALPHA
   PTD->PDDR |= 1<<8|1<<9|1<<10|1<<11|1<<12; //FND_COM

   // 7-Segment setup (PortD: Data and Select pins)
   for (int i = 1; i <= 12; i++) {
      PORTD->PCR[i] = PORT_PCR_MUX(1); /* Set Port D1-D12 as GPIO */
   }


   /*=============================================== PORTE ===============================================*/
   // TRIG_PIN Configuration
   PTE->PDDR |= (1 << 0);
   PORTE->PCR[0] |= PORT_PCR_MUX(1);
   // ECHO_PIN Configuration
   PTE->PDDR &= ~(1 << 1);
   PORTE->PCR[1] |= PORT_PCR_MUX(1);

   // LED1 ,2 PIN Configuration, INNER/OUTTER
   PTE->PDDR |= 1<<2|1<<3;		/* Port E2~3:  Data Direction = output */
   PORTE->PCR[2]  = PORT_PCR_MUX(1);	/* Port E2: MUX = GPIO  */ //INNER
   PORTE->PCR[3]  = PORT_PCR_MUX(1);	/* Port E3: MUX = GPIO  */ //OUTTER

   // LED5 Pin Configuration wildlife LED
   PTE->PDDR |= (1 << 6);
   PORTE->PCR[6] |= PORT_PCR_MUX(1);

   // LED6 (PTE7) Configuration SOS LED
   PTE->PDDR |= (1 << 7); /* Set PTE7 as output */
   PORTE->PCR[7] |= PORT_PCR_MUX(1);  /* Port E7: GPIO */
}


void WDOG_disable (void)
{
   WDOG->CNT=0xD928C520;     /* Unlock watchdog       */
   WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value    */
   WDOG->CS = 0x00002100;    /* Disable watchdog       */
}

void LPIT0_init(void)
{
   /*!
   * LPIT Clocking:
   * ==============================
   */
   PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
   PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs 		*/
   /*!
   * LPIT Initialization:
   */
   LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
                                       /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                                       /* SW_RST=0: SW reset does not reset timer chans, regs */
                                       /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
   LPIT0->MIER = 0x03;  /* TIE0=1: Timer Interrupt Enabled fot Chan 0,1 */

   LPIT0->TMR[0].TVAL = 40000;      /* Chan 0 Timeout period: 1ms clocks */
   LPIT0->TMR[0].TCTRL = 0x00000001;
                        /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/

   LPIT0->TMR[1].TVAL = 40;      /* Chan 1 Timeout period: 1us clocks */
   LPIT0->TMR[1].TCTRL = 0x00000001;
                        /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

void LPIT0_Ch2_init (uint32_t delay)
{
   uint32_t timeout;
   /*!
   * LPIT Clocking:
   * ==============================
   */
   // PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
   // PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs       */
   /*!
   * LPIT Initialization:
   */
   // LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
                                       /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                                       /* SW_RST=0: SW reset does not reset timer chans, regs */
                                       /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */


   timeout=delay* 40000;
   LPIT0->TMR[2].TVAL = timeout;      /* Chan 0 Timeout period: 40M clocks */
   LPIT0->TMR[2].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
                           /* T_EN=1: Timer channel is enabled */
                           /* CHAIN=0: channel chaining is disabled */
                           /* MODE=0: 32 periodic counter mode */
                           /* TSOT=0: Timer decrements immediately based on restart */
                           /* TSOI=0: Timer does not stop after timeout */
                           /* TROT=0 Timer will not reload on trigger */
                           /* TRG_SRC=0: External trigger soruce */
                           /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

void delay_ms (volatile int ms) {
   LPIT0_Ch2_init(ms);           /* Initialize PIT0 for 1 second timeout  */
   while (0 == (LPIT0->MSR & LPIT_MSR_TIF2_MASK)) {} /* Wait for LPIT0 CH0 Flag */     
               LPIT0->MSR |= LPIT_MSR_TIF2_MASK; /* Clear LPIT0 timer flag 0 */
}

void LPIT0_Ch3_init (uint32_t delay)
{
   uint32_t timeout;
   /*!
    * LPIT Clocking:
    * ==============================
    */
   // PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
   // PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs       */
     /*!
      * LPIT Initialization:
      */
   // LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
                                        /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
                                        /* SW_RST=0: SW reset does not reset timer chans, regs */
                                        /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */

    timeout=delay* 4;
    LPIT0->TMR[3].TVAL = timeout;      /* Chan 0 Timeout period: 40M clocks */
    LPIT0->TMR[3].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
                              /* T_EN=1: Timer channel is enabled */
                              /* CHAIN=0: channel chaining is disabled */
                              /* MODE=0: 32 periodic counter mode */
                              /* TSOT=0: Timer decrements immediately based on restart */
                              /* TSOI=0: Timer does not stop after timeout */
                              /* TROT=0 Timer will not reload on trigger */
                              /* TRG_SRC=0: External trigger soruce */
                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

void delay_100ns (volatile int hns) {
   LPIT0_Ch3_init(hns);           /* Initialize PIT0 for 1 second timeout  */
   while (0 == (LPIT0->MSR & LPIT_MSR_TIF3_MASK)) {} /* Wait for LPIT0 CH0 Flag */     
               LPIT0->MSR |= LPIT_MSR_TIF3_MASK; /* Clear LPIT0 timer flag 0 */
}

void NVIC_init_IRQs(void)
{
	/*LPIT ch0 overflow set*/
	S32_NVIC->ICPR[1] = 1 << (48 % 32);
	S32_NVIC->ISER[1] = 1 << (48 % 32);
	S32_NVIC->IP[48] = 0x20;

   /*LPIT ch1 overflow set*/
	S32_NVIC->ICPR[1] = 1 << (49 % 32);
	S32_NVIC->ISER[1] = 1 << (49 % 32);
	S32_NVIC->IP[49] = 0x30;

	/*LPIT ch2 overflow set*/
	S32_NVIC->ICPR[1] = 1 << (50 % 32);
	S32_NVIC->ISER[1] = 1 << (50 % 32);
	S32_NVIC->IP[50] = 0x40;

   /*LPIT ch3 overflow set*/
	S32_NVIC->ICPR[1] = 1 << (51 % 32);
	S32_NVIC->ISER[1] = 1 << (51 % 32);
	S32_NVIC->IP[51] = 0x10;

   /*PORTC Interrupt set*/
	S32_NVIC->ICPR[1] = 1 << (61 % 32);
	S32_NVIC->ISER[1] = 1 << (61 % 32);
	S32_NVIC->IP[61] = 0x00;
}

void FTM_init (void){

	//FTM2 clocking
	PCC->PCCn[PCC_FTM2_INDEX] &= ~PCC_PCCn_CGC_MASK;		//Ensure clk diabled for config
	PCC->PCCn[PCC_FTM2_INDEX] |= PCC_PCCn_PCS(0b010)		//Clocksrc=1, 8MHz SIRCDIV1_CLK
								| PCC_PCCn_CGC_MASK;		//Enable clock for FTM regs

   // FTM3 clocking
   PCC->PCCn[PCC_FTM3_INDEX] &= ~PCC_PCCn_CGC_MASK;		//Ensure clk diabled for config
	PCC->PCCn[PCC_FTM3_INDEX] |= PCC_PCCn_PCS(0b010)		//Clocksrc=1, 8MHz SIRCDIV1_CLK
								| PCC_PCCn_CGC_MASK;		//Enable clock for FTM regs


	//FTM2 Initialization
	FTM2->SC = FTM_SC_PWMEN5_MASK							//Enable PWM channel 1output
				|FTM_SC_PS(0);								//TOIE(timer overflow Interrupt Ena) = 0 (deafault)
															//CPWMS(Center aligned PWM Select) =0 (default, up count)
															/* CLKS (Clock source) = 0 (default, no clock; FTM disabled) 	*/
															/* PS (Prescaler factor) = 0. Prescaler = 1 					*/

	FTM2->MOD = 3000-1;									//FTM2 counter final value (used for PWM mode)
															// FTM2 Period = MOD-CNTIN+0x0001~=8000 ctr clks=4ms
															//8Mhz /1 =8MHz
	FTM2->CNTIN = FTM_CNTIN_INIT(0);


	FTM2->CONTROLS[5].CnSC |=FTM_CnSC_MSB_MASK;
	FTM2->CONTROLS[5].CnSC |=FTM_CnSC_ELSA_MASK;			/* FTM2 ch5: edge-aligned PWM, low true pulses 		*/
															/* CHIE (Chan Interrupt Ena) = 0 (default) 			*/
															/* MSB:MSA (chan Mode Select)=0b10, Edge Align PWM		*/
															/* ELSB:ELSA (chan Edge/Level Select)=0b10, low true 	*/

   // FTM3 Initialization
   FTM3->SC = FTM_SC_PWMEN0_MASK	| FTM_SC_PWMEN1_MASK						//Enable PWM channel 1output
				|FTM_SC_PS(0);

   FTM3->MOD = 8000-1;

   FTM3->CNTIN = FTM_CNTIN_INIT(0);

   FTM3->CONTROLS[0].CnSC |=FTM_CnSC_MSB_MASK;
	FTM3->CONTROLS[0].CnSC |=FTM_CnSC_ELSA_MASK;

   FTM3->CONTROLS[1].CnSC |=FTM_CnSC_MSB_MASK;  
	FTM3->CONTROLS[1].CnSC |=FTM_CnSC_ELSA_MASK;

   FTM3->SC |= FTM_SC_CLKS(3);

}

void FTM2_CH5_PWM (int i){//uint32_t i){

	FTM2->CONTROLS[5].CnV = i; //8000~0 duty; ex(7200=> Duty 0.1 / 800=>Duty 0.9)
	//start FTM0 counter with clk source = external clock (SOSCDIV1_CLK)
	FTM2->SC|=FTM_SC_CLKS(3);
}
