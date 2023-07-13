// Wordle.c
// Runs on TM4C123
// Devin Chaky and Aldo Baez
// This is a starter project for the ECE319K Lab 10

// Last Modified: 1/2/2023 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/ST7735.h"
#include "Random.h"
#include "TExaS.h"
#include "../inc/ADC.h"
#include "../inc/wave.h"
#include "Timer1.h"
#include "Timer0.h"
#include "Initializations.h" // need to make header file
#include "Sprites.h"

extern const char words[2473][6];
extern const char spanishWords[25][6];
extern const char alphabet[27];
extern const unsigned short trophy_confetti[];
extern const unsigned short trophy[];
extern const unsigned short crying[];
extern const unsigned short sad[];

// language type def
typedef enum {English, Spanish} Language_t;
Language_t myLanguage; // my language global
// outcome type def
typedef enum {Win, Lose} outcome_t;
outcome_t Outcome;

// function prototypes
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay_10ms(uint32_t time);
void Generate_Word(Language_t language);
void Change_Selector(void);
void Redraw_Selector(void);
void Redraw_Letter(void);
void Collapse_Logo(logo_t logo);
void Change_Selector(void);	// need to move eventually
void Remove_Bounce(void);
int Assign_Color(void);
void Assign_Score(void);
void Flip_Letter(letterbox_t letter);
void Erase_Selector(selector_t selector);
uint32_t Switch_Debounce(void);
void End_Win(void);
void End_Lose(void);

// Global Variables
char WORD[6];	// global var will hold randomly seleted word
uint32_t ADCdata;	// 12-bit adc data
uint8_t currentIndex; // current index global
uint8_t currentRow;	// current row aka guess
uint32_t TimerCount; // used for delay
uint32_t TotalGuesses;	// holds total number of guesses
uint32_t Score;	// holds total score
status_t GameStatus; // active or not active game

//ISRs
// ADC sample ISR
uint32_t ADCvalue[2];
void Timer1A_Handler(void){ // can be used to perform tasks in background
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
   // execute user task
	ADCdata = ADC_In();	// sample ADC
	ADC_In89(ADCvalue); // sample ADC
	// if slide pot has moved, change index
	if((ADCdata > ((currentIndex +1)*819)) || (ADCdata < ((currentIndex)*819))){
		Change_Selector();
	}
	if(ADCvalue[1] > 3000) {
		Letters[currentRow][currentIndex].alphaind += 1;
		if(Letters[currentRow][currentIndex].alphaind > 26){
			Letters[currentRow][currentIndex].alphaind = 0;
		}
		Letters[currentRow][currentIndex].needDraw = 1;
	}
	if(ADCvalue[1] < 1000) {
		Letters[currentRow][currentIndex].alphaind -= 1;
		if(Letters[currentRow][currentIndex].alphaind < 0){
			Letters[currentRow][currentIndex].alphaind = 26;
		}
		Letters[currentRow][currentIndex].needDraw = 1;
	}
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = 0x00000001;  // acknowledge
  TimerCount++;
}

// edge-triggered buttons ISR
void GPIOPortE_Handler(void){
	if(GPIO_PORTE_RIS_R & 0x02){
		GPIO_PORTE_ICR_R = 0x02;      // acknowledge flag0
		Letters[currentRow][currentIndex].alphaind += 1;
		if(Letters[currentRow][currentIndex].alphaind > 26){
			Letters[currentRow][currentIndex].alphaind = 0;
		}
	}
	if(GPIO_PORTE_RIS_R & 0x01){
		GPIO_PORTE_ICR_R = 0x01;      // acknowledge flag1
		Letters[currentRow][currentIndex].alphaind -= 1;
		if(Letters[currentRow][currentIndex].alphaind < 0){
			Letters[currentRow][currentIndex].alphaind = 26;
		}
	}
	Letters[currentRow][currentIndex].needDraw = 1;
	Switch_Debounce();
}

