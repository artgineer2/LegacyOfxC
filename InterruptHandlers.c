
#include "InterruptHandlers.h"
#include "Constants.h"
#include "CustomTypes.h"
#include "FlashModule.h"
#include "CommModule.h"
#include "FXCoreModule.h"
#include "Ofx.h"
#include "UiModule.h"
#include "Utilities.h"



uint8_t count_up;
uint8_t count_down;
uint8_t count_dir;

//extern uint8_t FPGA_section_status;
static uint8_t rx_done;
static uint32_t rx_count;

uint8_t getRxDone(void)
{
	return rx_done;
}
void setRxDone(uint8_t rxDone)
{
	rx_done = rxDone;
}
uint32_t getRxCount(void)
{
	return rx_count;
}
void setRxCount(uint32_t rxCount)
{
	rx_count = rxCount;
}

static uint8_t rx_type;



static LongBytes_t rx_total;

static LongBytes_t rx_start_addr;

static uint8_t rx_model;
static uint8_t rx_section;
static uint16_t rx_section_address;
static uint8_t rx_stage;
static uint16_t rx_stage_address;

static uint8_t index_count;
static uint16_t word_count;
static FXCoreWord_t instr;

/*************************************************
 *             External Interrupt Handler
 *			  for rotary encoder
 **************************************************/
ISR(INT0_vect)
{
	uint8_t Enc_Dir = 0;


	do
	{
		Enc_Dir = PINF && 0x01;
	}while(!(PIND && 0x02)); // Read Pin F.1 while Pin D.2 is low

	if (Enc_Dir == 1) // if Enc_Dir is high
	{
		count_up = 1;
		count_down = 0;
	}
	else if (Enc_Dir == 0)// if Enc_Dir is low
	{
		count_up = 0;
		count_down = 1;
	}


}

/*************************************************
 *
 *             UART RX Interrupt Handler
 *
 **************************************************/
