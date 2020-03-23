#include "UiDrivers.h"

#include "InterruptHandlers.h"
#include "IoLines.h"
#include "Macros.h"
#include "Utilities.h"


void lcdWrite(void)
{
	PORTB |= BIT(LCD_E);
	delay(800); //200
	PORTB &= ~BIT(LCD_E);
}

void lcdDataEn(void)	//Set R/S pin high
{
	PORTB |= BIT(LCD_RS);
}

void lcdInstrEn(void)	  //Set R/S pin low
{
	PORTB &= ~BIT(LCD_RS);
}

/***********************************************************
 *			Writing data bytes to LCD Display
 ***********************************************************/
void lcdData(uint8_t data)
{
	PORTA = data;
	lcdDataEn();
	lcdWrite();
	delay(60);  //10
	PORTA = 0x00;

}

/***********************************************************
 *			Writing instructions to LCD Display
 ***********************************************************/
void lcdInstr(uint8_t data)
{
	PORTA = data;
	lcdInstrEn();
	lcdWrite();
	delay(300);  //50
	PORTA = 0x00;

}



/***********************************************************
 *		Reading the keypad
 ***********************************************************/
uint8_t readSwitch(void)
{
	uint8_t switches;

	DDRA = 0xC0;
	PORTA = 0x00;
	delay(80);
	PORTB &= ~BIT(Sw_nREAD);  //Set switch_buffer CS pin active low
	delay(80);
	switches = PINA & 0x3F;  // Read switches
	PORTB |= BIT(Sw_nREAD);   //Set switch_buffer CS pin high
	delay(80);
	PORTA = 0x00;
	delay(80);
	DDRA = 0xFF;

	return switches;
}


uint8_t rotaryDir(void)
{
	uint8_t dir;

	if (count_up == 1)
	{
		dir = 1;
	}
	else if (count_down == 1)
	{
		dir = 2;
	}
	count_up = 0;
	count_down = 0;

	return dir;
}


/***********************************************************
 *		Inc/Dec effect parameter value
 *		count MUST be a global variable
 ***********************************************************/
void rotCount(uint8_t dir, uint8_t *count)
{

	if ((dir == 1) && (*count < 100))
	{
		(*count)++;
	}
	else if ((dir == 2) && (*count > 0))
	{
		(*count)--;
	}
}
