// DAC.c
// This software configures DAC output
// Labs 6 and 10 requires 6 bits for the DAC
// Runs on LM4F120 or TM4C123
// Program written by: Devin Chaky
// Date Created: 3/6/17 
// Last Modified: 5/1/23 
// Lab number: 6
// Hardware connections
// 6-Bit DAC output ports PB5-0

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 6-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
  // used in Lab 6 and lab 10
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x02;	// enable clock for port b
	delay = SYSCTL_RCGCGPIO_R;	// wait for clock
	GPIO_PORTB_AMSEL_R &= ~0x3F;// disable analog on PB3-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // configure PB3-0 as GPIO
	GPIO_PORTB_DIR_R |= 0x3F;	// set direction register
	GPIO_PORTB_AFSEL_R &= 0x3F; // disable alt funct on PB3-0
	GPIO_PORTB_DEN_R |= 0x3F;	// digital enable output pins
	GPIO_PORTB_DR8R_R |= 0x3F;	// enable driver for 8mA
}

// **************DAC_Out*********************
// output to DAC
// Input: 6-bit data, 0 to 63 
// Input=n is converted to n*3.3V/63
// Output: none
void DAC_Out(uint32_t data){
  // used in Lab 6 and lab 10
	GPIO_PORTB_DATA_R = data;
}
