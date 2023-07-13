// Initializations.c
// This software holds all the initialization functions
// Runs on LM4F120 or TM4C123
// Program written by: Devin Chaky and Aldo Baez
// Date Created: 4/4/23
// Last Modified: 4/4/23
// Lab number: 10

#ifndef Initializations_H
#define Initializations_H
#include <stdint.h>

// used to initialize main screen
void GameInit(void);

// initializes periodic Systick Interrupt
// used to generate random number
void SysTick_Init(uint32_t period);

// initializes start screen logo
void Logo_Init(void);

// initializes edge triggered interrupts
void EdgeTrigger_Init(void);

// initializes port e buttons
void PortE_Init(void);

//initializes port d buttons
void PortD_Init(void);

// initializes LEDs
void Red(void);
void Yellow(void);
void Green(void);

#endif