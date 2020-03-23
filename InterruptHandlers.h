#ifndef INTERRUPT_HANDLERS_H
#define INTERRUPT_HANDLERS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

extern uint8_t count_up;
extern uint8_t count_down;
extern uint8_t count_dir;
extern uint8_t getRxDone(void);
extern void setRxDone(uint8_t rxDone);
extern uint32_t getRxCount(void);
extern void setRxCount(uint32_t rxCount );

/*************************************************
*             External Interrupt Handler
*			  for rotary encoder
**************************************************/
ISR(INT0_vect);

/*************************************************
*             Pin Change Interrupt Handler
**************************************************/
ISR(PCINT0_vect);

/*************************************************
*             SPI Interrupt Handler
*
**************************************************/
ISR(SPI_STC_vect);

/*************************************************
*
*             UART RX Interrupt Handler
*
**************************************************/
ISR(USART0_RX_vect);



ISR(TIMER0_OVF_vect);




#endif
