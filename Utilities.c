
#include "Utilities.h"

#include "Constants.h"
#include "Macros.h"



void delay(uint32_t delay)
{
	uint32_t i;

	for (i = 0; i < delay; i++)
	{
		NOP();
	}
}


uint8_t bin2Dec(uint8_t *word)
{
	uint8_t dec;
	uint8_t mult;
	uint8_t i;

	dec = 0;
	mult = 1;
	for(i = 0; i < 8; i++)
	{
		if (word[7-i] == '1')
		{
			dec = dec + mult;
		}
		mult = mult*2;
	}

	return dec;
}



void dec2Bin(uint8_t *bin, uint8_t dec)
{
	uint8_t mult;
	uint8_t i;
	uint8_t bin_str[9];

	mult = 128;

	for(i = 0; i < 8; i++)
	{
		if (dec >= mult)
		{
			dec = dec - mult;
			bin_str[i] = '1';
		}
		else
		{
			bin_str[i] = '0';
		}
		mult = mult/2;
	}
	bin_str[8] = '\0';
	memcpy(bin,bin_str,1);
}


/*************************** TO BE USED LIKE STRCAT()****************************/
void byteCat(char *string, uint8_t value)
{
	char num[4];
	uint8_t str_len;
	uint8_t digit_len;
	uint8_t digit_pos;


	str_len = strlen(string);

	itoa(value,num,10);
	digit_len = strlen(num);


	for(digit_pos = 0; digit_pos < digit_len; digit_pos++)
	{
		string[str_len + digit_pos] = num[digit_pos];
	}
	string[str_len + digit_len] = '\0';

}

void intCat(char *string, uint16_t value)
{

	char num[6];
	uint8_t str_len;
	uint8_t digit_len;
	uint8_t digit_pos;

	str_len = strlen(string);

	itoa(value,num,10);
	digit_len = strlen(num);


	for(digit_pos = 0; digit_pos < digit_len; digit_pos++)
	{
		string[str_len + digit_pos] = num[digit_pos];
	}
	string[str_len + digit_len] = '\0';

}

void longCat(char *string, uint32_t value)
{

	char num[11];
	uint8_t str_len;
	uint8_t digit_len;
	uint8_t digit_pos;


	str_len = strlen(string);

	ltoa(value,num,10);
	digit_len = strlen(num);


	for(digit_pos = 0; digit_pos < digit_len; digit_pos++)
	{
		string[str_len + digit_pos] = num[digit_pos];
	}
	string[str_len + digit_len] = '\0';
}

/********************************************************************************/


void sectCpy(uint8_t *trgt, uint8_t trgt_start, uint8_t *src, uint8_t src_start, uint8_t length)
{
	uint8_t i;

	for(i = 0; i < length; i++)
	{
		trgt[i+trgt_start] = src[i+src_start];
	}
	trgt[length+trgt_start] = '\0';
}



void clearBuffer(char *buffer, uint8_t length)
{
	uint8_t i;

	for(i = 0; i < length; i++)
	{
		buffer[i] = 0;
	}

	buffer[length] = '\0';
}



uint8_t ascii2OfxUi(uint8_t input)  // this should be done in PC software
{
	uint8_t output;

	if ((input >= '0') && (input <= '9'))
	{
		output = input - '0';  // convert ascii number t0 ofxui number
	}
	if ((input >= 'a') && (input <= 'z'))
	{
		output = input - 'a' + 9; // convert ascii letter to ofxui letter (only lower case letters are used in comments)
	}

	return output;
}

/****************************************************************************
 *
 *				Convert FXCore stage, section and address to Flash address
 *
 ****************************************************************************/
uint32_t fxcoreSect2FlashAddr(uint8_t int_model, uint8_t int_section, uint16_t int_address)
{
	uint32_t flash_addr;

	if (int_section == 0)
	{
		flash_addr = BINFILE_LIMIT + MODEL_SIZE*int_model + int_address;
	}
	else
	{
		flash_addr = BINFILE_LIMIT + MODEL_SIZE*int_model + SECTION_SIZE*int_section + 4*int_address;
	}

	return flash_addr;
}

void flashAddr2FXCoreSect(uint32_t int_flash_addr, uint8_t *int_model, uint8_t *int_section, uint16_t *int_address)
{
	uint32_t model_storage_addr; // address within model storage section
	uint8_t model;
	uint8_t section; 	// 0 = header
	// 1 = stage 1 instr, 2 = stage 1 LUT
	// 3 = stage 2 instr, 4 = stage 2 LUT
	// 5 = stage 3 instr, 6 = stage 3 LUT

	uint16_t model_addr; // address within model
	uint16_t section_addr; // address within section

	model_storage_addr = int_flash_addr - BINFILE_LIMIT; // address within model storage section
	model = (model_storage_addr/MODEL_SIZE);
	model_addr = model_storage_addr - model*MODEL_SIZE;

	section = model_addr/SECTION_SIZE;
	section_addr = model_addr - SECTION_SIZE*section;

	if (section == 0)
	{
		*int_address = section_addr;
	}
	else
	{
		*int_address = section_addr/4;
	}

	*int_model = model;
	*int_section = section;
}

void section2Stage(uint8_t section, uint16_t sect_addr, uint8_t *stage, uint16_t *stage_addr)
{
	switch(section)
	{
	case 1:
		*stage = 0;
		*stage_addr = sect_addr;
		break;
	case 2:
		*stage = 0;
		*stage_addr = sect_addr+512;
		break;
	case 3:
		*stage = 1;
		*stage_addr = sect_addr;
		break;
	case 4:
		*stage = 1;
		*stage_addr = sect_addr+512;
		break;
	case 5:
		*stage = 2;
		*stage_addr = sect_addr;
		break;
	case 6:
		*stage = 2;
		*stage_addr = sect_addr+512;
		break;
	default:
		break;
	}
}

void stage2Section(uint8_t stage, uint16_t stage_addr, uint8_t *section, uint16_t *sect_addr)
{
	if (stage_addr < 512)
	{
		*section = stage;
		*sect_addr = stage_addr;
	}
	else
	{
		*section = stage + 1;
		*sect_addr = stage_addr - 512;
	}

}

