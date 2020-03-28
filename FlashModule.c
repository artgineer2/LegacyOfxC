/*
 * FlashModule.c
 *
 *  Created on: Mar 28, 2020
 *      Author: buildrooteclipse
 */


#include "IoLines.h"
#include "Utilities.h"
#include "Constants.h"
#include "CommModule.h"
#include "FlashModule.h"
#include "FXCoreModule.h"
#include "Ofx.h"
#include "UiModule.h"

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





static uint32_t FP;  // flash pointer
static ParamLutFlashPointer_t param_lut_flash_pointer[10];
static uint8_t param_lut_type; // serves as index for Parameter LUT Pointer struct
static uint8_t param_lut_type_num; // number of param LUT types
static StageLutFlashPointer_t stage_lut_flash_pointer[3][10];
static uint8_t stage_lut_num_max; // number of stage LUT types




/******************************************************************************
 *
 *		Relocates Flash Pointer to the start of the model header section for
 *			given model
 *
 *******************************************************************************/
uint32_t flashModelHeaderStart(uint8_t model)
{
	FP = BINFILE_LIMIT + model*MODEL_SIZE;
	/*********************************************************************
	 *
	 *		DO NOT PUT FLASH STATUS CHECKER IN THIS ROUTINE.
	 *		FLASH IS PROGRAMMED TO 255
	 *
	 **********************************************************************/
	return FP;
}


/******************************************************************************
 *
 *		Relocates Flash Pointer to the start of the model stage section for
 *			given model
 *
 *******************************************************************************/
uint32_t flashModelStageStart(uint8_t model, uint8_t stage_index)
{
	FP = BINFILE_LIMIT + model*MODEL_SIZE + HEADER_SIZE + stage_index*STAGE_SIZE;   // model_header + stage_base
	return FP;
}

/******************************************************************************
 *
 *		Relocates Flash Pointer to the start of the scratch header section
 *
 *******************************************************************************/
uint32_t flashScratchHeaderStart(void)
{
	FP = SCRATCH;
	return FP;
}


/******************************************************************************
 *
 *		Relocates Flash Pointer to the start of the scratch stage_index section
 *
 *******************************************************************************/
uint32_t flashScratchStageStart(uint8_t stage_index)
{
	FP = SCRATCH + HEADER_SIZE + stage_index*STAGE_SIZE;   // model_header + stage_base
	return FP;
}

/****************************************************************************
 *
 *		Get_up_tab: returns the fxb word type on the up tab.
 *
 ****************************************************************************/
uint8_t getUpTab(void)
{
	uint8_t tab;

	tab = flashRead(FP);
	if (tab == 255) flash_status = 1;
	return tab & 0x0F;  // up_tab is upper nibble of tab uint8_t
}

/****************************************************************************
 *
 *		Get_down_tab: returns the fxb word type on the down tab.
 *
 ****************************************************************************/
uint8_t getDownTab(void)
{
	uint8_t tab;

	tab = flashRead(FP);
	if (tab == 255) flash_status = 1;
	return (tab&0xF0)>>4;  // down_tab is lower nibble of tab uint8_t
}

