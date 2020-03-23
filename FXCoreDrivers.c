#include "FXCoreDrivers.h"

#include "Constants.h"
#include "Macros.h"
#include "Utilities.h"
#define READ   		0b00000011

uint8_t fpgaConfig(void)
{
	uint32_t bin_file_index;

	uint8_t FPGA_section_status = 0;

	/**************** Get Flash ready for reading *****************/
	flashEn();
	delay(30);
	spiTx(READ);		   // Send READ instr to flash
	spiTx(0);	// MSB of address
	spiTx(0);
	spiTx(0); // LSB of address

	/************** Get the FPGA ready for config ****************/
	delay(1000); // give FPGA time to clear (500us)  0.5*1000
	PORTG = PING | BIT(nPROG_B);  // set Program pin inactive high
	delay(500); // arbitrary delay?
	PORTG = PING | BIT(nINIT_B);  // set Init pin inactive high
	delay(30); // delay of 4us
	PORTB &= ~BIT(Sw_nREAD);  //enable clk and data lines to FPGA
	bin_file_index = 0;
	delay(30);
	SPCR |= BIT(DORD);     // FPGA bitstream requires LSB first
	delay(30);
	while(((PING & BIT(PROG_DONE)) == 0) && (bin_file_index < BINFILE_LIMIT))  // do loop until Done pin goes high
	{
		spiRx();// reading flash, which sends data to FPGA
		bin_file_index++;
	}
	spiRx();
	spiRx();
	spiRx();
	PORTB |= BIT(Sw_nREAD);   //disable clk and data lines to FPGA
	SPCR &= ~BIT(DORD);   // set back to MSB first

	flashDis();

	if(bin_file_index == BINFILE_LIMIT)
	{
		FPGA_section_status = 1;
	}
	return FPGA_section_status;
}


/***********************************************************
 *
 *
 *
 *
 ***********************************************************/
void fpgaWrEn(void)
{
	PORTD = PIND | BIT(WRnRD);
}

void fpgaRdEn(void)
{
	PORTD = PIND & ~BIT(WRnRD);
}

//*****************************************************

void fpgaWr(uint8_t data)
{
	DDRC = 0xFF;
	PORTC = data;
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	PORTD = PIND | BIT(3);

	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	PORTD = PIND & ~BIT(3);
	NOP();

	DDRC = 0x00;

}

//*****************************************************

uint8_t fpgaRd(void)
{
	uint8_t data;

	DDRC = 0x00;
	NOP();
	NOP();
	NOP();
	NOP();
	PORTD = PIND | BIT(3);
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	data = PINC;
	NOP();
	NOP();
	NOP();
	NOP();
	PORTD = PIND & ~BIT(3);
	DDRC = 0x00;

	return data;
}

void fxcoreHoldOn(void)
{
	PORTD = PIND | BIT(FXCore_Hold);
}

void fxcoreHoldOff(void)
{
	PORTD = PIND & ~BIT(FXCore_Hold);
}

void fpgaRstEn(void)
{
	PORTE = PINE | BIT(RST);
}
void fpgaRstDis(void)
{
	PORTE = PINE & ~BIT(RST);
}



/***********************************************************
 *		Writing to the FPGA
 ***********************************************************/
void fxcoreWrite(uint8_t stage, uint32_t address, FXCoreWord_t input_instr)
{
	uint32_t addr;
	uint8_t address1, address2, address3;
	uint8_t data1, data2, data3, data4;

	addr = address + 2048*stage;
	address1 = (addr >> 16) & 0x07; // Set up High Address Byte on port
	address2 = (addr >> 8) & 0xFF; // Set up Mid Address on port
	address3 = (addr) & 0xFF; // Set up Low Address uint8_t on port

	data1 = input_instr.main;
	data2 = input_instr.data1;
	data3 = input_instr.data2;
	data4 = input_instr.data3;
	DDRC = 0xFF;

	fpgaWrEn(); // Tell FPGA to fetch data from memory
	NOP();
	NOP();
	if ((PIND & BIT(5)) == 0)   //FXCore_Hold is off
	{
		while((PIND & 0x04) == 0){NOP();}   // Start is low
	}
	fpgaWr(address1); // writing addresses to FPGA/MCU Int port, High uint8_t first
	fpgaWr(address2);
	fpgaWr(address3);

	fpgaWr(data1);   // writing data to FPGA/MCU Int port, High uint8_t (main instr uint8_t) first
	fpgaWr(data2);
	fpgaWr(data3);
	fpgaWr(data4);

	fpgaRdEn();
	DDRC = 0x00;
}


/***********************************************************
 *		Reading from the FPGA
 ***********************************************************/
FXCoreWord_t fxcoreRead(uint8_t stage, uint32_t address)
{
	uint32_t addr;
	uint8_t address1, address2, address3;
	FXCoreWord_t output_instr;

	addr = address + 2048*stage;
	address1 = (addr >> 16) & 0x07; // Set up Address Byte3 on port
	address2 = (addr >> 8) & 0xFF; // Set up Address Byte2 on port
	address3 = (addr) & 0xFF; // Set up Address Byte1 on port

	fpgaRdEn(); // Tell FPGA to fetch data from memory
	NOP();
	NOP();
	DDRC = 0xFF;
	if ((PIND & BIT(5)) == 0)   //FXCore_Hold is off
	{
		while((PIND & 0x04) == 0){ NOP();}   // Start is low
	}

	fpgaWr(address1); // writing addresses to FPGA/MCU Int port, High uint8_t first
	fpgaWr(address2);
	fpgaWr(address3);

	DDRC = 0x00;	  // set port for data reading

	if ((PIND & BIT(5)) == 0)   //FXCore_Hold is off
	{
		while((PIND & 0x04) == 0){ NOP();}   // Start is low
	}

	output_instr.main = fpgaRd();
	output_instr.data1 = fpgaRd();
	output_instr.data2 = fpgaRd();
	output_instr.data3 = fpgaRd();

	return output_instr;
}

