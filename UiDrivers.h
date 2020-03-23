#ifndef UI_DRIVERS_H
#define UI_DRIVERS_H

#include <avr/io.h>
#include <stdint.h>
extern void lcdWrite(void);

extern void lcdDataEn(void);

extern void lcdInstrEn(void);

/***********************************************************
*			Writing data bytes to LCD Display
***********************************************************/
extern void lcdData(uint8_t data);

/***********************************************************
*			Writing instructions to LCD Display
***********************************************************/
extern void lcdInstr(uint8_t data);



/***********************************************************
*		Reading the keypad
***********************************************************/
extern uint8_t readSwitch(void);


extern uint8_t rotaryDir(void);


/***********************************************************
*		Inc/Dec effect parameter value
*		count MUST be a global variable
***********************************************************/
extern void rotCount(uint8_t dir, uint8_t *count);

#endif
