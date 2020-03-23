/*
 * ofx.h
 *
 *  Created on: Mar 19, 2020
 *      Author: mike
 */

#ifndef OFX_H_
#define OFX_H_

extern uint8_t FPGA_section_status;
extern uint8_t main_state;
extern uint8_t main_model;
extern uint16_t main_fxcore_address;
extern uint32_t main_flash_address;
extern uint8_t main_stage;
extern uint8_t main_effect;
extern uint8_t main_param;
extern uint8_t Flash_header_status;
extern uint8_t Flash_stage_status[3];
extern uint8_t state_change;
extern uint8_t model_change;
extern uint8_t stage_change;
extern uint8_t effect_change;
extern uint8_t param_change;


#endif /* OFX_H_ */
