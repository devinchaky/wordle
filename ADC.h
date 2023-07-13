// ADC.h
// Runs on TM4C123
// Provide functions that initialize ADC0

// Student names: Devin Chaky
// Last modification date: 5/1/23
#ifndef ADC_H
#define ADC_H
#include <stdint.h>

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void);

void ADC_Init89(void);

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void);

void ADC_In89(uint32_t data[2]);

#endif
