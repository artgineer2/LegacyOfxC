/*
 * ofx.h
 *
 *  Created on: Mar 19, 2020
 *      Author: mike
 */

#ifndef OFX_H_
#define OFX_H_

/*extern uint8_t FPGA_section_status;
extern uint8_t main_state;
extern uint8_t Flash_header_status;
extern uint8_t Flash_stage_status[3];
extern uint8_t model_change;*/

extern uint8_t getMainStage(void);
extern uint8_t getMainEffect(void);
extern void setFpgaSectionStatus(uint8_t status);
extern uint8_t getFpgaSectionStatus(void);
extern void setMainState(uint8_t state);
extern uint8_t getMainState(void);
extern void setFlashHeaderStatus(uint8_t status);
extern uint8_t getFlashHeaderStatus(void);
extern void setFlashStageStatus(uint8_t stageIndex, uint8_t status);
extern uint8_t getFlashStageStatus(uint8_t stageIndex);
extern void setModelChange(uint8_t status);
extern uint8_t getModelChange(void);


#endif /* OFX_H_ */
