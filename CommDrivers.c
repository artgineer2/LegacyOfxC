
#include "CommDrivers.h"
#include "Utilities.h"
#include "Constants.h"
#include "IoLines.h"
#include "Macros.h"

/*********************************************************************
 *
 *                   PC Transmit Routine
 *					input: start address of data to be sent.
 *					bytes_out: determines total number of bytes sent.
 *					     Used to keep TX length constant with
 *						 varying data buffer lengths.
 *
 *********************************************************************/

static char txBuffer[TX_BUFFER_SIZE+10];
static uint16_t txCount;

void setTxCount(uint16_t _txCount)
{
	txCount = _txCount;
}
void setTxBuffer(uint8_t *str)
{
	strncpy((char *)txBuffer, (char *)str,TX_BUFFER_SIZE);
}
char *getTxBuffer(void)
{
	return txBuffer;
}
void txBufferCat(char *chr)
{
	strcat((char *)txBuffer,chr);
}

void txBufferByteCat(uint8_t byte)
{
	byteCat(txBuffer,byte);
}

uint16_t getTxCount(void)
{
	return txCount;
}
void clearTxBuffer(void)
{
	memcpy(txBuffer, '\0',TX_BUFFER_SIZE);
}

/***********************************************************
 *		UART Configuration
 ***********************************************************/
void uartConfig(void)
{
	UCSR0B = 0x00;
	UCSR0A = BIT(U2X0);
	UCSR0C = BIT(UCSZ01)|BIT(UCSZ00);
	UBRR0H = 0x00;
	UBRR0L = 12; // 1 = 250Kbd, 12 = 38.4Kbd, 12 = 76.8Kbd w/ U2X0 = 1
	UCSR0B = BIT(RXCIE0)|BIT(RXEN0)|BIT(TXEN0);
}

/***********************************************************
 *		SPI Configuration
 ***********************************************************/
void spiConfig(void)
{
	SPCR = BIT(SPE)|BIT(MSTR);
	SPSR = 0X00;
}

/*****************************************************/
/*****************************************************/
/*****************************************************/

void spiTx(uint8_t data)
{
	char dummy = 0;
	SPDR = data;
	while(!(SPSR & BIT(SPIF))){} //Wait for Tx to complete
	dummy = SPDR; //access SPDR to clear SPIF bit
}

uint8_t spiRx(void)
{
	uint8_t data;
	SPDR = 0x00;
	while(!(SPSR & BIT(SPIF))){} //Wait for Rx to complete
	data = SPDR;
	return data;
}

/*****************************************************/
/*****************************************************/
/*****************************************************/

void uartTx(uint8_t data)
{
	UDR0 = data;
	while(!(UCSR0A & BIT(TXC0))){} //Wait for Tx to complete
	UCSR0A |= BIT(TXC0);
}

/*****************************************************/
/*****************************************************/
/*****************************************************/



void print(char *input, uint16_t bytes_out)
{
	uint16_t m;


	for(m = 0; m < bytes_out; m++)
	{
		uartTx(input[m]);
	}
}


