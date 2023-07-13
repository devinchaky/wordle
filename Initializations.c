// Initializations.c
// This software holds all the initialization functions
// Runs on LM4F120 or TM4C123
// Program written by: Devin Chaky and Aldo Baez
// Date Created: 4/4/23
// Last Modified: 4/4/23
// Lab number: 10

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Images.h"
#include "../inc/ST7735.h"
#include "Sprites.h"
letterbox_t Letters[6][5];
selector_t Selectors[6];
logo_t LogoLetters[6];
// initializes the letterbox sprites, private helper function
void LB_Init(void) {
	int i, j;	// index variables
	// initialize letterbox sprites
	int32_t xcor = 16, ycor = 30;
	for(i = 0; i < 6; i ++) {
		xcor = 16;
		for(j = 0; j < 5; j++) {
			Letters[i][j].alphaind = 0;
			Letters[i][j].letter = alphabet[0];
			Letters[i][j].x = xcor;
			Letters[i][j].y = ycor;
			Letters[i][j].index = i;
			Letters[i][j].row = j;
			Letters[i][j].color = ST7735_WHITE;
			Letters[i][j].width = 15;
			Letters[i][j].height = 17;
			Letters[i][j].needDraw = 0;
			xcor += 20;
		}
		ycor += 22;
	}
	// draw boxes to screen
	for(i = 0; i < 6; i ++) {
		for(j = 0; j < 5; j++) {
			ST7735_FillRect(Letters[i][j].x, Letters[i][j].y, Letters[i][j].width, Letters[i][j].height, Letters[i][j].color);
		}
	}
}

// initialize selectors, helper function
void Selectors_Init(void) {
	int i;	// index variables
	int32_t xcor = 16, ycor = 47;
	for(i=0; i < 6; i++){
		Selectors[i].row = i;
		Selectors[i].x = 15;
		Selectors[i].y = ycor;
		Selectors[i].color = 0xFC38;
		Selectors[i].height = 3;
		Selectors[i].width = 17;
		Selectors[i].needDraw = 0;
		if(i==0){
			Selectors[i].status = Active;
			ST7735_FillRect(Selectors[i].x, Selectors[i].y, Selectors[i].width, Selectors[i].height, Selectors[i].color);
		} else {
			Selectors[i].status = Disabled;
		}
		Selectors[i].needDraw = 0;
		ycor += 22;
	}
}

char Logo_Message[] ="on the TM4C!";
char EngMessage1[] = "Press up";
char EngMessage2[] = "for";
char EngMessage3[] = "English";
char SpanMessage1[] = "Toca abajo";
char SpanMessage2[] = "para";
char SpanMessage3[] = " Espa\xA4ol";
void Logo_Init(void) {
	int i;
	int32_t xcor=5, ycor=85;
	for(i=0; i<6; i++){
		LogoLetters[i].x = xcor;
		LogoLetters[i].y = ycor;
		LogoLetters[i].width = 18;
		LogoLetters[i].height = 18;
		xcor += 20;
	}
	LogoLetters[0].image = wlogo;
	LogoLetters[1].image = ologo;
	LogoLetters[2].image = rlogo;
	LogoLetters[3].image = dlogo;
	LogoLetters[4].image = llogo;
	LogoLetters[5].image = elogo;
	
	for(i=0; i<6;i++){
		ST7735_DrawBitmap(LogoLetters[i].x, LogoLetters[i].y,LogoLetters[i].image, LogoLetters[i].width, LogoLetters[i].height);
	}
	ST7735_DrawString(8, 9, Logo_Message, ST7735_YELLOW);
	
	ST7735_DrawString(7, 2, EngMessage1, ST7735_WHITE);
	ST7735_DrawString(9, 3, EngMessage2, ST7735_WHITE);
	ST7735_DrawString(7, 4, EngMessage3, ST7735_WHITE);
	ST7735_DrawString(6, 12, SpanMessage1, ST7735_WHITE);
	ST7735_DrawString(9, 13, SpanMessage2, ST7735_WHITE);
	ST7735_DrawString(7, 14, SpanMessage3, ST7735_WHITE);
}


// used to initialize main screen
void GameInit(void) {
	// draw blanks over start screen messages
	ST7735_DrawString(7, 2, EngMessage1, ST7735_BLACK);
	ST7735_DrawString(9, 3, EngMessage2, ST7735_BLACK);
	ST7735_DrawString(7, 4, EngMessage3, ST7735_BLACK);
	ST7735_DrawString(6, 12, SpanMessage1, ST7735_BLACK);
	ST7735_DrawString(9, 13, SpanMessage2, ST7735_BLACK);
	ST7735_DrawString(7, 14, SpanMessage3, ST7735_BLACK);
	// initialize letterbox sprites
	LB_Init();
	// initialize selectors, only enable one
	Selectors_Init();
	// draw wordle logo
	ST7735_DrawBitmap(17, 26, wordlelogo, 91, 28);
}

// initializes periodic Systick Interrupt
// used to generate random number
void SysTick_Init(uint32_t period){
  NVIC_ST_CTRL_R = 0;              // 1) disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;   // 2) maximum reload value
  NVIC_ST_CURRENT_R = 0;           // 3) any write to current clears it
  NVIC_ST_CTRL_R = 0x00000005;     // 4) enable SysTick with core clock
}

// initializes edge triggered interrupts
void EdgeTrigger_Init(void){ volatile int delay;      
  SYSCTL_RCGCGPIO_R |= 0x00000010; // (a) activate clock for port E
	delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTE_DIR_R &= ~0x03;    // (c) make PE1 and PE0 in (built-in button)
  GPIO_PORTE_DEN_R |= 0x03;     //     enable digital I/O on PE0, PE1
  GPIO_PORTE_IS_R &= ~0x03;     // (d) PE0, PE1 is edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x03;    //     not both edges
  GPIO_PORTE_IEV_R |= 0x03;    //     rising edge event
  GPIO_PORTE_ICR_R = 0x03;      // (e) clear flag1 and flag0
  GPIO_PORTE_IM_R |= 0x03;      // (f) arm interrupt on PE0, Pe1
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R = 0x00000010;      // (h) enable interrupt 30 in NVIC
}

// initializes port e buttons
void PortE_Init(void) { volatile int delay;
	SYSCTL_RCGCGPIO_R |= 0x00000010; // (a) activate clock for port E
	delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTE_DIR_R &= ~0x0B;    // (c) make PE1 and PE0 in (built-in button)
  GPIO_PORTE_DEN_R |= 0x0B;     //     enable digital I/O on PE0, PE1
}

void PortD_Init(void) { volatile int delay;
	SYSCTL_RCGCGPIO_R |= 0x00000008;
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTD_DIR_R |= 0x43;
	GPIO_PORTD_DEN_R |= 0x43;
}

void Red(void) {
	GPIO_PORTD_DATA_R &= ~0x41;
	GPIO_PORTD_DATA_R |= 0x02;
}

void Yellow(void) {
	GPIO_PORTD_DATA_R &= ~0x03;
	GPIO_PORTD_DATA_R |= 0x40;
}

void Green(void) {
	GPIO_PORTD_DATA_R &= ~0x42;
	GPIO_PORTD_DATA_R |= 0x01;
}