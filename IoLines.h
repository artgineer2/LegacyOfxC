#ifndef IO_LINES_H
#define IO_LINES_H
#include <avr/io.h>

//PORTB:LCD control lines
#define FLASH_nCS		7
#define LCD_E		5
#define LCD_RS		4
//PORTB:Switch,Codec
#define Sw_nREAD	0
#define CodecAdr	6
//PORTD
#define RotEncA		1
//PORT D:FPGA
#define FXCore_Hold 5
#define	WRnRD		4
#define	Int_Clk	 	3
#define	Start		2
#define RST			2
//PORTE:EEPROM
#define EE_nCS		4
//PORT G:FPGA programming from flash
#define PROG_DONE	1
#define nINIT_B		2
#define nPROG_B		3

#endif
