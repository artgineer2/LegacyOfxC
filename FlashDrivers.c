

#include "FlashDrivers.h"
#include "CommDrivers.h"
#include "IoLines.h"
#include "Utilities.h"

// Flash instructions
#define WREN   		0b00000110
#define WRDI   		0b00000100
#define RDSR   		0b00000101
#define WRSR   		0b00000001
#define READ   		0b00000011
#define WRITE  		0b00000010
#define ERASE4K		0b00100000
#define ERASE64K	0xD8
#define CHIPERASE 	0x60
#define SQWRITE 	0b10101101
//Flash Status Register
#define nRDY	0	// nReady/Busy Status
#define WEL		1	// Write Enable Latch Status
#define SWP0	2	// Software Protection Status, bit 0
#define SWP1	3	// Software Protection Status, bit 1
#define WPP		4	// Write Protect Pin Status (active low)
#define EPE		5	// Erase/Program Error
#define SPM		6	// Sequential Program Mode Status
#define SPRL	7 	// Section Protection Registers Locked



/***********************************************************
 *                    Flash Routines
 ***********************************************************/
void flashEn(void)
{
	PORTB &= ~BIT(FLASH_nCS);
}

void flashDis(void)
{
	PORTB |= BIT(FLASH_nCS);
}


/***********************************************************
 *		Setting flash Write Enable
 *		(Do this before writing to the flash)
 *
 ***********************************************************/
void flashWREN(void)
{
	flashEn();
	spiTx(WREN);
	flashDis();
}

/***********************************************************
 *		Resetting flash Write Enable
 *		(Do this after writing to the flash)
 *
 ***********************************************************/
void flashWRDI(void)
{
	flashEn();
	spiTx(WRDI);
	flashDis();
}

/***********************************************************
 *		Reading flash Status Register
 ***********************************************************/
uint8_t flashRDSR(void)
{
	uint8_t x;
	flashEn();
	spiTx(RDSR);
	x = spiRx();
	flashDis();
	return x;
}

void flashInit(void)
{
	flashWREN();
	flashEn();
	spiTx(WRSR);
	spiTx(0x00);
	flashDis();
}

/***********************************************************
 *
 *				Reading from the flash
 *
 *				data_status:  	0:1st uint8_t
 *								1:sequence uint8_t
 *								2:Last uint8_t
 *
 ***********************************************************/
uint8_t flashRead(uint32_t address)
{
	uint8_t data;

	flashEn();
	spiTx(READ);		   // Read instruction
	spiTx((address >> 16) & 0x0000FF);	// MSB of address
	spiTx((address >> 8) & 0x0000FF);
	spiTx(address & 0x0000FF); // LSB of address
	data = spiRx();
	flashDis();

	return data;
}

/***********************************************************
 *
 *				Writing to the flash
 *
 ***********************************************************/
uint8_t flashWrite(uint32_t address, uint8_t data)
{
	uint8_t status;

	flashWREN();
	flashEn();

	spiTx(WRITE);		   // Write instruction
	spiTx((address >> 16) & 0x0000FF);	// MSB of address
	spiTx((address >> 8) & 0x0000FF);
	spiTx(address & 0x0000FF); // LSB of address
	spiTx(data);
	flashDis();
	while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)

	if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;
	else status = 0;

	return status;
}


/***********************************************************
 *
 *				Writing sequence to the flash (from the PC)
 *
 ***********************************************************/
uint8_t flashSeqWrite(uint32_t start_address, uint8_t data_in, uint8_t byte_status)
{
	uint8_t status;

	status = 0;
	switch(byte_status)
	{
	case 0:  // First uint8_t in sequence
		flashWREN();
		flashEn();
		spiTx(SQWRITE);		   // Sequential Write instruction
		spiTx((start_address >> 16) & 0x0000FF);	// MSB of address
		spiTx((start_address >> 8) & 0x0000FF);
		spiTx(start_address & 0x0000FF); // LSB of address
		spiTx(data_in);
		flashDis();
		while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)

		if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;

		break;
	case 1:  // Middle uint8_t in sequence
		flashEn();
		spiTx(SQWRITE);		   // Sequential Write instruction
		spiTx(data_in);
		flashDis();
		while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)

		if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;

		break;
	case 2: // Last uint8_t in sequence
		flashEn();
		spiTx(SQWRITE);		   // Sequential Write instruction
		spiTx(data_in);
		flashDis();
		while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();}//Wait for nRDY bit to be set (low)

		if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;

		flashWRDI();
		break;
	default:
		break;
	}
	return status;
}


