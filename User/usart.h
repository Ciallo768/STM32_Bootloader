#ifndef USART_H
#define USART_H
#include "stdint.h"
#include  "stm32f1xx_hal.h"
#include "string.h"

#define RING_BUF_SIZE 512
#define RX_Data_SIZE 256
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;

typedef struct _UART_Device{
	UART_HandleTypeDef *huart;
	DMA_HandleTypeDef *hdma;
	uint8_t UART_RING_BUF[RING_BUF_SIZE];//环形缓冲区
	uint8_t RX_Data[RX_Data_SIZE];//原始的DMA数据接收数组
	uint16_t write_pos;
	uint16_t read_pos;
	
	void(*Init)(struct _UART_Device *self,UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
    uint16_t (*Read)(struct _UART_Device *self, uint8_t *buf, uint16_t len);
    uint16_t (*Write)(struct _UART_Device *self, uint8_t *data, uint16_t len);
	uint16_t(*Available)(struct _UART_Device*self);
	void(*Rx_callback)(struct _UART_Device*self,uint16_t size);
	uint16_t(*Send)(struct _UART_Device *self,const uint8_t *pData,uint16_t len);
	void(*Clear)(struct _UART_Device *self);
}UART_Device;

void UART_Device_Init(UART_Device *dev, UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma);
#endif
