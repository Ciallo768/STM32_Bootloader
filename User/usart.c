#include "usart.h"
#include <stdarg.h>
//DMA + IDLE + circular
//配置 uart i2c spi 等 通常采用配置结构体而非返回新对象的方法
/* 写环形缓冲区 */
 uint16_t UART_Ring_Write(UART_Device *self,uint8_t *data,uint16_t len){
	uint16_t remain = (self->write_pos + RING_BUF_SIZE - self->read_pos -1)%RING_BUF_SIZE;
	if(len > remain) len = remain;
	if(len == 0) return 0;
	if(self->write_pos + len <RING_BUF_SIZE ){
		memcpy(&self->UART_RING_BUF[self->write_pos],data,len);
		self->write_pos += len;
	}
	else{
		uint16_t first_len = RING_BUF_SIZE - self->write_pos;
        memcpy(&self->UART_RING_BUF[self->write_pos], data, first_len);
        memcpy(self->UART_RING_BUF, &data[first_len], len - first_len);
        self->write_pos = len - first_len;
	}
	return len;
}

/* 读环形缓冲区 */
 uint16_t UART_Ring_Read(UART_Device *self, uint8_t *buf, uint16_t len) {
    uint16_t count = 0;
    while (count < len && self->read_pos != self->write_pos) {
        buf[count++] = self->UART_RING_BUF[self->read_pos++];
        if (self->read_pos >= RING_BUF_SIZE)
            self->read_pos = 0;
    }
    return count;
}
/*返回环形缓冲区剩余空间*/
 uint16_t UART_Ring_Available(UART_Device *self) {
    if (self->write_pos >= self->read_pos)
        return self->write_pos - self->read_pos;
    else
        return RING_BUF_SIZE - self->read_pos + self->write_pos;
}
 
/* 串口发送数据 （阻塞式发送）*/
 uint16_t UART_Device_Send(UART_Device *self,const uint8_t *pData,uint16_t len){
     
	 HAL_StatusTypeDef st =HAL_UART_Transmit(self->huart,  pData, len,200); 
	 if(st == HAL_OK){
		return len;
	 }
	 else return 0;
}
/*串口回调函数*/
void UART_Device_RxCallback(UART_Device *dev, uint16_t Size) {
    UART_Ring_Write(dev, dev->RX_Data, Size);
    HAL_UARTEx_ReceiveToIdle_DMA(dev->huart, dev->RX_Data, RX_Data_SIZE);
    __HAL_DMA_DISABLE_IT(dev->hdma, DMA_IT_HT);
}
/*清空环形缓冲区*/
void UART_Device_Clear(UART_Device *dev){
	memset(dev->UART_RING_BUF,0,RING_BUF_SIZE);
}
void UART_Device_Init(UART_Device *dev, UART_HandleTypeDef *huart, DMA_HandleTypeDef *hdma){
    memset(dev, 0, sizeof(UART_Device));
    dev->huart = huart;
    dev->hdma = hdma;

    dev->Init = UART_Device_Init;
    dev->Read = UART_Ring_Read;
    dev->Write = UART_Ring_Write;
    dev->Available = UART_Ring_Available;
	dev->Rx_callback = UART_Device_RxCallback;
	dev->Send = UART_Device_Send;
	dev->Clear = UART_Device_Clear;
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    HAL_UARTEx_ReceiveToIdle_DMA(huart, dev->RX_Data, RX_Data_SIZE);
    __HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT);
}

