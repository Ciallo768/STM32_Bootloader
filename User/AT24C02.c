#include "AT24C02.h"
#include "string.h"
#include "My_main.h"
#define AT24C02_PAGE_SIZE 8
#define AT24C02_WRITE_DELAY 10   // 写入延时 (ms)
void WriteByte(AT24C02* self,uint8_t mem_addr,uint8_t data){
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(self->hi2c,(self->device_addr)<<1,mem_addr,I2C_MEMADD_SIZE_8BIT,&data, 1, 100);
	if (status == HAL_OK) {
		HAL_I2C_IsDeviceReady(self->hi2c, (self->device_addr << 1), 10, 100);
		}
}

uint8_t ReadByte(AT24C02* self,uint8_t mem_addr){
	uint8_t data;
	HAL_I2C_Mem_Read(self->hi2c,(self->device_addr)<<1,mem_addr,I2C_MEMADD_SIZE_8BIT,&data,1,100);
	return data;
}
void WritePage(AT24C02* self, uint8_t mem_addr, uint8_t *data, uint8_t len){
	if(len >AT24C02_PAGE_SIZE ) len = AT24C02_PAGE_SIZE;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(self->hi2c,(self->device_addr)<<1,mem_addr,I2C_MEMADD_SIZE_8BIT,data, len, 100);
	if (status == HAL_OK) {
        HAL_I2C_IsDeviceReady(self->hi2c, (self->device_addr << 1), 10, 100); 
    }
}

void ReadData(AT24C02 *self, uint8_t mem_addr, uint8_t *data, uint8_t len)
{
    HAL_I2C_Mem_Read(self->hi2c, (self->device_addr << 1), mem_addr,
                     I2C_MEMADD_SIZE_8BIT, data, len, 100);
}
//读取AT24C02中的结构体数据
void ReadOTA(AT24C02 *self, OTA_InfoCB *buf)
{
    memset(buf, 0, sizeof(OTA_InfoCB));
    ReadData(self, 0, (uint8_t *)buf, sizeof(OTA_InfoCB));
}

//将结构体数据写入AT24C02
void WriteOTAInfo(AT24C02 *self){
	uint8_t total_len = sizeof(OTA_Info);
	uint8_t i;
	uint8_t * wptr;
	wptr = (uint8_t *)&OTA_Info;
	for(i=0;i<total_len/AT24C02_PAGE_SIZE;i++){
		uint8_t current_addr = i * AT24C02_PAGE_SIZE;
		WritePage(self,current_addr,wptr + current_addr,AT24C02_PAGE_SIZE);
	}
	
}


AT24C02 AT24C02_Create(I2C_HandleTypeDef *hi2c,uint8_t addr){
	AT24C02 dev;
	dev.hi2c = hi2c;
	dev.device_addr = addr;
	memset(dev.buffer,0,sizeof(dev.buffer));
	dev.WriteByte = WriteByte;
	dev.ReadByte = ReadByte;
	dev.WritePage = WritePage;
	dev.ReadData = ReadData;
	dev.ReadOTA = ReadOTA;
	dev.WriteOTA = WriteOTAInfo;
	return dev;
}

