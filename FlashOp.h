#ifndef FLASH_OP_H
#define FLASH_OP_H
#include <stdint.h>

#include "Structs.h"
/******************************************************************************
*
*		Relocates Flash Pointer to the start of the model header section for
*			given model
*
*******************************************************************************/

extern uint32_t flashModelHeaderStart(uint8_t model);


/******************************************************************************
*
*		Relocates Flash Pointer to the start of the model stage section for
*			given model
*
*******************************************************************************/
extern uint32_t flashModelStageStart(uint8_t model, uint8_t stage);

/******************************************************************************
*
*		Relocates Flash Pointer to the start of the scratch header section
*
*******************************************************************************/
extern uint32_t flashScratchHeaderStart(void);


/******************************************************************************
*
*		Relocates Flash Pointer to the start of the scratch stage section
*
*******************************************************************************/
extern uint32_t flashScratchStageStart(uint8_t stage);

/****************************************************************************
*
*		Get_up_tab: returns the fxb word type on the up tab.
*
****************************************************************************/
extern uint8_t getUpTab(void);

/****************************************************************************
*
*		Get_down_tab: returns the fxb word type on the down tab.
*
****************************************************************************/
extern uint8_t getDownTab(void);

extern uint8_t incFP(void);

extern uint8_t decFP(void);

extern void paramLutFlashPointerInit(uint8_t model);
extern void stageLutFlashPointerInit(uint8_t model, uint8_t stage);

extern void setFP(uint32_t flashPtr);
extern uint32_t getFP(void);

extern void setParamLutFlashPtr(uint8_t index, ParamLutFlashPointer_t paramLutFlashPtr);
extern ParamLutFlashPointer_t getParamLutFlashPtr(uint8_t index);

extern void setParamLutType(uint8_t paramLutType);
extern uint8_t getParamLutType();

extern void setParamLutTypeNum(uint8_t paramLutTypeNum);
extern uint8_t getParamLutTypeNum();

extern void setStageLutFlashPtr(uint8_t stage_index, uint8_t stage_lut_index,
 		StageLutFlashPointer_t stageLutFlashPtr);
extern StageLutFlashPointer_t getStageLutFlashPtr(uint8_t stage_index, uint8_t stage_lut_index);

extern void setStageLutNumMax(uint8_t stageLutNumMax);
extern uint8_t getStageLutNumMax();

#endif
