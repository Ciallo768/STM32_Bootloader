#ifndef W25Q64_H
#define W25Q64_H
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stdint.h"
typedef struct _W25Q64{
SPI_HandleTypeDef *hspi;
GPIO_TypeDef *CS_Port;
//uint16_t CS_Pin;
uint32_t device_id;
void (*cs_low)(struct _W25Q64 *self);
void (*cs_high)(struct _W25Q64 *self);
void (*write_enable)(struct _W25Q64 *self);
void (*wait_busy)(struct _W25Q64 *self);
uint32_t (*read_id)(struct _W25Q64 *self);
void (*read_data)(struct _W25Q64 *self, uint32_t addr, uint8_t *buf, uint16_t len);
void (*page_program)(struct _W25Q64 *self, uint32_t addr, uint8_t *buf, uint16_t len);
void (*sector_erase)(struct _W25Q64 *self, uint32_t addr);
}W25Q64;
W25Q64 W25Q64_Create(SPI_HandleTypeDef *hspi,GPIO_TypeDef *CS_Port);

#endif