ISR(USART0_RX_vect)
{
	uint8_t temp = 0;
	uint8_t stage = 0;
	uint16_t address = 0;
	uint16_t i = 0;
	uint16_t j = 0;

	FXCoreWord_t rx_instr;

	switch(rx_count)
	{
	case 0 : // RX Type
		rx_type = UDR0;
		rx_total.long_var = 0;
		rx_start_addr.long_var = 0;
		rx_count++;
		break;
	case 1 : // RX Total (High Byte)
		rx_total.high_byte = UDR0;
		rx_count++;
		break;
	case 2 : // RX Total (Mid Byte)
		rx_total.mid_byte = UDR0;
		rx_count++;
		break;
	case 3 : // RX Total (Low Byte)
		rx_total.low_byte = UDR0;
		rx_count++;
		break;
	case 4:  // RX Starting Address (High Byte)
		rx_start_addr.high_byte = UDR0;
		rx_count++;
		break;
	case 5 : // RX Starting Address (Mid Byte)
		rx_start_addr.mid_byte = UDR0;
		rx_count++;
		break;
	case 6 : // RX Starting Address (Low Byte)
		rx_start_addr.low_byte = UDR0;
		index_count = 0;
		word_count = 0;
		rx_count++;
		break;
	default :
		switch(rx_type)
		{
		case 0: // erase FPGA section of flash
			temp = UDR0;
			Display("Erasing","FPGA Section");
			for(i = 0; i < 4; i++)
			{
				if(flash64kErase(i*65536) != 0)
				{
					Display("Error erasing","FPGA section");
					delay(200000);
				}
			}
			Display("FXCore Code","Erased");
			delay(200000);
			print("done ", 5);
			setFpgaSectionStatus(1);
			setModelChange(1);
			setMainState(0);
			rx_total.long_var = 0;
			rx_count = 0;
			rx_done = 1;
			break;
		case 1: // download FPGA config file for FXCore
		{
			temp = UDR0;
			if (rx_count == 7) // first uint8_t of RXed data
			{
				flashSeqWrite(BINFILE_ADDR, temp,0);
				rx_count++;
			}
			else if ((rx_count-7) == (rx_total.long_var-1)) //all data received
			{
				flashSeqWrite(BINFILE_ADDR,temp,2);
				Display("FXCore Code","Updated");
				delay(300000);
				fpgaConfig();
				rx_done = 1;
				rx_total.long_var = 0;
				rx_count = 0;
				setMainState(0);
				setModelChange(1);
			}
			else
			{
				flashSeqWrite(BINFILE_ADDR,temp,1);
				rx_count++;
			}
		}

		break;
		case 2:	// Read from Flash
			temp = UDR0;
			Display("TXing",NULL);
			clearTxBuffer();

			for(i = 0; i < (TX_BUFFER_SIZE-1); i++)
			{
				txBufferByteCat(flashRead(rx_start_addr.long_var + i));
			}
			print((char *)getTxBuffer(),TX_BUFFER_SIZE);
			setModelChange(1);
			setMainState(0);
			rx_total.long_var = 0;
			rx_count = 0;
			rx_done = 1;
			break;
		case 3: // Erase Flash Sector
			temp = UDR0;
			clearBuffer1();
			clearBuffer2();
			Display("Erasing"," ");

			flashAddr2FXCoreSect(rx_start_addr.long_var,&rx_model,&rx_section,&rx_section_address);

			if(flash4kErase(rx_start_addr.long_var) == 0)
			{
				switch(rx_section)
				{
				case 0: fxcoreHeaderLoad(rx_model);
				Display("Header","erased");
				break;
				case 1:	fxcoreInstrErase(0);
				Display("Stage 1 Instr","erased");
				break;
				case 2:	fxcoreLutErase(0);
				Display("Stage 1 LUT","erased");
				break;
				case 3:	fxcoreInstrErase(1);
				Display("Stage 2 Instr","erased");
				break;
				case 4:	fxcoreLutErase(1);
				Display("Stage 2 LUT","erased");
				break;
				case 5:	fxcoreInstrErase(2);
				Display("Stage 3 Instr","erased");
				break;
				case 6:	fxcoreLutErase(2);
				Display("Stage 3 LUT","erased");
				break;
				case 7: Display("Parameter LUT","sect 1 erased");
				break;
				case 8: Display("Parameter LUT","sect 2 erased");
				break;
				case 9: Display("Parameter LUT","sect 3 erased");
				break;
				case 10: Display("Parameter LUT","sect 4 erased");
				break;
				default:
					Display("Invalid section",NULL);
					break;
				}
			}
			else
			{
				Display("Erase","error");
			}
			setMainState(0);
			delay(200000);
			print("done ", 5);
			setModelChange(1);
			rx_count = 0;
			rx_total.long_var = 0;
			rx_done = 1;
			break;
		case 4: // Write to Flash
			temp = UDR0;
			if (rx_count == 7) // first uint8_t of RXed data
			{
				flashSeqWrite(rx_start_addr.long_var, temp,0);
				rx_count++;
			}
			else if ((rx_count-7) == rx_total.long_var-1) //all data received
			{
				flashSeqWrite(rx_start_addr.long_var, temp,2);
				flashAddr2FXCoreSect(rx_start_addr.long_var,&rx_model,&rx_section,&rx_section_address);
				section2Stage(rx_section, rx_section_address, &rx_stage, &rx_stage_address);
				clearBuffer1();
				if (rx_section == 0)
				{
					setFlashHeaderStatus(fxcoreHeaderLoad(rx_model));
					setMainState(0);
				}
				else
				{
					setFlashStageStatus(rx_stage,fxcoreStageLoad(rx_model, rx_stage));
					if(getFlashStageStatus(rx_stage) != 0) buffer1Cat("error");
					else
					{

						buffer1Cat("stage ");
						buffer1ByteCat(stage);
					}
					setMainState(0);
				}
				Display("done",getBuffer1());
				debugDisplay(getMainState(),0,rx_model,rx_section,rx_stage,rx_stage_address);
				setMainState(0);
				delay(200000);
				print("done ", 5);
				setModelChange(1);
				rx_count = 0;
				rx_total.long_var = 0;
				rx_done = 1;
			}
			else
			{
				flashSeqWrite(rx_start_addr.long_var, temp,1);
				rx_count++;
			}
			break;
		case 5: // RD from FXCore
			temp = UDR0;
			flashAddr2FXCoreSect(rx_start_addr.long_var,&rx_model,&rx_section,&rx_section_address);
			section2Stage(rx_section,rx_section_address, &rx_stage, &rx_stage_address);
			clearBuffer1();
			buffer1ULongCat(rx_start_addr.long_var);
			buffer1Cat(" ");
			buffer1ByteCat(rx_model);
			buffer1Cat(" ");
			buffer1ByteCat(rx_section);
			buffer1Cat(" ");
			buffer1UIntCat(rx_section_address);
			buffer1Cat(" ");

			clearBuffer2();
			buffer2ByteCat(rx_stage);
			buffer2Cat(" ");
			buffer2UIntCat(rx_stage_address);
			Display(getBuffer1(),getBuffer2());
			clearTxBuffer();

			clearTxBuffer();
			i = 0;
			j = rx_stage_address;
			delay(400000);
			while(i < rx_total.long_var)
			{
				rx_instr = fxcoreRead(rx_stage+1, j);
				j++;
				txBufferByteCat(rx_instr.main);
				i++;
				txBufferByteCat(rx_instr.data1);
				i++;
				txBufferByteCat(rx_instr.data2);
				i++;
				txBufferByteCat(rx_instr.data3);
				i++;
			}

			delay(20000);
			print((char *)getTxBuffer(),TX_BUFFER_SIZE);
			setModelChange(1);
			setMainState(0);
			rx_count = 0;
			rx_done = 1;

			break;
		case 6: // Erase FXCore
			temp = UDR0;
			flashAddr2FXCoreSect(rx_start_addr.long_var, &rx_model,&rx_section,&rx_section_address);
			clearBuffer1();
			buffer1ByteCat(stage);
			Display("erasing",getBuffer1());
			switch(rx_section)
			{
			case 1:	fxcoreInstrErase(0);
			Display("Stage 1 Instr","erased");
			break;
			case 2:	fxcoreLutErase(0);
			Display("Stage 1 LUT","erased");
			break;
			case 3:	fxcoreInstrErase(1);
			Display("Stage 2 Instr","erased");
			break;
			case 4:	fxcoreLutErase(1);
			Display("Stage 2 LUT","erased");
			break;
			case 5:	fxcoreInstrErase(2);
			Display("Stage 3 Instr","erased");
			break;
			case 6:	fxcoreLutErase(2);
			Display("Stage 3 LUT","erased");
			break;
			default:
				Display("Invalid section",NULL);
				break;
			}
			delay(200000);
			rx_count = 0;
			rx_done = 1;
			break;
			case 7: // WR to FXCore
				temp = UDR0;
				if (index_count == 3)
				{
					flashAddr2FXCoreSect(rx_start_addr.long_var, &rx_model,&rx_section,&rx_section_address);
					fxcoreWrite(stage+1, address, instr);
					index_count = 0;
				}
				else
				{
					index_count++;
				}
				rx_count = 0;
				rx_done = 1;
				break;
			case 8: // User Interface Control from PC
				temp = UDR0;
				Display("reseting",NULL);
				delay(200000);
				print("done ", 5);
				setModelChange(0);
				rx_count = 0;
				rx_total.long_var = 0;
				rx_done = 1;

				break;
			case 9: temp = UDR0;
			Display("Erasing","Flash Chip");
			if(flashChipErase() == 0)
			{
				Display("Flash Chip","Erased");
				delay(200000);
			}
			else
			{
				Display("Flash Chip","Error");
				delay(200000);
			}
			rx_count = 0;
			rx_done = 1;
			break;
			case 10:temp = UDR0;
			Display("Communication","working");
			delay(200000);
			rx_count = 0;
			rx_done = 1;
			break;
			default:
				break;
		}

	}
}
