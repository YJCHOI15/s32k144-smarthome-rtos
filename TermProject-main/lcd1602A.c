/*
 * lcd1602A.c
 *
 *  Created on: 2022. 9. 19.
 *      Author: PECSCHO
 */
#include "device_registers.h"
#include "clocks_and_modes.h"
#include "lcd1602A.h"
#include "system_init.h"

int flag_counter=0;

#define BASE 	 10

void lcdinit(void){

	lcdinput(0x28);	//lcd function set #1
	delay_100ns(70000);

	lcdinput(0x28);	//lcd function set #2
	delay_100ns(70000);

	lcdinput(0x28);	//lcd function set #3
	delay_100ns(70000);


	lcdinput(0x08);	//lcd off
	delay_100ns(700);

	lcdinput(0x01);	//lcd clear
	delay_100ns(20000);

	lcdinput(0x06);	//lcd modeset
	delay_100ns(700);


	lcdinput(0x0F);	//lcd display,cursor
	delay_100ns(20000);

	lcdinput(0x80);	//dd ram address
	delay_100ns(700);


	lcdinput(0x02);	//return home
	delay_100ns(20000);

}

void lcdinput(uint16_t data){
	uint16_t data1 = ((uint16_t)data&0xF0)<<(BASE-4);	//D7 ~ 4
	uint16_t data2 = (((uint16_t)data&0x0F)<<BASE);		//D3 ~ 0
	LCD_RW;

	//data1
	LCD_EN;
	PTB->PCOR |= 0xF<<BASE;
	PTB->PSOR |= data1;
	delay_100ns(5);

	LCD_NEN;

	//clear before 2nd transmission
	delay_100ns(2);
	PTB->PCOR |= 0xF<<BASE;
	delay_100ns(30);

	//data2
	LCD_EN;
	PTB->PCOR |= 0xF<<BASE;
	PTB->PSOR |= data2;
	delay_100ns(5);

	LCD_NEN;

	//clear before next order
	delay_100ns(2);
	PTB->PCOR |= 0xF<<BASE;
	PTB->PCOR |= ((1<<(2))+(0xF<<BASE));
	delay_100ns(30);

}

void lcdcharinput(char data){
	uint16_t data1 = ((uint16_t)data&0xF0)<<(BASE-4); //D7 ~ 4
	uint16_t data2 = (((uint16_t)data&0x0F)<<BASE); //D3 ~ 0

	LCD_RS;
	//data1
	LCD_EN;
	PTB->PCOR |= (0xF<<BASE);
	PTB->PSOR |= data1;
	delay_100ns(3);

	LCD_NEN;

	//clear before 2nd transmission
	delay_100ns(2);
	PTB->PCOR |= 0xF<<BASE;
	delay_100ns(30);


	//data2
	LCD_EN;
	PTB->PCOR |= 0xF<<BASE;
	PTB->PSOR |= data2;
	delay_100ns(30);

	LCD_NEN;

	//clear before next order
	delay_100ns(2);
	PTB->PCOR |= 0xF<<BASE;
	PTB->PCOR |= ((1<<(2))+(0xF<<BASE));
	delay_100ns(30);

}
