// put implementations for functions, explain how it works
// Luke Y and Cole L November 27 2020
// Port B bits 3-0 have the 4-bit DAC

#include "DAC.h"
#include "..//tm4c123gh6pm.h"

// **************DAC_Init*********************
// Initialize 4-bit DAC 
// Input: none
// Output: none
void DAC_Init(void){
	unsigned long volatile delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; //activate port B
  delay = SYSCTL_RCGC2_R;    
  GPIO_PORTB_AMSEL_R &= ~0x0F;      //no analog 
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; //reg func
  GPIO_PORTB_DIR_R |= 0x0F;      // PB0-PB3 out for DAC
  GPIO_PORTB_AFSEL_R &= ~0x0F;   // disable alt PB3
  GPIO_PORTB_DEN_R |= 0x0F;      
	GPIO_PORTB_DR8R_R |= 0x0F;      
}


// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Output: none
void DAC_Out(unsigned long data){
  GPIO_PORTB_DATA_R = data;
}

