#ifndef MY_MAIN_H
#define MY_MAIN_H
int my_main(void);
#include "stdint.h"
#include "usart.h"
#include "stdio.h"
#include "oled.h"
#include "font.h"
#include "AT24C02.h"
#include "W25Q64.h"
#include "printf.h"
#include "flash.h"
#include "esp8266.h"
#include "bootloader.h"
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

extern SPI_HandleTypeDef hspi1;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

extern UART_Device uart1_dev;

#define STM32_FLASH_SADDR 0x8000000 //Flash 起始地址
#define STM32_PAGE_SIZE 1024 //flash 扇区大小(字节)
#define STM32_FLASH_NUM 64 //flash 总扇区个数
#define STM32_B_PAGE_NUM 20// B区 扇区个数
#define STM32_A_PAGE_NUM STM32_FLASH_NUM - STM32_B_PAGE_NUM // A区 扇区个数
#define STM32_A_SPAGE STM32_B_PAGE_NUM //A区起始扇区编号
#define STM32_A_SADDR STM32_FLASH_SADDR + STM32_B_PAGE_NUM * STM32_PAGE_SIZE //A区 起始地址
#define OTA_SET_FLAG 0xAABB1122
#define UPDATA_A_FLAG 0x00000001
#define IAP_XMODEMC_FLAG 0x00000002//发送C的状态标志
#define IAP_XMODEMD_FLAG 0x00000004//Xmodem 传输正在进行标志位
#define SET_OTAVER_FLAG 0x00000008//设置OTA版本号标志位
//每 2 个十六进制数字 对应 1 个字节（8 位）
typedef struct _OTA_InfoCB{
	uint32_t OTA_flag;
	uint32_t Firelen[11]; //0号成员对应OTA的大小
	uint8_t OTA_ver[32];	
}OTA_InfoCB;
typedef struct {
	uint8_t Updatabuff[STM32_PAGE_SIZE];//更新A区时，保存W25Q64中的数据
	uint32_t W25Q64_BlockNB;//一个固件块为64KB W25Q64的最大物理块为64KB,用于记录从哪个W25Q64的块中读取数据
	uint32_t XmodemTimer;//简易延时变量
	uint32_t XmodemNB;//用于保存接收的数据包的个数
	uint32_t XmodemCRC;
}UpDataA_CB;

#define OTA_INFOCB_SIZE  sizeof(OTA_InfoCB);
extern OTA_InfoCB OTA_Info;
extern UpDataA_CB UpDataA;
extern uint32_t BootStaflag;
#endif

