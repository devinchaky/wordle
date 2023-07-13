// wave.h 
// playing wav files.
//
// For use with the TM4C123
// Sounds for ECE319K Lab 10
// Program written by: Devin Chaky
// 4/21/23

#ifndef __WAVE_h
#define __WAVE_h
#include <stdint.h>
// 6-bit sounds, 11.025kHz sampling
#define shootsize 4080
extern const uint8_t shoot[4080];
#define invaderkilledsize 3377
extern const uint8_t invaderkilled[3377] ;
#define explosionsize 2000
extern const uint8_t explosion[2000] ;
#define fastinvader1size 982
extern const uint8_t fastinvader1[982];
#define fastinvader2size 1042
extern const uint8_t fastinvader2[1042];
#define fastinvader3size 1054
extern const uint8_t fastinvader3[1054] ;
#define fastinvader4size 1098
extern const uint8_t fastinvader4[1098] ;
#define highpitchsize 1802
extern const uint8_t highpitch[1802];
#define flaunchsize 6204
extern const uint8_t flaunch[6204];
#define menuclicksize 2511
extern const uint8_t menuclick[2511];
#define pianosize 26344
extern const uint8_t piano[26344];
#define winsize 21891
extern const uint8_t win[21891];

//******* Wave_Start ************
// These functions do not output to the DAC. 
// Rather, they set a pointer and counter, and then enables the timer interrupt.
// It starts the sound, and the timer ISR does the output
// feel free to change the parameters
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Wave_Start(const uint8_t *pt, uint32_t count);
// stop wave output
void Wave_Stop(void);
// start playing shoot
void Wave_Shoot(void);
// start playing invaderkilled
void Wave_Killed(void);
// start playing explosion
void Wave_Explosion(void);
// start playing highpitch
void Wave_Highpitch(void);
void Wave_Flaunch(void);
void Wave_Menuclick(void);
void Wave_Piano(void);
void Wave_Win(void);
void Wave_Init(void);
#endif