// MAIN ENGINE
int main(void){ uint32_t i, correct = 0;
	// initialization calls
  DisableInterrupts();
  TExaS_Init(NONE);        // Bus clock is 80 MHz 
  Output_Init();
	Wave_Init();
	SysTick_Init(0x00FFFFFF);
	ADC_Init();
	ADC_Init89();
	Timer1_Init(2666666, 4);	// will sample ADC at 30 Hz
	Timer0_Init(800000, 2);
	PortE_Init();
	PortD_Init();
	EnableInterrupts();
  ST7735_FillScreen(0x0000);            // set screen to black
	
	// set global index and row to 0
	// initialize all other global variables
	currentIndex = 0;
	currentRow = 0;
	TotalGuesses = 0;
	Score = 0;
	GameStatus = Active;

	Logo_Init();	// initialize main screen image and text

	// busy wait for start button press
	while((GPIO_PORTE_DATA_R&0x03) == 0){ } 
	
	// if up press, set language to english
	if((GPIO_PORTE_DATA_R&0x01)!=0){
		myLanguage = English;
	}
	// if down press set language to spanish
	if((GPIO_PORTE_DATA_R&0x02)!=0){
		myLanguage = Spanish;
	}
	
	// generates random word
	Generate_Word(myLanguage); 
	Wave_Piano();	// play start sound
	
	// start image animation
	for(i=0;i<6;i++){
		Collapse_Logo(LogoLetters[i]);
	}
	
	// enable edge trigger int for letter scroll
	DisableInterrupts();
	EdgeTrigger_Init();
	EnableInterrupts();
	GameInit();		// init letter box sprites
	
	//main engine
	while(GameStatus == Active) {
		// turn on LED based on current guess #
		if(TotalGuesses <= 1){
			Green();
		} else if(TotalGuesses <= 3) {
			Yellow();
		} else {
			Red();
		}
		// while guess button not pressed
		// check for selector movement and change letter
		while((GPIO_PORTE_DATA_R&0x08)==0) {
			if(Selectors[currentRow].needDraw == 1) {
				Redraw_Selector();
			}
			if(Letters[currentRow][currentIndex].needDraw == 1) {
				Redraw_Letter();
			}
		}
		TotalGuesses++; // increment total # of guesses
		
		// assign color to guessed letters
		// returns 1 if all guessed correctly
		correct = Assign_Color();	
		Assign_Score();	// add to total score based on # correct guesses
		// flip letters animation
		for(i=0; i<5; i++){
			Flip_Letter(Letters[currentRow][i]);
		}
		Erase_Selector(Selectors[currentRow]);	// clear current selector
		// if all guessed correctly end game in win
		if(correct){
			GameStatus = Disabled;
			Outcome = Win;
			break;
		} 
		// if user does not guess word in 6 tries end game in lose
		else if(TotalGuesses == 6){
			GameStatus = Disabled;
			Outcome = Lose;
			break;
		}
		// if game still active increment to next row
		currentRow++;
		// redraw selector for new row
		Selectors[currentRow].needDraw = 1;
	}
	ST7735_FillScreen(0x0000);	// clear screen for end screen
	
	if(Outcome == Win){
		End_Win();
	} else{
		End_Lose();
	}
}

// HELPER FUNCS
// generates random word
void Generate_Word(Language_t language){ 
	uint32_t randomi, i;
	Random_Init(NVIC_ST_CURRENT_R);
	//generates random english word
	if(language == English) {
		randomi = Random32()%2473;		// will generate random index
		for(i=0; i < 6; i++){			// will generate random word --> create separate function w/ language param 
			WORD[i] = words[randomi][i];
		}
	}
	// generate random spanish word
	if(language == Spanish) {
		randomi = Random32()%25;
		for(i=0; i < 6; i++){			// will generate random word --> create separate function w/ language param 
			WORD[i] = spanishWords[randomi][i];
		}
	}
}

// changes selector position based on ADC sample
void Change_Selector(void){
	Selectors[currentRow].oldx = Selectors[currentRow].x;
	if(ADCdata < 819){
		Selectors[currentRow].x = 15;
		Selectors[currentRow].needDraw = 1;
		currentIndex = 0;
	}
	if(ADCdata >= 819 && ADCdata < 1638) {
		Selectors[currentRow].x = 35;
		Selectors[currentRow].needDraw = 1;
		currentIndex = 1;
	}
	if(ADCdata >= 1638 && ADCdata < 2457) {
		Selectors[currentRow].x = 55;
		Selectors[currentRow].needDraw = 1;
		currentIndex = 2;
	}
	if(ADCdata >= 2457 && ADCdata < 3276) {
		Selectors[currentRow].x = 75;
		Selectors[currentRow].needDraw = 1;
		currentIndex = 3;
	}
	if(ADCdata >= 3276) {
		Selectors[currentRow].x = 95;
		Selectors[currentRow].needDraw = 1;
		currentIndex = 4;
	}
}

