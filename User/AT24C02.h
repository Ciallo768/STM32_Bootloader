#ifndef AT24C02_H
#define AT24C02_H
typedef struct _OTA_InfoCB OTA_InfoCB;

#include "stm32f1xx_hal.h"
#include "My_main.h"


typedef struct _AT24C02{
	I2C_HandleTypeDef *hi2c;
	uint8_t device_addr;//设备地址
	uint8_t buffer[8];//一页最大8字节
	
	void(*WriteByte)(struct _AT24C02 *self,uint8_t mem_addr,uint8_t data);
	void(*WritePage)(struct _AT24C02 *self, uint8_t mem_addr, uint8_t *data, uint8_t len);
	uint8_t(*ReadByte)(struct _AT24C02* self,uint8_t mem_addr);//
	void(*ReadData)(struct _AT24C02*self,uint8_t mem_addr, uint8_t *data, uint8_t len);//
	void (*ReadOTA)(struct _AT24C02*self, OTA_InfoCB *buf);//
	void (*WriteOTA)(struct _AT24C02*self);
}AT24C02;
//构造函数
AT24C02 AT24C02_Create(I2C_HandleTypeDef *hi2c,uint8_t addr);

#endif
