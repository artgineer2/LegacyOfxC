/*
 * FXCoreModule.h
 *
 *  Created on: Mar 28, 2020
 *      Author: buildrooteclipse
 */

#ifndef FXCOREMODULE_H_
#define FXCOREMODULE_H_

#include <avr/io.h>
#include <stdint.h>


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


typedef struct FXCoreWord
{
	uint8_t main;
	uint8_t data1;
	uint8_t data2;
	uint8_t data3;
} FXCoreWord_t;


extern uint8_t fpgaConfig(void);

extern void fpgaRstEn(void);
extern void fpgaRstDis(void);

extern void fxcoreWrite(uint8_t stage, uint32_t address, FXCoreWord_t input_instr);
extern FXCoreWord_t fxcoreRead(uint8_t stage, uint32_t address);

extern uint8_t flash_status;
extern uint8_t fxcoreStageSave(uint8_t model_index, uint8_t stage_index);

extern uint8_t fxcoreHeaderLoad(uint8_t model);

extern uint8_t fxcoreStageLoad(uint8_t model_index, uint8_t stage_index);


extern uint16_t fxcoreParameterUpdate(uint8_t stage_index, uint8_t effect_index, uint8_t parameter_index, uint8_t parameter_type, uint8_t in_value);



extern void fxcoreInstrErase(uint8_t stage);
extern void fxcoreLutErase(uint8_t stage);

extern void setModelNameAddr(uint32_t addr);
extern uint32_t getModelNameAddr();
extern void setFswAddr(uint8_t index, uint32_t addr);
extern uint32_t getFswAddr(uint8_t index);
extern void setStage(uint8_t index, Stage_t _stage);
extern Stage_t getStage(uint8_t index);
extern Effect_t *getStageEffects(uint8_t stage_index);
extern Effect_t getStageEffect(uint8_t stage_index, uint8_t effect_index);
extern Parameter_t *getStageEffectParams(uint8_t stage_index, uint8_t effect_index);
extern Parameter_t getStageEffectParam(uint8_t stage_index, uint8_t effect_index, uint8_t param_index);
extern uint8_t *getStageEffectParamValuePtr(uint8_t stage_index, uint8_t effect_index, uint8_t param_index);
extern void freeStageEffectParams(uint8_t stage_index, uint8_t effect_index);
extern void freeStageEffects(uint8_t stage_index);
extern uint8_t getStageEffectsCount(uint8_t stage_index);
extern uint8_t getStageEffectParamCount(uint8_t stage_index, uint8_t effect_index);
extern void setStageLutNum(uint8_t lutIndex);
extern uint8_t getStageLutNum();
extern void incStageLutNum(void);
extern void decStageLutNum(void);



#endif /* FXCOREMODULE_H_ */
