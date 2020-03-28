
#include "ChipInit.h"
#include "CommModule.h"
#include "Macros.h"
/***********************************************************
 *		IO Configuration
 ***********************************************************/
void ioConfig(void)
{	 	 //76543210
	DDRA  = 0b11111111;
	PORTA = 0b00000000;
	//76543210
	DDRB  = BIT(FLASH_nCS)|BIT(LCD_E)|BIT(LCD_RS)|BIT(Sw_nREAD)|BIT(CodecAdr)|BIT(2)|BIT(1); //Enable SPI MOSI and SCK
	PORTB = 0b01000001;
	//76543210
	DDRC  = 0b11111111;
	PORTC = 0b00000000;
	//76543210
	DDRD  = 0b11111000;
	PORTD = 0b00000000;
	//76543210
	DDRE  = 0b11011111;
	PORTE = 0b00000000;
	//76543210
	DDRF  = 0b11111110;
	PORTF = 0b00000000;
	//76543210
	DDRG  = 0b00001001;  // set pin 4 (PROG_B) and pin 0 (codecRst) to output
	PORTG = 0b00000010;  // set pin 1 (PROG_DONE) to be a pulled-up input port
}

//Watchdog initialize
// prescale: 512K
void watchdogInit(void)
{
	WDR(); //this prevents a timeout on enabling
	WDTCR |= (1<<WDCE) | (1<<WDE);
	WDTCR = 0x00;

}

void counter0Init(uint8_t prescaler)
{
	TCCR0A = prescaler; // 1 -> 5
	TIMSK0 = 0x01;  // enable overflow interrupt
}


/***********************************************************
 *				Configuring the chip
 ***********************************************************/
void configChip(void)
{
	CLI(); //disable all interrupts
	ioConfig();
	uartConfig();
	spiConfig();

	MCUCR = 0x00;
	EICRA = 0x00; /*enable active low INT0 */
	EIMSK = 0x01; /*enable Ext interrupt*/
	PCMSK0 = 0x00; /*disable individual masks*/
	PCMSK1 = 0x00; //
	TIMSK0 = 0x00; /*timer interrupt sources*/
	TIMSK1 = 0x00;
	TIMSK2 = 0x00;
	SEI(); /*re-enable interrupts*/
}
