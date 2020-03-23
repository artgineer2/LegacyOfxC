#include "UiOp.h"

#include "FlashDrivers.h"
#include "FXCoreOp.h"
#include "Ofx.h"
#include "UiDrivers.h"





static uint8_t buffer1[17];
static uint8_t buffer2[17];
static uint8_t IndBuf[17];
static uint8_t abbr_buffer[5];

void setBuffer1(char *string)
{
	strncpy((char *)buffer1,string,16);
	buffer1[16] = '\0';
}
void setBuffer2(char *string)
{
	strncpy((char *)buffer2,string,16);
	buffer2[16] = '\0';
}
void setIndBuf(char *string)
{
	strncpy((char *)IndBuf,string,16);
	IndBuf[16] = '\0';
}
void setAbbrBuffer(char *string)
{
	strncpy((char *)abbr_buffer,string,4);
	abbr_buffer[4] = '\0';
}




void buffer1Cat(char *str)
{
	strcat((char *)buffer1, str);
}

void buffer2Cat(char *str)
{
	strcat((char *)buffer1, str);
}

void buffer1ByteCat(uint8_t byte)
{
	byteCat((char *)buffer1, byte);
}
void buffer2ByteCat(uint8_t byte)
{
	byteCat((char *)buffer2, byte);
}
void buffer1UIntCat(uint16_t integer)
{
	intCat((char *)buffer1, integer);
}
void buffer2UIntCat(uint16_t integer)
{
	intCat((char *)buffer2, integer);
}
void buffer1ULongCat(uint32_t longVar)
{
	longCat((char *)buffer1, longVar);
}
void buffer2ULongCat(uint32_t longVar)
{
	longCat((char *)buffer2, longVar);
}

void indBufCat(char *chr)
{
	strcat((char *)IndBuf,chr);
}



char *getBuffer1(void)
{
	return (char *)buffer1;
}
char *getBuffer2(void)
{
	return (char *)buffer2;
}
char *getIndBuf(void)
{
	return (char *)IndBuf;
}
char *getAbbrBuffer(void)
{
	return (char *)abbr_buffer;
}

void clearBuffer1(void)
{
	memcpy(buffer1,'\0',17*sizeof(char));
}
void clearBuffer2(void)
{
	memcpy(buffer2,'\0',17*sizeof(char));
}
void clearIndBuffer(void)
{
	memcpy(IndBuf,'\0',17*sizeof(char));
}
void clearAbbrBuffer(void)
{
	memcpy(abbr_buffer,'\0',5*sizeof(char));
}
/***********************************************************
 *					Initializing LCD Display
 ***********************************************************/
void lcdInit(void)
{
	delay(2000); //1000
	lcdInstr(0x030);
	delay(666); //333
	lcdInstr(0x030);
	delay(200); //100
	lcdInstr(0x030);
	delay(200); //100
	lcdInstr(0b00111000);
	lcdInstr(0x010);
	lcdInstr(0x001);
	lcdInstr(0b00000110);
	lcdInstr(0b00001100);
}


void uiIndBufLoad(uint8_t state, uint8_t index, uint8_t index_max)
{
	uint8_t i;
	uint8_t result;

	result = index/3;
	IndBuf[0] = '\0';

	switch(state)
	{
	case 2: // effect ABBRs
		for(i = 0; i < 3; i++)
		{
			if ((3*result+i) < index_max)
			{
				flashArrayRead(getStageEffect(main_stage,3*result+i).flash_label_addr + 2,(uint8_t *)abbr_buffer,4);
				strcat((char *)IndBuf," ");
				strcat((char *)IndBuf,(char *)abbr_buffer);
			}
			else
			{
				strcat((char *)IndBuf,"     ");
			}
		}
		break;
	case 3:	// parameter ABBRs
		for(i = 0; i < 3; i++)
		{
			if ((3*result+i) < index_max)
			{
				flashArrayRead(getStageEffectParam(main_stage,main_effect,3*result+i).flash_label_addr + 2,(uint8_t *)abbr_buffer,4);
				strcat((char *)IndBuf," ");
				strcat((char *)IndBuf,(char *)abbr_buffer);
			}
			else
			{
				strcat((char *)IndBuf,"     ");
			}
		}
		break;
	default:
		break;
	}
}


void capInd(char *target, char *source, uint8_t select)
{
	uint8_t char_count, sp_count;
	sp_count = 0;

	while(select >= 3)
	{
		select = select - 3;
	}

	for(char_count = 0; char_count < 16; char_count++)
	{ 
		if (isalpha(source[char_count]))
		{
			if(sp_count == (select + 1))
			{
				target[char_count] = source[char_count] - 32;
			}
			else
			{
				target[char_count] = source[char_count];
			}
		}
		else if (source[char_count] == ' ')
		{
			sp_count++;
			target[char_count] = source[char_count];
		}
		else if ((source[char_count] == '-') && (sp_count == (select + 1)))
		{
			target[char_count] = '*';
		}
		else
		{
			target[char_count] = source[char_count];
		}
	}
	target[16] = '\0';
}


/***********************************************************
 *		Writing strings to LCD Display
 ***********************************************************/
void Display(char *line1, char *line2)
{
	uint8_t n=0;

	lcdInstr(0x01);
	lcdInstr(0b00000110);
	while (line1[n] != '\0')
	{
		lcdData((char)line1[n]);
		n++;
	}

	n=0;
	lcdInstr(0b11000000);
	while (line2[n] != '\0')
	{
		lcdData((char)line2[n]);
		n++;
	}
}


/*************************************************************
 *
 *		Get address array for stage UI.
 *		Use when changing stages in model.
 *		flash_value:  	0: don't import values from flash
 *						1: import values from flash
 *************************************************************/
void uiStageClear(uint8_t stage_index)
{
	uint8_t i;

	for(i = 0; i < getStageEffectsCount(stage_index); i++)
	{
		freeStageEffectParams(stage_index,i);
	}
	freeStageEffects(stage_index);

}

void debugDisplay(uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4, uint16_t data5, uint16_t data6)
{
	uint8_t debug_str1[16];
	uint8_t debug_str2[16];

	debug_str1[0] = '\0';
	debug_str2[0] = '\0';

	intCat((char *)debug_str1,data1);
	strcat((char *)debug_str1," ");
	intCat((char *)debug_str1,data2);
	strcat((char *)debug_str1," ");
	intCat((char *)debug_str2,data3);
	strcat((char *)debug_str2," ");
	intCat((char *)debug_str2,data4);
	strcat((char *)debug_str2," ");
	intCat((char *)debug_str2,data5);
	strcat((char *)debug_str2," ");
	intCat((char *)debug_str2,data6);
	strcat((char *)debug_str2," ");

	Display((char *)debug_str1,(char *)debug_str2);
}











