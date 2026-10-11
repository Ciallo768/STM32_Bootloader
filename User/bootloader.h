#ifndef BOOTLOADER_H
#define BOOTLOADER_H
#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "my_main.h"
#include "usart.h"
void BootLoader_Branch(void);
typedef void (*pFunction)(void);
void Jump_To_Application(void);
void Bootloader_DeInit(void);
uint8_t Bootloader_Enter(uint8_t timeout);
void Bootloader_Info(void);
void Bootloader_Event(uint8_t *data,uint16_t datalen);
uint16_t Xmodem_CRC_16(uint8_t *data,uint16_t datlen);
#endif
