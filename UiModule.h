/*
 * UiModule.h
 *
 *  Created on: Mar 28, 2020
 *      Author: buildrooteclipse
 */

#ifndef UIMODULE_H_
#define UIMODULE_H_

#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "Utilities.h"

extern uint8_t readSwitch(void);


extern uint8_t rotaryDir(void);
extern void rotCount(uint8_t dir, uint8_t *count);

extern void setBuffer1(char *string);
extern void setBuffer2(char *string);
extern void setIndBuf(char *string);
extern void setAbbrBuffer(char *string);

extern char *getBuffer1(void);
extern char *getBuffer2(void);
extern char *getIndBuf(void);
extern char *getAbbrBuffer(void);
extern void buffer1Cat(char *chr);
extern void buffer2Cat(char *chr);
extern void buffer1ByteCat(uint8_t byte);
extern void buffer2ByteCat(uint8_t byte);
extern void buffer1UIntCat(uint16_t byte);
extern void buffer2UIntCat(uint16_t byte);
extern void buffer1ULongCat(uint32_t byte);
extern void buffer2ULongCat(uint32_t byte);
extern void clearIndBuffer(void);
extern void indBufCat(char *chr);
extern void clearBuffer1(void);
extern void clearBuffer2(void);
extern void clearAbbrBuffer(void);
extern void lcdInit(void);

extern void uiIndBufLoad(uint8_t state, uint8_t index, uint8_t index_max);

extern void capInd(char *target, char *source, uint8_t select);

extern void Display(char *line1, char *line2);

extern void uiStageClear(uint8_t stage_index);

extern void debugDisplay(uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4, uint16_t data5, uint16_t data6);

#endif /* UIMODULE_H_ */
