#ifndef FXCORE_OP_H
#define FXCORE_OP_H
#include <avr/io.h>
#include <stdint.h>

#include "Structs.h"
/************************************************************
*
*				FXCore Stage Save: FXCore -> Flash
*				(update to include LUTs)
*
************************************************************/
extern uint8_t flash_status;
extern uint8_t fxcoreStageSave(uint8_t model_index, uint8_t stage_index);


/*************************************************************
*
*		Get address array for main UI.
*		Use when changing models
*
*************************************************************/
extern uint8_t fxcoreHeaderLoad(uint8_t model);

extern uint8_t fxcoreStageLoad(uint8_t model_index, uint8_t stage_index);


extern uint16_t fxcoreParameterUpdate(uint8_t stage_index, uint8_t effect_index, uint8_t parameter_index, uint8_t parameter_type, uint8_t in_value);



extern void fxcoreThruStage(uint8_t stage_index);


/************************************************************
*					NEEDED???????
*
*					Stage Instr Erase
*
************************************************************/
extern void fxcoreInstrErase(uint8_t stage);

/************************************************************
*					NEEDED???????
*
*					Stage Erase
*
************************************************************/
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
extern void freeStageEffectParams(uint8_t stage_index, uint8_t effect_index);
extern void freeStageEffects(uint8_t stage_index);
extern uint8_t getStageEffectsCount(uint8_t stage_index);
extern uint8_t getStageEffectParamCount(uint8_t stage_index, uint8_t effect_index);
extern void setStageLutNum(uint8_t lutIndex);
extern uint8_t getStageLutNum();
extern void incStageLutNum(void);
extern void decStageLutNum(void);
#endif
