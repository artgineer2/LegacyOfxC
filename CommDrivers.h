#ifndef COMM_DRIVERS_H
#define COMM_DRIVERS_H



#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
/*********************************************************************
*
*                   PC Transmit Routine
*					input: start address of data to be sent.
*					bytes_out: determines total number of bytes sent.
*					     Used to keep TX length constant with
*						 varying data buffer lengths.
*
*********************************************************************/
extern void uartConfig(void);
extern void spiConfig(void);

extern void print(char *input, uint16_t bytes_out);

extern void spiTx(uint8_t data);
extern uint8_t spiRx(void);

extern void uartTx(uint8_t data);

extern void setTxBuffer(uint8_t *str);
extern char *getTxBuffer(void);
extern void txBufferCat(char *chr);
extern void txBufferByteCat(uint8_t byte);
extern void setTxCount(uint16_t txCount);
extern uint16_t getTxCount(void);
extern void clearTxBuffer(void);

#endif
