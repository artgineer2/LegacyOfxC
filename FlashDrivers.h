
#ifndef FLASH_DRIVERS_H
#define FLASH_DRIVERS_H

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Macros.h"

/***********************************************************
*                    Flash Routines
***********************************************************/
extern void flashEn(void);
extern void flashDis(void);

/***********************************************************
*		Setting flash Write Enable
*		(Do this before writing to the flash)
*
***********************************************************/
extern void flashWREN(void);

/***********************************************************
*		Resetting flash Write Enable
*		(Do this after writing to the flash)
*
***********************************************************/
extern void flashWRDI(void);

/***********************************************************
*		Reading flash Status Register
***********************************************************/
extern uint8_t flashRDSR(void);

extern void flashInit(void);

extern uint8_t flashRead(uint32_t address);

/***********************************************************
*
*				Writing to the flash
*
***********************************************************/
extern uint8_t flashWrite(uint32_t address, uint8_t data);


/***********************************************************
*
*				Writing sequence to the flash (from the PC)
*
***********************************************************/
extern uint8_t flashSeqWrite(uint32_t start_address, uint8_t data_in, uint8_t byte_status);


/***********************************************************
*
*				Reading array from the flash
*
***********************************************************/
extern uint8_t flashArrayRead(uint32_t start_address, uint8_t *data_out, uint8_t array_size);


/***********************************************************
*
*				Writing array to the flash
*
***********************************************************/
extern uint8_t flashArrayWrite(uint32_t start_address, uint8_t *data_in, uint8_t array_size);

/***********************************************************
*		Erasing 4K flash blocks
*		(erases entire FXCore Section)
***********************************************************/
extern uint8_t flash4kErase(uint32_t starting_address);

/***********************************************************
*		Erasing 64K flash blocks
*		(erases half of FPGA section)
***********************************************************/
extern uint8_t flash64kErase(uint32_t starting_address);


extern uint8_t flashChipErase(void);

/*********************************************************
*
*		Move flash data array from one section to another
*
*********************************************************/
extern uint8_t flashArrayMove(uint32_t target_starting_addr, uint32_t source_starting_addr, uint16_t array_size);



#endif