uint8_t incFP(void)
{
	uint8_t FP_status; // 1 indicates end of section, 2 indicates error or blank flash

	FP_status = 0; // set to 0, initially

	switch(getUpTab())
	{
	case MODEL_NAME_TAB: // Model Name
		FP = FP + MODEL_NAME_SIZE;
		break;
	case FOOTSWITCH_TAB: // FSW
		FP = FP + FOOTSWITCH_SIZE;
		break;
	case HEADER_COMMENT_TAB: // Header Comments
		FP = FP + HEADER_COMMENT_SIZE;
		break;
	case STAGE_LABEL_TAB: // Stage ABBR/Name
		FP = FP + STAGE_LABEL_SIZE;
		break;
	case EFFECT_LABEL_TAB: // Effect ABBR/Name
		FP = FP + EFFECT_LABEL_SIZE;
		break;
	case PARAMETER_LABEL_TAB: // Parameter ABBR/Name,Parameter Type and Value
		FP = FP + PARAMETER_LABEL_SIZE;
		break;
	case FXCORE_INSTR_TAB: // FXCore Instruction
		FP = FP + FXCORE_INSTR_SIZE;
		break;
	case STAGE_LUT_POINTER_TAB: // FXCore LUT
		FP = FP + STAGE_LUT_POINTER_SIZE;
		break;
	case STAGE_LUT_TAB: // FXCore LUT
		FP = FP + 3 + stage_lut_flash_pointer[getMainStage()][getStageLutNum()].entry_size*4;
		break;
	case PARAM_LUT_POINTER_TAB: // Parameter LUT Pointer
		FP = FP + PARAM_LUT_POINTER_SIZE;
		break;
	case PARAM_LUT_TAB: // Parameter LUT
		FP = FP + param_lut_flash_pointer[param_lut_type].entry_size;
		break;
	case RAM2_CONST_TAB:  // RAM2 Constant
		FP = FP + RAM2_CONST_SIZE;
		break;
	case END_TAB:// End
		FP_status = 1;  // end of section, will not go any further
		break;
	default:// bad tab, go to next flash uint8_t
		FP = FP + 1;
		FP_status = 2;
		break;
	}

	return FP_status;
}

uint8_t decFP(void)
{
	uint8_t FP_status; // 1 indicates end of section

	FP_status = 0; // set to 0, initially

	switch(getDownTab())
	{
	case 0: // Beginning of section.  Will not go any further
		FP_status = 1;
		break;
	case MODEL_NAME_TAB: // Model Name
		FP = FP - MODEL_NAME_SIZE;
		break;
	case FOOTSWITCH_TAB: // FSW
		FP = FP - FOOTSWITCH_SIZE;
		break;
	case HEADER_COMMENT_TAB: // Header Comments
		FP = FP - HEADER_COMMENT_SIZE;
		break;
	case STAGE_LABEL_TAB: // Stage Name/ABBR
		FP = FP - STAGE_LABEL_SIZE;
		break;
	case EFFECT_LABEL_TAB: // Effect Name/ABBR
		FP = FP - EFFECT_LABEL_SIZE;
		break;
	case PARAMETER_LABEL_TAB: // Parameter Name/ABBR and value
		FP = FP - PARAMETER_LABEL_SIZE;
		break;
	case FXCORE_INSTR_TAB: // FXCore Instruction
		FP = FP - FXCORE_INSTR_SIZE;
		break;
	case STAGE_LUT_POINTER_TAB: // FXCore LUT.  FXCore address is two bytes (high,low)
		FP = FP - STAGE_LUT_POINTER_SIZE;
		break;
	case STAGE_LUT_TAB: // FXCore LUT.  FXCore address is two bytes (high,low)
		FP = FP - 3 + stage_lut_flash_pointer[getMainStage()][getStageLutNum()].entry_size*4;
		break;
	case PARAM_LUT_POINTER_TAB: // Parameter LUT Pointer
		FP = FP - PARAM_LUT_POINTER_SIZE;
		break;
	case PARAM_LUT_TAB: // Parameter LUT
		FP = FP - param_lut_flash_pointer[param_lut_type].entry_size;
		break;
	case RAM2_CONST_TAB:  // RAM2 Constant
		FP = FP - RAM2_CONST_SIZE;
		break;
	case END_TAB:// beginning of section.  Will not go any further
		FP = FP - END_SIZE;
		break;
	default:// bad tab, go to next flash uint8_t
		FP = FP - 1;
		FP_status = 2;
		break;
	}

	return FP_status;
}


