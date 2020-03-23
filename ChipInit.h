#ifndef CHIP_INIT_H
#define CHIP_INIT_H

#include <avr/interrupt.h>

#include <stdlib.h>
#include <ctype.h>
#include "IoLines.h"
/***********************************************************
*		IO Configuration
***********************************************************/
extern void ioConfig(void);

extern void watchdogInit(void);
extern void counter0Init(uint8_t prescaler);

/***********************************************************
*				Configuring the chip
***********************************************************/
extern void configChip(void);

#endif
