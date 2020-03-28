/*
 * FXCoreModule.c
 *
 *  Created on: Mar 28, 2020
 *      Author: buildrooteclipse
 */


#include "IoLines.h"
#include "Utilities.h"
#include "CustomTypes.h"
#include "Constants.h"
#include "Macros.h"
#include "Utilities.h"
#include "CommModule.h"
#include "FlashModule.h"
#include "FXCoreModule.h"
#include "UiModule.h"


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


uint8_t flash_status; //leave as global for speed

static uint32_t model_name_addr;
static uint32_t fsw_addr[2];
static Parameter_t *param;
static Effect_t *fx;
static Stage_t stage[3];
static uint8_t stage_lut_num; // serves as index for FXCore LUT Pointer struct

/************************************************************
 *
 *				FXCore Stage Save: FXCore -> Flash
 *				(update to include LUTs)
 *
 ************************************************************/
uint8_t fxcoreStageSave(uint8_t model_index, uint8_t stage_index)
{
	uint32_t FXCore_instr_addr;
	uint8_t intraword_addr;
	uint8_t flash_byte;
	uint8_t up_tab;
	uint8_t fx_index,param_index;
	FXCoreWord_t save_instr;
	uint8_t status;

	flashModelStageStart(model_index,stage_index);  // Set FP to stage to be saved
	fx_index = 0;
	param_index = 0;
	flash_status = 0;
	clearTxBuffer();
	clearBuffer1();
	if ((flash4kErase(getFP() + (5-model_index)*MODEL_SIZE) == 0)) // erasure of SCRATCH section successful
	{
		FXCore_instr_addr = 0;
		do  // Save FXCore Instructions
		{
			up_tab = getUpTab();

			switch(up_tab)
			{
			case STAGE_LABEL_TAB: // Stage ABBR/name
				for(intraword_addr = 0; intraword_addr < STAGE_LABEL_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}

				break;
			case EFFECT_LABEL_TAB: // Effect ABBR/name
				for(intraword_addr = 0; intraword_addr < EFFECT_LABEL_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				fx_index++;
				param_index = 0;
				break;
			case PARAMETER_LABEL_TAB: // Parameter ABBR/name, type and value (18 uint8_t word)
				for(intraword_addr = 0; intraword_addr < 18; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				param_index++;
				break;
			case FXCORE_INSTR_TAB: // FXCore Instr
				save_instr = fxcoreRead(stage_index+1, FXCore_instr_addr);
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE, flashRead(getFP())) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+1, flashRead(getFP()+1)) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+2, save_instr.main) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+3, save_instr.data1) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+4, save_instr.data2) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+5, save_instr.data3) != 0) flash_status = 1;
				FXCore_instr_addr++;
				break;
			case RAM2_CONST_TAB: // FXCore Instr
				save_instr = fxcoreRead(stage_index+1, FXCore_instr_addr + 256); // read from RAM2
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE, flashRead(getFP())) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+1, flashRead(getFP()+1)) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+2, save_instr.main) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+3, save_instr.data1) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+4, save_instr.data2) != 0) flash_status = 1;
				if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE+5, save_instr.data3) != 0) flash_status = 1;
				FXCore_instr_addr++;
				break;
			case END_TAB: // End
				for(intraword_addr = 0; intraword_addr < END_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				break;
			default:
				break;
			}
			if (up_tab != 14) incFP();

		}while(up_tab != 14);


		stage_lut_num = 0;
		do  // Save FXCore LUT
		{
			up_tab = getUpTab();

			switch(up_tab)
			{
			case STAGE_LUT_POINTER_TAB: // FXCore LUT Pointer
				for(intraword_addr = 0; intraword_addr < PARAM_LUT_POINTER_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				stage_lut_num++; // increment FXCore_lut_index
				break;
			case STAGE_LUT_TAB: // FXCore LUT
				for(intraword_addr = 0; intraword_addr < getStageLutFlashPtr(stage_index,stage_lut_num).entry_size; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				break;
			case END_TAB: // End
				for(intraword_addr = 0; intraword_addr < END_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				break;
			default:
				break;
			}
			if (up_tab != 14) incFP();

		}while(up_tab != 14);

		status = flash_status;
	}
	else // erasure failed
	{
		Display("SCRATCH Erase","Error");
		delay(100000);
		status = 1;
	}

	flashModelStageStart(model_index,stage_index);  // Set FP to original stage
	fx_index = 255;
	param_index = 255;
	clearBuffer1();
	if (flash4kErase(getFP()) == 0)  // clear original stage block before re-programming it
	{
		flashScratchStageStart(stage_index);  // Set FP to stage in next model

		do
		{
			up_tab = getUpTab(); // get up_tab in SCRATCH stage

			switch(up_tab)
			{
			case STAGE_LABEL_TAB: // Stage ABBR/name
				for(intraword_addr = 0; intraword_addr < STAGE_LABEL_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if (flashWrite(getFP() - (5-model_index)*MODEL_SIZE + intraword_addr,flash_byte) != 0) flash_status = 1;
				}
				break;
			case EFFECT_LABEL_TAB: // Effect ABBR/name
				for(intraword_addr = 0; intraword_addr < EFFECT_LABEL_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if (flashWrite(getFP() - (5-model_index)*MODEL_SIZE + intraword_addr,flash_byte) != 0) flash_status = 1;
				}
				fx_index++;
				param_index = 255;
				break;
			case PARAMETER_LABEL_TAB: // Parameter ABBR/name and value
				for(intraword_addr = 0; intraword_addr < (PARAMETER_LABEL_SIZE-1); intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if (flashWrite(getFP() - (5-model_index)*MODEL_SIZE + intraword_addr,flash_byte) != 0) flash_status = 1;
				}
				param_index++;
				if (flashWrite(getFP() - (5-model_index)*MODEL_SIZE + (PARAMETER_LABEL_SIZE-1), stage[stage_index].fx[fx_index].param[param_index].FXCore_value) != 0) flash_status = 1;
				break;
			case FXCORE_INSTR_TAB: // FXCore Instruction
				for(intraword_addr = 0; intraword_addr < FXCORE_INSTR_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if (flashWrite(getFP() - (5-model_index)*MODEL_SIZE + intraword_addr,flash_byte) != 0) flash_status = 1;
				}
				break;
			case RAM2_CONST_TAB: // FXCore Instruction
				for(intraword_addr = 0; intraword_addr < RAM2_CONST_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if (flashWrite(getFP() - (5-model_index)*MODEL_SIZE + intraword_addr,flash_byte) != 0) flash_status = 1;
				}
				break;
			case END_TAB: // End
				for(intraword_addr = 0; intraword_addr < END_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if (flashWrite(getFP() - (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				break;
			default:
				break;
			}
			if (up_tab != 14) incFP();

		}while(up_tab != 14); // stop after end word

		stage_lut_num = 0;
		do  // Save FXCore LUT
		{
			up_tab = getUpTab();

			switch(up_tab)
			{
			case STAGE_LUT_POINTER_TAB: // FXCore LUT Pointer
				for(intraword_addr = 0; intraword_addr < PARAM_LUT_POINTER_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				stage_lut_num++; // increment FXCore_lut_index
				break;
			case STAGE_LUT_TAB: // FXCore LUT
				for(intraword_addr = 0; intraword_addr < getStageLutFlashPtr(stage_index,stage_lut_num).entry_size; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				break;
			case END_TAB: // End
				for(intraword_addr = 0; intraword_addr < END_SIZE; intraword_addr++)
				{
					flash_byte = flashRead(getFP() + intraword_addr);
					if(flashWrite(getFP() + (5-model_index)*MODEL_SIZE + intraword_addr, flash_byte) != 0) flash_status = 1;
				}
				break;
			default:
				break;
			}
			if (up_tab != 14) incFP();

		}while(up_tab != 14);


		status = flash_status;
	}
	else
	{
		Display("Stage Erase","Error");
		delay(100000);
		status = 30;
	}

	return status;
}


/*************************************************************
 *
 *		Get address array for main UI.
 *		Use when changing models
 *
 *************************************************************/
uint8_t fxcoreHeaderLoad(uint8_t model) // first 2 screens for each model
{
	uint8_t status;

	flash_status = 0;
	flashModelHeaderStart(model);
	if(flash_status == 0)
	{
		model_name_addr = getFP();
		incFP();
		fsw_addr[0] = getFP();
		incFP();
		fsw_addr[1] = getFP();
		stage[0].flash_label_addr = flashModelStageStart(model,0);
		stage[1].flash_label_addr = flashModelStageStart(model,1);
		stage[2].flash_label_addr = flashModelStageStart(model,2);

		status = 0;
	}
	else
	{
		status = 10;
	}

	return status;
}

uint8_t fxcoreStageLoad(uint8_t model_index, uint8_t stage_index)
{
	uint8_t up_tab;
	uint16_t i;
	uint8_t effect_index, param_index;
	uint8_t status;
	uint8_t instr_addr;
	FXCoreWord_t load_instr;
	FXCoreWord_t lut_data;
	uint16_t LUT_addr;
	flash_status = 0;
	up_tab = 0;


	flashModelStageStart(model_index,stage_index);

	if(flash_status == 0)
	{
		stage[stage_index].num_fx = 0;

		/********************* Allocate memory for effect structs *********************/

		setFP(stage[stage_index].flash_label_addr);

		while((incFP() == 0) && (flash_status == 0)) // next tab is good and flash is good
		{
			up_tab = getUpTab();
			if(up_tab == EFFECT_LABEL_TAB) // effect ABBR/name
			{
				stage[stage_index].num_fx++;
			}

		}

		stage[stage_index].fx = calloc(stage[stage_index].num_fx,sizeof(*fx));  // effect is a struct (only data types and structs can be used in sizeof()

		/********************* Allocate memory for parameter structs********/

		setFP(stage[stage_index].flash_label_addr);

		effect_index = 255;
		while((incFP() == 0) && (flash_status == 0)) // next tab is good and flash is good
		{
			up_tab = getUpTab();
			if(up_tab == EFFECT_LABEL_TAB) // effect ABBR/name
			{
				effect_index++;
				stage[stage_index].fx[effect_index].num_param = 0;
			}
			else if(up_tab == PARAMETER_LABEL_TAB) // parameter ABBR/name and value
			{
				stage[stage_index].fx[effect_index].num_param++;
			}
		}
		for(i = 0; i < stage[stage_index].num_fx; i++)
		{
			stage[stage_index].fx[i].param = calloc(stage[stage_index].fx[i].num_param,sizeof(*param));
		}

		/********************* Create Array **************************/

		setFP(stage[stage_index].flash_label_addr); // set flash pointer to beginning of stage
		effect_index = 255;
		param_index = 255;
		i = 0;

		fxcoreHoldOn();
		while((incFP() == 0) && (flash_status == 0)) // next tab is good and flash is good
		{
			up_tab = getUpTab();
			if (up_tab == EFFECT_LABEL_TAB) // effect ABBR/name addr
			{
				effect_index++;
				param_index = 255;
				stage[stage_index].fx[effect_index].flash_label_addr = getFP();
				stage[stage_index].fx[effect_index].FXCore_addr = flashRead(getFP()+1);
			}
			else if (up_tab == PARAMETER_LABEL_TAB) // parameter ABBR/name addr and value
			{
				param_index++;
				stage[stage_index].fx[effect_index].param[param_index].flash_label_addr = getFP();
				stage[stage_index].fx[effect_index].param[param_index].FXCore_addr = flashRead(getFP()+1);
				stage[stage_index].fx[effect_index].param[param_index].param_type = flashRead(getFP()+16);
				stage[stage_index].fx[effect_index].param[param_index].FXCore_value = flashRead(getFP()+17);
			}
			else if (up_tab == FXCORE_INSTR_TAB)
			{
				//**************************FXCore instr**************************

				instr_addr = flashRead(getFP()+1);
				load_instr.main = flashRead(getFP()+2);
				load_instr.data1 = flashRead(getFP()+3);
				load_instr.data2 = flashRead(getFP()+4);
				load_instr.data3 = flashRead(getFP()+5);

				fxcoreWrite(stage_index+1, instr_addr, load_instr);
			}
			//****************************************************************
			else if (up_tab == RAM2_CONST_TAB)
			{
				//**************************FXCore RAM2 Constant**************************

				instr_addr = flashRead(getFP()+1);
				load_instr.main = flashRead(getFP()+2);
				load_instr.data1 = flashRead(getFP()+3);
				load_instr.data2 = flashRead(getFP()+4);
				load_instr.data3 = flashRead(getFP()+5);

				fxcoreWrite(stage_index+1, instr_addr + 256, load_instr); // Write to RAM2

				//****************************************************************
			}
			else if (up_tab == END_TAB)
			{
				//**************************FXCore end**************************

				instr_addr = flashRead(getFP()+1);
				load_instr.main = flashRead(getFP()+2);
				load_instr.data1 = flashRead(getFP()+3);
				load_instr.data2 = flashRead(getFP()+4);
				load_instr.data3 = flashRead(getFP()+5);

				fxcoreWrite(stage_index+1, instr_addr, load_instr);

				/****************************************************************/
			}
		} // end of FXCore code section
		i = 0;
		for(stage_lut_num = 0; stage_lut_num < getStageLutNumMax(); stage_lut_num++) // enter the "stage_lut_num_max" number of stage LUTs into the FXCore
		{
			setFP(getStageLutFlashPtr(stage_index,stage_lut_num).flash_base_addr); // set flash pointer to beginning of stage LUT in flash
			do
			{
				clearBuffer1();
				up_tab = getUpTab();
				//**************************FXCore LUT**************************
				LUT_addr = 512 + 256*flashRead(getFP()+1) + flashRead(getFP()+2);  // LUT section base addr plus stage_base_addr
				lut_data.main = flashRead(getFP()+3);
				lut_data.data1 = flashRead(getFP()+4);
				lut_data.data2 = flashRead(getFP()+5);
				lut_data.data3 = flashRead(getFP()+6);

				fxcoreWrite(stage_index+1, LUT_addr, lut_data);
				//****************************************************************
				//****** End word will not be entered into FXCore stage LUT ******

				if (up_tab == 9) incFP(); // if word is LUT entry word, go to the next tab
			}while(getUpTab() == 9);  // loop until the end word appears
		} // end of FXCore LUT section
		fxcoreHoldOff();
		delay(200000);
		/***************************** Print effect and parameter numbers **********************/
		txBufferByteCat(getStageEffectsCount(stage_index));
		txBufferCat(":");
		/***************************************************************************************/

		status = 0;
	}
	else
	{
		status = 20;
	}

	return status;
}


uint16_t fxcoreParameterUpdate(uint8_t stage_index, uint8_t effect_index, uint8_t parameter_index, uint8_t parameter_type, uint8_t in_value)
{
	// in_value is address of entry within LUT

	uint32_t lut_flash_base;
	uint32_t entry_data_addr; // address of entry data word in flash
	uint8_t fxcore_base_addr; // main FXCore parameter addr, reference for fxcore_rel_addrs
	uint16_t UI_value;
	FXCoreWord_t old_instr;
	FXCoreWord_t new_instr;
	FXCoreWord_t lut_data;
	uint8_t i;
	uint8_t num_data; // number of data words in entry
	uint32_t long_temp;


	lut_flash_base = getParamLutFlashPtr(parameter_type).flash_base_addr;  // base address of parameter LUT in serial flash
	fxcore_base_addr = stage[stage_index].fx[effect_index].param[parameter_index].FXCore_addr; // base address of parameter in FXCore
	num_data = (getParamLutFlashPtr(parameter_type).entry_size - 3)/4;

	for(i = 0; i < num_data; i++)
	{
		entry_data_addr = lut_flash_base // base address of LUT in flash
				+ in_value*getParamLutFlashPtr(parameter_type).entry_size // base address of entry in LUT
				+ 3+i*4; // LUT entry data address relative to base address of entry
		lut_data.main = flashRead(entry_data_addr); // FXCore addr of data relative to fxcore_base

		long_temp = fxcore_base_addr + lut_data.main;
		old_instr = fxcoreRead(stage_index+1,long_temp);  // read register contents

		new_instr.main = old_instr.main;
		new_instr.data1 = flashRead(entry_data_addr + 1); // data1 within LUT entry
		new_instr.data2 = flashRead(entry_data_addr + 2); // data2 within LUT entry
		new_instr.data3 = flashRead(entry_data_addr + 3); // data3 within LUT entry

		fxcoreWrite(stage_index+1,long_temp,new_instr);  // write register with new data

	}
	UI_value = 0;
	UI_value += 256*flashRead(getParamLutFlashPtr(parameter_type).flash_base_addr + in_value*getParamLutFlashPtr(parameter_type).entry_size+1);
	UI_value += flashRead(getParamLutFlashPtr(parameter_type).flash_base_addr + in_value*getParamLutFlashPtr(parameter_type).entry_size+2);

	return UI_value;
}



void fxcoreThruStage(uint8_t stage_index)
{
	FXCoreWord_t blank_stage[3];

	blank_stage[0].main = 72;
	blank_stage[0].data1 = 0;
	blank_stage[0].data2 = 0;
	blank_stage[0].data3 = 0;

	blank_stage[1].main = 96;
	blank_stage[1].data1 = 0;
	blank_stage[1].data2 = 0;
	blank_stage[1].data3 = 0;

	blank_stage[2].main = 255;
	blank_stage[2].data1 = 0;
	blank_stage[2].data2 = 0;
	blank_stage[2].data3 = 0;


	fxcoreWrite(stage_index+1,0,blank_stage[0]);
	fxcoreWrite(stage_index+1,1,blank_stage[1]);
	fxcoreWrite(stage_index+1,2,blank_stage[2]);

}


/************************************************************
 *					NEEDED???????
 *
 *					Stage Instr Erase
 *
 ************************************************************/
void fxcoreInstrErase(uint8_t stage)
{
	uint16_t i;
	FXCoreWord_t erase_instr;

	erase_instr.main = 0;
	erase_instr.data1 = 0;
	erase_instr.data2 = 0;
	erase_instr.data3 = 0;

	for(i = 0; i < 512; i++)
	{
		fxcoreWrite(stage+1,i, erase_instr);
	}
}

/************************************************************
 *					NEEDED???????
 *
 *					Stage Erase
 *
 ************************************************************/
void fxcoreLutErase(uint8_t stage)
{
	uint16_t i;
	FXCoreWord_t erase_LUT;

	erase_LUT.main = 0;
	erase_LUT.data1 = 0;
	erase_LUT.data2 = 0;
	erase_LUT.data3 = 0;

	for(i = 512; i < 1024; i++)
	{
		fxcoreWrite(stage+1,i, erase_LUT);
	}
}


void setModelNameAddr(uint32_t addr)
{
	model_name_addr = addr;
}

uint32_t getModelNameAddr()
{
	return model_name_addr;
}

void setFswAddr(uint8_t index, uint32_t addr)
{
	fsw_addr[index] = addr;
}

uint32_t getFswAddr(uint8_t index)
{
	return fsw_addr[index];
}

void setStage(uint8_t index, Stage_t _stage)
{
	stage[index] = _stage;
}

Stage_t getStage(uint8_t index)
{
	return stage[index];
}

uint8_t getStageEffectsCount(uint8_t stage_index)
{
	return stage[stage_index].num_fx;
}
Effect_t *getStageEffects(uint8_t stage_index)
{
	return stage[stage_index].fx;
}
Effect_t getStageEffect(uint8_t stage_index, uint8_t effect_index)
{
	return stage[stage_index].fx[effect_index];
}
Parameter_t *getStageEffectParams(uint8_t stage_index, uint8_t effect_index)
{
	return stage[stage_index].fx[effect_index].param;
}

Parameter_t getStageEffectParam(uint8_t stage_index, uint8_t effect_index, uint8_t param_index)
{
	return stage[stage_index].fx[effect_index].param[param_index];
}


extern uint8_t *getStageEffectParamValuePtr(uint8_t stage_index, uint8_t effect_index, uint8_t param_index)
{
	return &(stage[stage_index].fx[effect_index].param[param_index].FXCore_value);
}

void freeStageEffectParams(uint8_t stage_index, uint8_t effect_index)
{
	free(stage[stage_index].fx[effect_index].param);
	stage[stage_index].fx[effect_index].flash_label_addr = 0;
}

void freeStageEffects(uint8_t stage_index)
{
	free(stage[stage_index].fx);
	stage[stage_index].flash_label_addr = 0;
}


uint8_t getStageEffectParamCount(uint8_t stage_index, uint8_t effect_index)
{
	return stage[stage_index].fx[effect_index].num_param;
}


void setStageLutNum(uint8_t lutIndex)
{
	stage_lut_num = lutIndex;
}

uint8_t getStageLutNum()
{
	return stage_lut_num;
}

void incStageLutNum(void)
{
	stage_lut_num++;
}

void decStageLutNum(void)
{
	stage_lut_num--;
}
