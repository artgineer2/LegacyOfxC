#ifndef CONSTANTS_H
#define CONSTANTS_H


#define BINFILE_LIMIT	0x040000
#define MODEL_SIZE		0x020000	// 128kB
#define SECTION_SIZE	0x001000	// 4kB
#define HEADER_SIZE		0x001000	// 4kB
#define STAGE_SIZE		0x002000   	// 8kB

#define BINFILE_ADDR	0x0
#define OFX0_ADDR   	0x040000
#define OFX1_ADDR		0x060000
#define OFX2_ADDR		0x080000
#define OFX3_ADDR		0x0A0000
#define OFX4_ADDR		0x0C0000
#define SCRATCH			0x0E0000
#define FLASH_END		0x0FFFFF

#define TX_BUFFER_SIZE  512
#define RX_BUFFER_SIZE  200

#define MODEL_NAME_TAB			1
#define MODEL_NAME_SIZE			14

#define FOOTSWITCH_TAB			2
#define FOOTSWITCH_SIZE			6

#define HEADER_COMMENT_TAB		3
#define HEADER_COMMENT_SIZE		200

#define STAGE_LABEL_TAB			4
#define STAGE_LABEL_SIZE		16

#define EFFECT_LABEL_TAB		5
#define EFFECT_LABEL_SIZE		16

#define PARAMETER_LABEL_TAB		6
#define PARAMETER_LABEL_SIZE	18

#define FXCORE_INSTR_TAB		7
#define FXCORE_INSTR_SIZE		6

#define STAGE_LUT_POINTER_TAB	8
#define STAGE_LUT_POINTER_SIZE	7

#define STAGE_LUT_TAB			9
#define STAGE_LUT_SIZE			7

#define PARAM_LUT_POINTER_TAB	10
#define PARAM_LUT_POINTER_SIZE	5

#define PARAM_LUT_TAB			11

#define RAM2_CONST_TAB			12
#define RAM2_CONST_SIZE 		6

#define END_TAB					14
#define END_SIZE				6


#endif
