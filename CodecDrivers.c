#include "CodecDrivers.h"

#include "CommDrivers.h"
#include "IoLines.h"
#include "Macros.h"
#include "Utilities.h"

void codecEn(void)
{
	PORTB &= ~BIT(CodecAdr);
}

void codecDis(void)
{
	PORTB |= BIT(CodecAdr);
}

void codecRst(void)
{
	PORTG &=  ~BIT(0);
	delay(1000);
	PORTG |=  BIT(0);
}
/***********************************************************
 *		Writing data to the codec
 ***********************************************************/
void codecWrite(uint8_t address, uint8_t data)
{
	codecEn();
	spiTx(address);
	spiTx(data);
	codecDis();
}



/***********************************************************
 *				Configuring the codec
 ***********************************************************/
void configCodec(void)
{
	codecWrite(64,0b11000001);
	codecWrite(65,0b11111111);
	codecWrite(66,0b11111111);
	codecWrite(67,0b00000001);
	codecWrite(68,0b01000000);
	codecWrite(69,0b00000000);
	codecWrite(70,0b11010110);
	codecWrite(71,0b11010110);
	codecWrite(72,0b00000001);
	codecWrite(73,0b00000000);
}
