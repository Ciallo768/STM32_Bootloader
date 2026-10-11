#ifndef Flash_H
#define Flash_H
#include "stdint.h"
void STM32_EraseFlash(uint32_t startPageAddr, uint16_t pageCount);
void STM32_WriteFlash(uint32_t addr, uint32_t *data, uint32_t wordCount);




#endif
