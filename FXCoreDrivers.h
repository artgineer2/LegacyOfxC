#ifndef FXCORE_DRIVERS_H
#define FXCORE_DRIVERS_H
//#include <stdint.h>
#include <avr/io.h>

#include "CommDrivers.h"
#include "FlashDrivers.h"
#include "IoLines.h"
#include "Structs.h"

/***********************************************************
*
*
*
*
***********************************************************/


extern uint8_t fpgaConfig(void);

extern void fpgaWrEn(void);
extern void fpgaRdEn(void);
extern void fpgaWr(uint8_t data);
extern uint8_t fpgaRd(void);
extern void fpgaRstEn(void);
extern void fpgaRstDis(void);
extern void fxcoreHoldOn(void);
extern void fxcoreHoldOff(void);



/***********************************************************
*		Writing to the FPGA
***********************************************************/
extern void fxcoreWrite(uint8_t stage, uint32_t address, FXCoreWord_t input_instr);


/***********************************************************
*		Reading from the FPGA
***********************************************************/
extern FXCoreWord_t fxcoreRead(uint8_t stage, uint32_t address);






#endif
