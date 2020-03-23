/*
 * structs.h
 *
 *  Created on: Mar 11, 2020
 *      Author: mike
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_
typedef struct LongBytes
{
	union
	{
		uint32_t long_var;
		struct
		{
			uint8_t const pad;
			uint8_t high_byte;
			uint8_t mid_byte;
			uint8_t low_byte;
		};
	};
} LongBytes_t;

typedef struct FXCoreWord
{
	uint8_t main;
	uint8_t data1;
	uint8_t data2;
	uint8_t data3;
} FXCoreWord_t;

typedef struct stage_lut_flash_pointer
{
	uint32_t flash_base_addr; // address in flash
	uint8_t stage_start_addr; // address in FXCore (divided by two)
	uint8_t num_entries;
	uint8_t entry_size;
} StageLutFlashPointer_t;

typedef struct param_lut_flash_pointer
{
	uint32_t flash_base_addr; // address in flash
	uint8_t entry_size; // size of word in flash, including tab (1 uint8_t), UI_value (2 bytes), data (num_data*4 bytes)
} ParamLutFlashPointer_t;

typedef struct parameter
{
	uint32_t flash_label_addr;
	uint8_t FXCore_addr;   // Base address for inserting data from parameter LUT entry
						// into FXCore.
	uint8_t param_type;
	uint8_t FXCore_value;  // Determines parameter LUT entry to be used in FXCore.
						// Data in entry will be inserted into FXCore according to
						// data addressing contained in entry, starting at
						// ...param[param_index].FXCore_addr
} Parameter_t;

typedef struct effect
{
	uint32_t flash_label_addr;
	uint8_t FXCore_addr;
	uint8_t num_param;
	Parameter_t *param;
} Effect_t;

typedef struct stage
{
	uint32_t flash_label_addr;
	uint8_t num_fx;
	struct effect *fx;
} Stage_t;




#endif /* STRUCTS_H_ */
