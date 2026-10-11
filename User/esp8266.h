#ifndef ESP8266_H
#define ESP8266_H
#include "usart.h"
#include "printf.h"
#include "string.h"
#include "stdio.h"
#include "stm32f1xx_hal.h"
typedef void (*ConnectionCallback)(const char *message);
typedef struct _ESP8266{
    UART_Device *uart;
    
    // 连接状态标志
    uint8_t wifi_connected;
    uint8_t tcp_connected;
    
    // 状态回调函数
    ConnectionCallback status_callback;
    
    // 方法函数指针
	int (*Send_Cmd)(struct _ESP8266 *self,const char *cmd,const char *expect,uint32_t timeout);
	int (*Wifi_Connect)(struct _ESP8266 *self,const char *ssid,const char *password);
	int (*Tcp_Connect)(struct _ESP8266 *self, const char *ip, uint16_t port);
	int (*Tcp_Send)(struct _ESP8266 *self, const uint8_t *data, uint16_t len);
} ESP8266;
void ESP8266_Init(ESP8266 *self,UART_Device *uart,ConnectionCallback callback);
#endif
