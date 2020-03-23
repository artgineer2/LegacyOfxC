#ifndef UTILITIES_H
#define UTILITIES_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern void delay(uint32_t delay);

extern uint8_t bin2Dec(uint8_t *word);

extern void dec2Bin(uint8_t *bin, uint8_t dec);
/*************************** TO BE USED LIKE STRCAT()****************************/
extern void byteCat(char *string, uint8_t value);

extern void intCat(char *string, uint16_t value);

extern void longCat(char *string, uint32_t value);

/********************************************************************************/

extern void sectCpy(uint8_t *trgt, uint8_t trgt_start, uint8_t *src, uint8_t src_start, uint8_t length);

extern void clearBuffer(char *buffer, uint8_t length);

extern uint8_t ascii2OfxUi(uint8_t input);

/****************************************************************************
*
*				Convert FXCore stage, section and address to Flash address
*
****************************************************************************/
extern uint32_t fxcoreSect2FlashAddr(uint8_t int_model, uint8_t int_section, uint16_t int_address);

extern void flashAddr2FXCoreSect(uint32_t int_flash_addr, uint8_t *int_model, uint8_t *int_section, uint16_t *int_address);

extern void section2Stage(uint8_t section, uint16_t sect_addr, uint8_t *stage, uint16_t *stage_addr);

extern void stage2Section(uint8_t stage, uint16_t stage_addr, uint8_t *section, uint16_t *sect_addr);



#endif
