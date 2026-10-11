#include "flash.h"
#include "stm32f1xx_hal.h"
#include "printf.h"
void STM32_EraseFlash(uint32_t startPageAddr, uint16_t pageCount)
{
    HAL_FLASH_Unlock();   // 解锁 Flash

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = startPageAddr;
    EraseInitStruct.NbPages = pageCount;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        // 错误处理（可添加返回值或打印）
		MyPrintf("flash erase error!!!");
    }

    HAL_FLASH_Lock();     // 上锁
}
//地址 数组 字长
void STM32_WriteFlash(uint32_t addr, uint32_t *data, uint32_t wordCount)
{
    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < wordCount; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data[i]);
        addr += 4;
    }

    HAL_FLASH_Lock();
}
