/*
 * FlashModule.h
 *
 *  Created on: Mar 28, 2020
 *      Author: buildrooteclipse
 */

#ifndef FLASHMODULE_H_
#define FLASHMODULE_H_

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Macros.h"

#include <stdint.h>

#include "CustomTypes.h"
/******************************************************************************
*
*		Relocates Flash Pointer to the start of the model header section for
*			given model
*
*******************************************************************************/
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


extern void flashEn(void);
extern void flashDis(void);
extern void flashInit(void);

extern uint8_t flashRead(uint32_t address);

extern uint8_t flashWrite(uint32_t address, uint8_t data);

extern uint8_t flashSeqWrite(uint32_t start_address, uint8_t data_in, uint8_t byte_status);

extern uint8_t flashArrayRead(uint32_t start_address, uint8_t *data_out, uint8_t array_size);

extern uint8_t flash4kErase(uint32_t starting_address);
extern uint8_t flash64kErase(uint32_t starting_address);
extern uint8_t flashChipErase(void);

extern uint32_t flashModelHeaderStart(uint8_t model);

extern uint32_t flashModelStageStart(uint8_t model, uint8_t stage);
extern uint32_t flashScratchStageStart(uint8_t stage);
extern uint8_t getUpTab(void);
extern uint8_t getDownTab(void);

extern uint8_t incFP(void);

extern uint8_t decFP(void);

extern void paramLutFlashPointerInit(uint8_t model);
extern void stageLutFlashPointerInit(uint8_t model, uint8_t stage);

extern void setFP(uint32_t flashPtr);
extern uint32_t getFP(void);

extern void setParamLutFlashPtr(uint8_t index, ParamLutFlashPointer_t paramLutFlashPtr);
extern ParamLutFlashPointer_t getParamLutFlashPtr(uint8_t index);

extern void setStageLutFlashPtr(uint8_t stage_index, uint8_t stage_lut_index,
 		StageLutFlashPointer_t stageLutFlashPtr);
extern StageLutFlashPointer_t getStageLutFlashPtr(uint8_t stage_index, uint8_t stage_lut_index);

extern void setStageLutNumMax(uint8_t stageLutNumMax);
extern uint8_t getStageLutNumMax();


#endif /* FLASHMODULE_H_ */