void paramLutFlashPointerInit(uint8_t model)
{
	LongBytes_t flash_addr;

	FP = BINFILE_LIMIT + model*MODEL_SIZE + HEADER_SIZE + 3*STAGE_SIZE;   	// put FP at start of ParamLUTPointer section
	// (model_header + 3 x stages)
	param_lut_type = 0;
	do
	{
		flash_addr.high_byte = flashRead(FP + 1);  // read high addr uint8_t in LUT pointer section
		flash_addr.mid_byte = flashRead(FP + 2);  // read mid addr uint8_t in LUT pointer section
		flash_addr.low_byte = flashRead(FP + 3);  // read low addr uint8_t in LUT pointer section

		param_lut_flash_pointer[param_lut_type].flash_base_addr = flash_addr.long_var;
		param_lut_flash_pointer[param_lut_type].entry_size = flashRead(FP + 4);  // read LUT entry size in LUT pointer section
		param_lut_type++;

	}while (incFP() == 0);
	param_lut_type_num = param_lut_type;
}


void stageLutFlashPointerInit(uint8_t model, uint8_t stage_index)
{
	uint8_t up_tab = 0;
	LongBytes_t flash_addr;

	flash_addr.long_var = 0;
	FP = fxcoreSect2FlashAddr(model, 2*(stage_index+1), 0);

	setStageLutNum(0);
	stage_lut_num_max = 0;

	clearBuffer1();
	clearBuffer2();
	do
	{
		up_tab = getUpTab();
		flash_addr.high_byte = flashRead(FP + 1);  // read high addr uint8_t in LUT pointer section
		flash_addr.mid_byte = flashRead(FP + 2);  // read mid addr uint8_t in LUT pointer section
		flash_addr.low_byte = flashRead(FP + 3);  // read low addr uint8_t in LUT pointer section

		stage_lut_flash_pointer[stage_index][getStageLutNum()].flash_base_addr = flash_addr.long_var;
		stage_lut_flash_pointer[stage_index][getStageLutNum()].stage_start_addr = flashRead(FP + 4);  // offset
		stage_lut_flash_pointer[stage_index][getStageLutNum()].num_entries = flashRead(FP + 5);
		stage_lut_flash_pointer[stage_index][getStageLutNum()].entry_size = flashRead(FP + 6);
		buffer1ByteCat(up_tab);
		buffer1Cat(" ");

		if (up_tab == 8)
		{
			incFP();
			incStageLutNum();
		}
	}while (up_tab == 8);
	stage_lut_num_max = getStageLutNum();
}



void setFP(uint32_t flashPtr)
{
	FP = flashPtr;
}

uint32_t getFP(void)
{
	return FP;
}

ParamLutFlashPointer_t getParamLutFlashPtr(uint8_t index)
{
	return param_lut_flash_pointer[index];
}
void setParamLutFlashPtr(uint8_t index, ParamLutFlashPointer_t paramLutFlastPtr)
{
	param_lut_flash_pointer[index] = paramLutFlastPtr;
}

uint8_t getParamLutType()
{
	return param_lut_type;
}
void setParamLutType(uint8_t paramLutType)
{
	param_lut_type = paramLutType;
}

uint8_t getParamLutTypeNum()
{
	return param_lut_type_num;
}
void setParamLutTypeNum(uint8_t paramLutTypeNum)
{
	param_lut_type_num = paramLutTypeNum;
}

StageLutFlashPointer_t getStageLutFlashPtr(uint8_t stage_index, uint8_t stage_lut_index)
{
	return stage_lut_flash_pointer[stage_index][stage_lut_index];
}
void setStageLutFlashPtr(uint8_t stage_index, uint8_t stage_lut_index,
		StageLutFlashPointer_t stageLutFlashPtr)
{
	stage_lut_flash_pointer[stage_index][stage_lut_index] = stageLutFlashPtr;
}

uint8_t getStageLutNumMax()
{
	return stage_lut_num_max;
}
void setStageLutNumMax(uint8_t stageLutNumMax)
{
	stage_lut_num_max = stageLutNumMax;
}

