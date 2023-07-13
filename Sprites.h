#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#ifndef Sprites
#define Sprites
//**************************************************************************
// Letterbox Sprite
// will hold the letter guesses
struct letterbox {
	char letter;
	int8_t alphaind;
	int32_t x;
	int32_t y;
	uint8_t index;
	uint16_t color;
	uint8_t row;
	uint32_t width;
	uint32_t height;
	uint32_t needDraw;
};
typedef struct letterbox letterbox_t;
extern letterbox_t Letters[6][5];	// share array of letter boxes

// Selector Sprite
// will be used by user to select which letter to change
// six selectors are initialized at beginning (one for each row)
// but only one is activated on screen
typedef enum {Active, Disabled} status_t;
struct selector {
	uint8_t row;
	uint8_t oldx;
	uint8_t oldy;
	int32_t x;
	int32_t y;
	uint16_t color;
	uint32_t width;
	uint32_t height;
	status_t status;
	uint32_t needDraw;
};
typedef struct selector selector_t;
extern selector_t Selectors[6]; // share array of Selectors

// Logo Sprite
// start screen logo sprite
// used for animated start screen logo
struct logo {
	int32_t x;
	int32_t y;
	const unsigned short *image;
	uint32_t width;
	uint32_t height;
};
typedef struct logo logo_t;
extern logo_t LogoLetters[6]; // share array of Logo letters
#endif
//***********************************************************************************