/***********************************************************
 *
 *				Reading array from the flash
 *
 ***********************************************************/
uint8_t flashArrayRead(uint32_t start_address, uint8_t *data_out, uint8_t array_size)
{
	uint8_t i,status;

	flashEn();

	spiTx(READ);		   // Write instruction
	spiTx((start_address >> 16) & 0x0000FF);	// MSB of address
	spiTx((start_address >> 8) & 0x0000FF);
	spiTx(start_address & 0x0000FF); // LSB of address
	for(i = 0; i < array_size; i++)
	{
		data_out[i] = spiRx();
	}
	data_out[array_size] = '\0';

	flashDis();

	if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;
	else status = 0;

	return status;
}


/***********************************************************
 *
 *				Writing array to the flash
 *
 ***********************************************************/
uint8_t flashArrayWrite(uint32_t start_address, uint8_t *data_in, uint8_t array_size)
{
	uint8_t i,status;

	flashWREN();
	flashEn();

	spiTx(SQWRITE);		   // Sequential Write instruction
	spiTx((start_address >> 16) & 0x0000FF);	// MSB of address
	spiTx((start_address >> 8) & 0x0000FF);
	spiTx(start_address & 0x0000FF); // LSB of address
	spiTx(data_in[0]);
	flashDis();
	delay(30);
	while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)

	for(i = 1; i < array_size; i++)
	{
		flashEn();
		spiTx(SQWRITE);
		spiTx(data_in[i]);
		flashDis();
		delay(30);  // try to use enough delay that the status reading isn't needed
		while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)
	}
	flashWRDI();

	if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;
	else status = 0;

	return status;
}

/***********************************************************
 *		Erasing 4K flash blocks
 *		(erases entire FXCore Section)
 ***********************************************************/
uint8_t flash4kErase(uint32_t starting_address)
{
	uint32_t block_addr;
	uint8_t status;

	block_addr = starting_address;
	flashWREN();
	delay(30);
	flashEn();
	spiTx(ERASE4K);		   // Erase instruction
	spiTx((block_addr & 0xFF0000) >> 16);	// MSB of address
	spiTx((block_addr  & 0x00FF00) >> 8);
	spiTx(block_addr & 0x0000FF); // LSB of address
	flashDis();
	delay(30);
	while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)
	if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;
	else status = 0;

	return status;
}

/***********************************************************
 *		Erasing 64K flash blocks
 *		(erases half of FPGA section)
 ***********************************************************/
uint8_t flash64kErase(uint32_t starting_address)
{
	uint32_t block_addr;
	uint8_t status;

	block_addr = starting_address;
	flashWREN();
	delay(30);
	flashEn();
	spiTx(ERASE64K);		   // Erase instruction
	spiTx((block_addr & 0xFF0000) >> 16);	// MSB of address
	spiTx((block_addr  & 0x00FF00) >> 8);
	spiTx(block_addr & 0x0000FF); // LSB of address
	flashDis();
	delay(30);
	while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)
	if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;
	else status = 0;

	return status;
}


uint8_t flashChipErase(void)
{
	uint8_t status;

	flashWREN();
	delay(30);
	flashEn();
	spiTx(CHIPERASE);		   // Erase instruction
	flashDis();
	while((BIT(nRDY) & flashRDSR()) != 0x00){NOP();} //Wait for nRDY bit to be set (low)
	if ((BIT(EPE) & flashRDSR()) != 0x00) status = 1;
	else status = 0;

	return status;
}

/*********************************************************
 *
 *		Move flash data array from one section to another
 *
 *********************************************************/
uint8_t flashArrayMove(uint32_t target_starting_addr, uint32_t source_starting_addr, uint16_t array_size)
{
	uint8_t status, flash_byte;
	uint16_t i;

	status = 0;
	if (flash4kErase(target_starting_addr) != 0) status = 1;
	for (i = 0; i < array_size; i++)
	{
		flash_byte = flashRead(source_starting_addr + i);
		if (flashWrite(target_starting_addr +i,flash_byte) != 0) status = 1;
	}
	if (flash4kErase(source_starting_addr) != 0) status = 1;

	return status;
}