// redraws new selector based on row and adc position
void Redraw_Selector(void){
	ST7735_FillRect(Selectors[currentRow].oldx, Selectors[currentRow].y, Selectors[currentRow].width, Selectors[currentRow].height, ST7735_BLACK);
	ST7735_FillRect(Selectors[currentRow].x, Selectors[currentRow].y, Selectors[currentRow].width, Selectors[currentRow].height, Selectors[currentRow].color);
	Wave_Menuclick();
	Selectors[currentRow].needDraw = 0;
}

// clears current selector
void Erase_Selector(selector_t selector) {
	ST7735_FillRect(selector.x, selector.y, selector.width, selector.height, ST7735_BLACK);
}

// redraws letter if alpha index changed
void Redraw_Letter(void) {
	char newLetter;
	newLetter = alphabet[Letters[currentRow][currentIndex].alphaind];
	Letters[currentRow][currentIndex].letter = newLetter;
	ST7735_DrawCharS((Letters[currentRow][currentIndex].x +2), (Letters[currentRow][currentIndex].y +1), newLetter, ST7735_BLACK, ST7735_WHITE, 2);
	Letters[currentRow][currentIndex].needDraw =0;
}

// 10 ms delay using Systick Periodic Timer
void Delay_10ms(uint32_t time){
	TimerCount = 0;
	Timer0A_Start();
	while(TimerCount != time){ } // wait for timer
	Timer0A_Stop();
}

// collapse logo animation
// slowly draws over top and botom of bitmap 
// until it disappears
void Collapse_Logo(logo_t logo) {
	char blank[] ="on the TM4C!";
	uint32_t height, width, i;
	int32_t x, bottomy, topy;
	
	height = logo.height;
	width = logo.width;
	x = logo.x;
	bottomy = logo.y;
	topy = logo.y - height +1;
	
	for(i=0; i< height/2; i++){
		ST7735_FillRect(x, topy, width, 1, ST7735_BLACK);
		ST7735_FillRect(x, bottomy, width, 1, ST7735_BLACK);
		Delay_10ms(3);
		topy++;
		bottomy--;
	}
	ST7735_DrawString(8, 9, blank, ST7735_BLACK);
}

// same method for collapse letter,
//but does it both in and out to give flip illusion
void Flip_Letter(letterbox_t letter){
	uint32_t height, width, i;
	int32_t x, bottomy, topy;
	
	height = letter.height;
	width = letter.width;
	x = letter.x;
	topy = letter.y;
	bottomy = letter.y + height -1;
	
	for(i=0; i<height/2; i++){
		ST7735_FillRect(x, topy, width, 1, ST7735_BLACK);
		ST7735_FillRect(x, bottomy, width, 1, ST7735_BLACK);
		Delay_10ms(3);
		topy++;
		bottomy--;
	}
	ST7735_FillRect(x, bottomy, width, 1, letter.color);
	
	topy--;
	bottomy++;
	for(i=0; i<height/2;i++){
		ST7735_FillRect(x, topy, width, 1, letter.color);
		ST7735_FillRect(x, bottomy, width, 1, letter.color);
		Delay_10ms(3);
		topy--;
		bottomy++;
	}
	ST7735_DrawCharS((letter.x +2), (letter.y +1), letter.letter, ST7735_WHITE, letter.color, 2);
}
	
// assign letter color based on if correct
// and in the right position
// will return 1 if all letters are correct
// and 0 otherwise
int Assign_Color(void){
	uint32_t checked, i, j, correct=0;
	for(i=0; i<5; i++){
		checked = 0;
		for(j=0; j<5; j++){
			if(Letters[currentRow][i].letter == WORD[j]){
				if(i == j){
					Letters[currentRow][i].color = 0x45C9;
					correct++;
					checked=1;
				} else if(checked ==0){
					Letters[currentRow][i].color = 0x1E1F;
				}
			}
		}
		if(Letters[currentRow][i].color == ST7735_WHITE){
			Letters[currentRow][i].color = 0xC594;
		}
	}
	if(correct==5){
		return 1;
	}
	return 0;
}

