//#define uint8_t   unsigned char

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Macros.h"
#include "TestParameterLut.h"
#include "Utilities.h"
#include "Structs.h"


uint8_t FPGA_section_status;
uint8_t main_state;
uint8_t main_model;
uint16_t main_fxcore_address;
uint32_t main_flash_address;
uint8_t main_stage;
uint8_t main_effect;
uint8_t main_param;
uint8_t Flash_header_status;
uint8_t Flash_stage_status[3];
uint8_t state_change;
uint8_t model_change;
uint8_t stage_change;
uint8_t effect_change;
uint8_t param_change;
uint8_t value_change;


#define DBG_MAIN 0
/******************************************************************
 *
 *		Peripheral driver files
 *
 ******************************************************************/
#include "IoLines.h"
#include "UiDrivers.h"
#include "FlashDrivers.h"
#include "FXCoreDrivers.h"
#include "CodecDrivers.h"

/*******************************************************************
 *
 *		Peripheral operation files
 *
 *******************************************************************/
#include "UiOp.h"
#include "FlashOp.h"
#include "FXCoreOp.h"

#include "ChipInit.h"
#include "InterruptHandlers.h"


int main(void)
{
	char abbr_buffer[5]; // use with Flash_array_read() to write to string buffers
	char name_buffer[11];
	char tempBuffer[17];
	uint8_t LCD_change;
	uint8_t switches,prev_switches;
	uint8_t ModelMax = 5;
	uint8_t StageMax = 3;
	uint16_t i = 0;


	SP = 0x10FF;	// Initialize stack-pointer

	configChip();
	lcdInit();
	flashInit();
	paramLutFlashPointerInit(0);
	stageLutFlashPointerInit(0,0);
	codecRst();
	configCodec();


	setTxCount(0);
	setRxCount(0);
	count_up = 0;
	count_down = 0;
	Flash_header_status = 1;
	Flash_stage_status[0] = 1;
	Flash_stage_status[1] = 1;
	Flash_stage_status[2] = 1;


	main_state = 0;
	state_change=stage_change=effect_change=value_change=param_change=0;
	main_model=main_stage=main_effect=main_param=0;

	model_change = 1;
	prev_switches = 0;
	LCD_change = 0;

	setRxDone(0);

	Display("      OFX     ","   WarpDrive   ");

	FPGA_section_status = fpgaConfig(); // load bitstream into FPGA
	delay(50);
	fpgaRstEn();
	delay(50);
	fpgaRstDis();
	delay(5000);


	while(1)
	{
		if(FPGA_section_status == 0) // FPGA configured successfully
		{

			/***************************************************************************
			 *
			 *		User Interface Push Button Code
			 *
			 ***************************************************************************/

			switches = readSwitch();
			count_dir = rotaryDir();
			if ((switches != prev_switches) && (switches != 0))
			{
				switch(switches)  // read push buttons
				{
				//*************** EDIT ****************************
				case 1: if (main_state < 9)
				{
					if((main_state == 0) && (Flash_header_status == 0))
					{
						main_state++;
						state_change = 1;
					}
					else if((main_state == 1) && (Flash_stage_status[main_stage] == 0))
					{
						main_state++;
						state_change = 1;
					}
					else
					{
						main_state++;
						state_change = 1;
					}
					LCD_change = 1;
				}
				break;
				//*************** LEFT ****************************
				case 2: switch(main_state)
				{
				case 0:
					if (value_change == 1)
					{
						Display("Save model","changes ?");
						while((readSwitch() != 1) && (readSwitch() != 8)) {NOP();}
						if (readSwitch() == 1)
						{
							Display("Saving *",NULL);
							if(fxcoreStageSave(main_model,0) != 0)
							{
								NOP();
							}
							Display("Saving **",NULL);
							if(fxcoreStageSave(main_model,1) != 0)
							{
								NOP();
							}
							Display("Saving ***",NULL);
							if(fxcoreStageSave(main_model,2) != 0)
							{
								NOP();
							}
						}
						value_change = 0;
					}

					if(Flash_stage_status[0] == 0) uiStageClear(0);  // make sure MCU RAM was allocated for stage
					if(Flash_stage_status[1] == 0) uiStageClear(1);  // before freeing allocated RAM.  Otherwise,
					if(Flash_stage_status[2] == 0) uiStageClear(2);  // system will lock up.

					if (main_model == 0)
					{
						main_model = ModelMax-1;
					}
					else
					{
						main_model--;
					}
					model_change = 1;
					break;
				case 1: if (main_stage <= 0)
				{
					main_stage = StageMax-1;
				}
				else
				{
					main_stage--;
				}
				stage_change = 1;
				break;
				case 2: if (main_effect <= 0)
				{
					main_effect = getStageEffectsCount(main_stage)-1;
				}
				else
				{
					main_effect--;
				}
				effect_change = 1;
				break;
				case 3: if (main_param <= 0)
				{
					main_param = getStageEffectParamCount(main_stage,main_effect)-1;
				}
				else
				{
					main_param--;
				}
				param_change = 1;
				break;
				default: break;
				}
				LCD_change = 1;
				break;
				//*************** RIGHT ****************************
				case 4: switch(main_state)
				{
				case 0:
					if (value_change == 1)
					{
						Display("Save model","changes ?");
						while((readSwitch() != 1) && (readSwitch() != 8)) {NOP();}
						if (readSwitch() == 1)
						{
							Display("Saving *",NULL);
							if(fxcoreStageSave(main_model,0) != 0)
							{
								NOP();
							}
							Display("Saving **",NULL);
							if(fxcoreStageSave(main_model,1) != 0)
							{
								NOP();
							}
							Display("Saving ***",NULL);
							if(fxcoreStageSave(main_model,2) != 0)
							{
								NOP();
							}
						}
						value_change = 0;
					}

					if(Flash_stage_status[0] == 0) uiStageClear(0);  // make sure MCU RAM was allocated for stage
					if(Flash_stage_status[1] == 0) uiStageClear(1);  // before freeing allocated RAM.  Otherwise,
					if(Flash_stage_status[2] == 0) uiStageClear(2);  // system will lock up.

					if (main_model >= ModelMax-1)
					{
						main_model = 0;
					}
					else
					{
						main_model++;
					}
					model_change = 1;
					break;
				case 1: if (main_stage >= StageMax-1)
				{
					main_stage = 0;
				}
				else
				{
					main_stage++;
				}
				stage_change = 1;
				break;
				case 2: if (main_effect >= getStageEffectsCount(main_stage)-1)
				{
					main_effect = 0;
				}
				else
				{
					main_effect++;
				}
				effect_change = 1;
				break;
				case 3: if (main_param >= getStageEffectParamCount(main_stage,main_effect)-1)
				{
					main_param = 0;
				}
				else
				{
					main_param++;
				}
				param_change = 1;
				break;
				default: break;
				}
				LCD_change = 1;
				break;
				//*************** EXIT ****************************
				case 8: if (main_state > 0)
				{
					main_state--;
					state_change = 1;
					LCD_change = 1;
				}
				break;
				default: ;
				}
			}

			prev_switches = switches;

			/***************************************************************************
			 *
			 *			State Code
			 *
			 ***************************************************************************/
			switch(main_state)
			{
			case 0:
				if ((model_change == 1) || (state_change == 1) || (getRxDone() == 1))
				{
					//********************** MODEL CODE *********************
					if(model_change == 1)
					{
						Flash_header_status = fxcoreHeaderLoad(main_model);
						clearTxBuffer();
						for (i = 0; i < 3; i++)
						{
							fxcoreInstrErase(i);
							fxcoreLutErase(i);
							stageLutFlashPointerInit(main_model,i);
							Flash_stage_status[i] = fxcoreStageLoad(main_model,i);
							if(Flash_stage_status[i] != 0)
							{
								clearBuffer1();
								buffer1ByteCat(i);
								buffer1Cat(" ");
								buffer1ByteCat(Flash_stage_status[i]);
								Display(getBuffer1(),NULL);
								delay(300000);
							}
						}
						paramLutFlashPointerInit(main_model);
					}

					clearBuffer1();
					clearBuffer2();
					clearIndBuffer();
					buffer1ByteCat(main_model);
					buffer1Cat(":");
					if(Flash_header_status == 0)
					{
						flashArrayRead(getModelNameAddr()+2,(uint8_t *)name_buffer,10);
					}
					else
					{
						strcpy(name_buffer,"          ");
					}
					buffer1Cat(name_buffer);

					for(i = 0; i < 2; i++)
					{
						if(Flash_header_status == 0)
						{
							flashArrayRead(getFswAddr(i)+2,(uint8_t *)abbr_buffer,4);
						}
						else								// stage[i] is blank
						{
							setAbbrBuffer("    ");
						}
						indBufCat(" ");
						indBufCat(abbr_buffer);
					}
					clearBuffer(tempBuffer,16);
					capInd(tempBuffer,getIndBuf(),0);
					setBuffer2(tempBuffer);
					model_change = 0;
				}
				break;
			case 1:
				if ((stage_change == 1) || (state_change == 1) || (getRxDone() == 1))
				{
					if (prev_switches == 1) main_stage = 0;
					//********************** MODEL CODE #2 ********************
					clearBuffer1();
					flashArrayRead(getModelNameAddr()+2,(uint8_t *)name_buffer,10);
					buffer1Cat(name_buffer);
					clearIndBuffer();
					for(i = 0; i < StageMax; i++)
					{
						if (Flash_stage_status[i] == 0) // stage[i] is active
						{
							flashArrayRead(getStage(i).flash_label_addr+2,(uint8_t *)abbr_buffer,4);
						}
						else							// stage[i] is blank
						{
							strcpy(abbr_buffer,"--->");
						}
						indBufCat(" ");
						indBufCat(abbr_buffer);
					}
					clearBuffer(tempBuffer,16);
					capInd(tempBuffer,getIndBuf(),main_stage);
					setBuffer2(tempBuffer);
					stage_change = 0;
				}
				break;
			case 2:
				if ((effect_change == 1) || (state_change == 1))
				{
					if (prev_switches == 1) main_effect = 0;
					//********************** STAGE CODE *********************
					clearBuffer1();
					clearBuffer2();
					flashArrayRead(getStage(main_stage).flash_label_addr+6,(uint8_t *)name_buffer,10);
					buffer1Cat( name_buffer);

					uiIndBufLoad(2,main_effect,getStage(main_stage).num_fx);
					clearBuffer(tempBuffer,16);
					capInd(tempBuffer,getIndBuf(),main_effect);
					setBuffer2(tempBuffer);
					effect_change = 0;
				}
				break;
			case 3:
				if ((param_change == 1) || (state_change == 1))
				{
					if (prev_switches == 1) main_param = 0;
					//********************** EFFECT CODE *********************
					clearBuffer1();
					flashArrayRead(getStageEffect(main_stage,main_effect).flash_label_addr+6,(uint8_t *)name_buffer,10);
					buffer1Cat(name_buffer);

					uiIndBufLoad(3,main_param,getStageEffectParamCount(main_stage,main_effect));
					clearBuffer(tempBuffer,16);
					capInd(tempBuffer,getIndBuf(),main_param);
					setBuffer2(tempBuffer);
					param_change = 0;
				}
				break;
			case 4:
				if (count_dir || (state_change == 1))
				{
					Parameter_t tempParam = getStageEffectParam(main_stage,main_effect,main_param);
					rotCount(count_dir, &tempParam.FXCore_value);
					value_change = 1;
					//********************** PARAMETER CODE *********************
					clearBuffer1();
					flashArrayRead(getStageEffectParam(main_stage,main_effect,main_param).flash_label_addr+6,(uint8_t *)name_buffer,10);
					buffer1Cat( name_buffer);
					buffer1Cat( " ");
					buffer1UIntCat(fxcoreParameterUpdate(main_stage,main_effect,main_param,0,getStageEffectParam(main_stage,main_effect,main_param).FXCore_value));
					clearBuffer2();
				}
				i = 0;
				break;
#if(DBG_MAIN)
			case 5: // display FXCore instructions in flash
				if (count_dir || (state_change == 1))
				{
					setStageLutNum(0);
					if (state_change == 1)
					{
						flashModelStageStart(main_model,main_stage);  // set flash-pointer
					}
					else
					{
						if (count_dir == 1) incFP();
						else if (count_dir == 2) decFP();
					}
					value_change = 1;
					//********************** PARAMETER CODE *********************
					main_flash_address = getFP();
					clearBuffer1();
					buffer1ULongCat(main_flash_address);
					buffer1Cat(" ");
					buffer1ByteCat(getDownTab());
					buffer1Cat("/");
					buffer1ByteCat(getUpTab());
					buffer1Cat(" ");
					buffer1ByteCat(flashRead(main_flash_address+1)); // FXCore address

					clearBuffer2();
					buffer2ByteCat(flashRead(main_flash_address+2));
					buffer2Cat(" ");
					buffer2ByteCat(flashRead(main_flash_address+3));
					buffer2Cat(" ");
					buffer2ByteCat(flashRead(main_flash_address+4));
					buffer2Cat(" ");
					buffer2ByteCat(flashRead(main_flash_address+5));
				}
				break;
			case 6:
				if (count_dir || (state_change == 1))
				{
					if (state_change == 1)
					{
						main_fxcore_address = 0;
					}
					else
					{
						if (count_dir == 1) main_fxcore_address++;
						else if (count_dir == 2) main_fxcore_address--;
					}
					value_change = 1;
					//********************** PARAMETER CODE *********************
					clearBuffer1();
					clearBuffer2();
					main_instr = fxcoreRead(main_stage+1,(uint32_t)(main_fxcore_address));
					buffer1UIntCat(main_fxcore_address);
					buffer1Cat( " ");
					buffer2ByteCat(main_instr.main);
					buffer2Cat( " ");
					buffer2ByteCat(main_instr.data1);
					buffer2Cat( " ");
					buffer2ByteCat(main_instr.data2);
					buffer2Cat( " ");
					buffer2ByteCat(main_instr.data3);
					buffer2Cat( " ");
				}
				break;
			case 7: // display FXCore LUT pointer in flash
				if (count_dir || (state_change == 1))
				{
					if (state_change == 1)
					{
						setFP(4096 + flashModelStageStart(main_model,main_stage)); // flash block after FXCore instr block
						setStageLutNum(0);
					}
					else
					{
						if (count_dir == 1)
						{
							if (incFP() == 0) incStageLutNum();
						}
						else if (count_dir == 2)
						{
							if (decFP() == 0) decStageLutNum();
						}
					}
					value_change = 1;
					//********************** PARAMETER CODE *********************
					main_flash_address = getFP();
					clearBuffer1();
					buffer1ULongCat(main_flash_address);
					buffer1Cat(" ");
					buffer1ByteCat(getDownTab());
					buffer1Cat("/");
					buffer1ByteCat(getUpTab());
					buffer1Cat(" ");
					buffer1ByteCat(flashRead(main_flash_address+1)); // Flash address high
					buffer1Cat(" ");
					buffer1ByteCat(flashRead(main_flash_address+2)); // Flash address mid
					buffer1Cat(" ");
					buffer1ByteCat(flashRead(main_flash_address+3)); // Flash address low

					clearBuffer2();
					buffer2ULongCat(getStageLutFlashPtr(main_stage,getStageLutNum()).flash_base_addr); // Stage_base_addr
				}
				break;
			case 8: 
				if (count_dir || (state_change == 1))
				{
					if (state_change == 1)
					{
						setFP(getStageLutFlashPtr(main_stage,getStageLutNum()).flash_base_addr);//4096 + 256 + flashModelStageStart(main_model,main_stage);
					}
					else
					{
						if (count_dir == 1) incFP();
						else if (count_dir == 2) decFP();
					}
					value_change = 1;
					//********************** PARAMETER CODE *********************
					main_flash_address = getFP();
					clearBuffer1();
					buffer1ULongCat(main_flash_address);
					buffer1Cat(" ");
					buffer1ByteCat(getDownTab());
					buffer1Cat("/");
					buffer1ByteCat(getUpTab());
					buffer1Cat(" ");
					buffer1ByteCat(flashRead(main_flash_address+1)); // FXCore address high
					buffer1Cat(" ");
					buffer1ByteCat(flashRead(main_flash_address+2)); // FXCore address low

					clearBuffer2();
					buffer2ByteCat(flashRead(main_flash_address+3)); // relative addr & MAC/AC indicator
					buffer2Cat(" ");
					buffer2ByteCat(flashRead(main_flash_address+4)); // data high
					buffer2Cat(" ");
					buffer2ByteCat(flashRead(main_flash_address+5)); // data mid
					buffer2Cat(" ");
					buffer2ByteCat(flashRead(main_flash_address+6)); // data low
				}
				break;
			case 9: 
				if (count_dir || (state_change == 1))
				{
					if (state_change == 1)
					{
						main_fxcore_address = 512;
					}
					else
					{
						if (count_dir == 1) main_fxcore_address++;
						else if (count_dir == 2) main_fxcore_address--;
					}
					value_change = 1;
					//********************** PARAMETER CODE *********************
					clearBuffer1();
					clearBuffer2();
					main_instr = fxcoreRead(main_stage+1,(uint32_t)(main_fxcore_address));
					buffer1UIntCat(main_fxcore_address);
					buffer1Cat(" ");
					buffer2ByteCat(main_instr.main);
					buffer2Cat(" ");
					buffer2ByteCat(main_instr.data1);
					buffer2Cat(" ");
					buffer2ByteCat(main_instr.data2);
					buffer2Cat(" ");
					buffer2ByteCat(main_instr.data3);
					buffer2Cat(" ");
				}
				break;
#endif
			default:;	 
			}
			state_change = 0;
			LCD_change = 1;
			setRxDone(0);

			delay(20000);

			if ((LCD_change == 1))
			{
				Display(getBuffer1(), getBuffer2());
				LCD_change = 0;
			}
		}
		else // FPGA configuration error.  Put FPGA diagnostics  in here ?
		{
			Display("FPGA section is","blank or corrupt");
			delay(300000);
		}
	}
	return 0;
}



