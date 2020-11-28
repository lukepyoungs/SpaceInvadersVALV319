// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EdX Lab 15

// Last Modified: 8/11/2020 
// for images and sounds, see http://users.ece.utexas.edu/~valvano/Volume1/Lab10Files
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2020

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2020

 Copyright 2020 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "Sound.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "DAC.h"
#include "Images.h"
#include "Timer0.h"
#include "Timer1.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void DAC_Init(void);
void HardInit(void);
void Timer2_Init(void(*task)(void), unsigned long period);
unsigned long ADCMail;
unsigned long ADCStatus;
signed long ADCData, Pos, Buf;

extern unsigned char String[12];
void ConvertUDec(unsigned long n);
void ConvertDistance(unsigned long n);
void LCD_OutUDec(unsigned long n){
  ConvertUDec(n);     // convert using your function
  ST7735_OutString((char *)String);  // output using your function
}
void LCD_OutDistance(unsigned long n){
  ConvertDistance(n);      // convert using your function
  ST7735_OutString((char *)String);  // output using your function
}
void PortF_Init(void){
	unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x20;
	delay=0;
	SYSCTL_RCGCGPIO_R |= 0x20;
	while((SYSCTL_PRGPIO_R&0x20) == 0){};
	GPIO_PORTE_DIR_R &= ~0x07;
	GPIO_PORTE_AFSEL_R &= ~0x07;
	GPIO_PORTE_AMSEL_R &= ~0x07;
	GPIO_PORTE_DEN_R |= 0x07;
}
void PortE_Init(void){
	unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x10;
	delay=0;
	SYSCTL_RCGCGPIO_R |= 0x10;
	while((SYSCTL_PRGPIO_R&0x10) == 0){};
	GPIO_PORTE_DIR_R &= ~0x03; // PE0 PE1 input
	GPIO_PORTE_AFSEL_R &= ~0x03;
	GPIO_PORTE_AMSEL_R &= ~0x03;
	GPIO_PORTE_DEN_R |= 0x03;
}
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;    					// disable SysTick during setup
  NVIC_ST_CTRL_R = 0x07;
  NVIC_ST_RELOAD_R = 2000000;			// reload value --> 40 Hz
  NVIC_ST_CURRENT_R = 0;      		// any write to current clears it
}
int IntCounter;

void SysTick_Handler(void){
//Toggle Heartbeat Twice
	PF2 ^= 0xFF;          
  PF2 ^= 0xFF;
//Sample ADC
	for(int i = 0; i < 20; i++){
		ADCMail += ADC_In();
	}
	ADCMail = (ADCMail/20);
//Set Flag
	ADCStatus = 1;
	PF2 ^= 0xFF;
	IntCounter = IntCounter + 1;
}

int main(void){
  DAC_Init();
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);
	PortF_Init();
	PortE_Init();
  Output_Init();
	Timer0_Init(*Sound_Play, 7256);
	
	typedef enum {English, Spanish} Language_t;
	Language_t myLanguage=English;
	typedef enum {HELLO,SELECT,LANGUAGE,GAME,OVER} phrase_t;
	const char Hello_English[]="HELLO";
	const char Hello_Spanish[]="\xADHola!";
	const char Select_English[]="SELECT";
	const char Select_Spanish[]="ESCOGER";
	const char Game_English[]="GAME";
	const char Game_Spanish[]="JUEGO";
	const char Over_Spanish[]="TERMINADO";
	const char Over_English[]="OVER";
	const char Language_Spanish[]="Espa\xA4ol";
	const char Language_English[]="English";
	const char *Phrases[5][2]={
		{Hello_English,Hello_Spanish},
		{Select_English,Select_Spanish},
		{Language_English, Language_Spanish},
		{Game_English, Game_Spanish},
		{Over_English, Over_Spanish}
	};






	
	
	
  ST7735_FillScreen(0x0000);            // set screen to black
  
  ST7735_DrawBitmap(52, 159, PlayerShip0, 18,8); // player ship middle bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);

  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);

  Delay100ms(50);              // delay 5 sec at 80 MHz

  ST7735_FillScreen(0x0000);            // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  LCD_OutUDec(1234);
  while(1){
  }

}


// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