// adds to total score based on number
// of correct guesses
void Assign_Score(void) {
	uint32_t i;
	uint16_t color;
	for(i=0; i<5; i++){
		color = Letters[currentRow][i].color;
		if(color == 0x45C9){
			Score += (7-TotalGuesses)*20;
		}
		if(color == 0x1E1F){
			Score += (7-TotalGuesses)*5;
		}
	}
}

// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 62.5 nsec for 16 MHz clock)
void SysTick_Wait(uint32_t delay){
  volatile uint32_t elapsedTime;
  uint32_t startTime = NVIC_ST_CURRENT_R;
  do{
    elapsedTime = (startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF;
  }
  while(elapsedTime <= delay);
}

//------------Switch_Debounce------------
// Read and return the status of the switch 
// Input: none
// Output: 0x02 if PB1 is high
//         0x00 if PB1 is low
// debounces switch
uint32_t Switch_Debounce(void){
uint32_t in,old,time; 
  time = 1000; // 10 ms
  old = GPIO_PORTE_DATA_R&0x03;
  while(time){
    SysTick_Wait(160); // 10us
    in = GPIO_PORTE_DATA_R&0x03;
    if(in == old){
      time--; // same value 
    }else{
      time = 1000;  // different
      old = in;
    }
  }
  return old;
}

char winMessage_eng[] = "You Win!";
char scoreMessage_eng[] = "Score: ";
char winMessage_span[] = "Tu Ganaste!";
char scoreMessage_span[] = "Puntos: ";
void End_Win(void) {
		Score += 400;
		if(myLanguage == English){
			ST7735_DrawString(7, 2, winMessage_eng, ST7735_WHITE);
			ST7735_DrawString(5, 13, scoreMessage_eng, ST7735_WHITE);
		} else {
			ST7735_DrawString(5, 2, winMessage_span, ST7735_WHITE);
			ST7735_DrawString(4, 13, scoreMessage_span, ST7735_WHITE);
		}
		ST7735_DrawBitmap(25, 120, trophy_confetti, 80, 76);
		ST7735_SetCursor(12, 13);
		ST7735_OutUDec(Score);
		Wave_Win();
		while(1){
			Delay_10ms(100);
			ST7735_DrawBitmap(25, 120, trophy, 80, 76);
			Delay_10ms(100);
			ST7735_DrawBitmap(25, 120, trophy_confetti, 80, 76);
		}
	}

char loseMessage_eng[] = "You Lose";
char loseMessage_span[] = "Tu Perdiste";
char endMessage_eng[] = "The Word Was: ";
char endMessage_span[] = "Palabra era: ";
void End_Lose(void) {
	if(myLanguage == English){
		ST7735_DrawString(7, 2, loseMessage_eng, ST7735_WHITE);
		ST7735_DrawString(6, 13, scoreMessage_eng, ST7735_WHITE);
		ST7735_DrawString(1, 14, endMessage_eng, ST7735_WHITE);
	} else {
		ST7735_DrawString(6, 2, loseMessage_span, ST7735_WHITE);
		ST7735_DrawString(6, 13, scoreMessage_span, ST7735_WHITE);
		ST7735_DrawString(1, 14, endMessage_span, ST7735_WHITE);
	}
		ST7735_DrawBitmap(25, 115, sad, 80, 76);
		ST7735_SetCursor(13, 13);
		ST7735_OutUDec(Score);
		ST7735_DrawString(15, 14, WORD, ST7735_WHITE);
		while(1){
			Delay_10ms(100);
			ST7735_DrawBitmap(25, 115, crying, 80, 74);
			Delay_10ms(100);
			ST7735_DrawBitmap(25, 115, sad, 80, 76);
		}
	}

// ********************* IN STARTER PROJECT ********************************
// not currently using but DO NOT DELETE
/*
typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};*/
/*
int main1(void){char l;
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Random_Init(1);

  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  
  ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom

  ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  ST7735_DrawBitmap(100, 9, SmallEnemy30pointB, 16,10);

  //Delay100ms(50);              // delay 5 sec at 80 MHz

  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(1, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(1, 2);
  ST7735_OutString("Nice try,");
  ST7735_SetCursor(1, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(2, 4);
  ST7735_OutUDec(1234);
  while(1){
  }

	// start of what was originally in main
	for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
 // Delay100ms(30);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
		Wave_Win();
//    Delay100ms(20);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  } 
}
*/
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

