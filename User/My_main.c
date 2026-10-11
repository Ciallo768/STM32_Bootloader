#include "My_main.h"
#define MAX_READ_LEN 133 // Xmodem 最大帧长
OTA_InfoCB OTA_Info;
UpDataA_CB UpDataA;
uint32_t BootStaflag;
int my_main(void){
	AT24C02 eeprom = AT24C02_Create(&hi2c2,0x50);
	W25Q64 flash = W25Q64_Create(&hspi1, GPIOA);
	UART_Device_Init(&uart1_dev, &huart1, &hdma_usart1_rx);
	uint8_t i = 0;
	//uint8_t ATdata[4] = {0};
	//uint8_t AT[4] = {0};//STM32 是小端存储，会自动把低位字节数据存在内存的低地址，无论是小端存储还是大端存储，在程序取用一个完整的多字节数据,CPU通常是根据该数据的起始地址（即最低地址）一次性读取所有需要的字节，并进行处理。
	//eeprom.WritePage(&eeprom,0x00,AT,4);
	//eeprom.ReadOTA(&eeprom,&OTA_Info);
	//1KB = 1024Byte (字节) 1MB = 1024KB
	OTA_Info.OTA_flag = 0x2211BBAA;
	for(i=0;i<8;i++){
	OTA_Info.Firelen[i] = i;
	}
	eeprom.WriteOTA(&eeprom);
	eeprom.ReadOTA(&eeprom,&OTA_Info);
	
	BootLoader_Branch();
	uint8_t recv_buf[MAX_READ_LEN]; 
    uint16_t read_bytes = 0;
	
	
while(1){
	HAL_Delay(10);
	
if(BootStaflag & IAP_XMODEMC_FLAG){
    // 每 50 * 10ms = 500ms 发送一次 C，且第一次进入立即发送（Bootloader_Event 已经发送一次）
    if (UpDataA.XmodemTimer >= 50) {
        SendChar('C');
        MyPrintf(">>> 发送 'C' 请求 XMODEM（重试）\r\n");
        UpDataA.XmodemTimer = 0;
    }
    UpDataA.XmodemTimer++;
}
	read_bytes = uart1_dev.Read(&uart1_dev, recv_buf, MAX_READ_LEN);

    if (read_bytes > 0) {
        // 将读取到的连续数据段分发给事件处理器
        Bootloader_Event(recv_buf, read_bytes);
    }

	
	
	
	
	//以下代码为UPDATA_A_FLAG置位，表明需要更新A区程序
	
	if(BootStaflag&UPDATA_A_FLAG){
	//A区更新
		MyPrintf("长度%d字节\r\n",OTA_Info.Firelen[UpDataA.W25Q64_BlockNB]);
		if(OTA_Info.Firelen[UpDataA.W25Q64_BlockNB]%4 ==0){
			// STM32F103的 Page Size 通常为 1024 字节
			STM32_EraseFlash(STM32_A_SPAGE,STM32_A_PAGE_NUM);
			
			// 1. 写入完整的 1024 字节页
			for(i=0;i<OTA_Info.Firelen[UpDataA.W25Q64_BlockNB]/STM32_PAGE_SIZE;i++){
				flash.read_data(&flash,i*1024 + UpDataA.W25Q64_BlockNB*64*1024,UpDataA.Updatabuff,STM32_PAGE_SIZE);
				// STM32_PAGE_SIZE/4 = 1024/4 = 256 字
				STM32_WriteFlash(STM32_FLASH_SADDR+i*STM32_PAGE_SIZE,(uint32_t *)UpDataA.Updatabuff,STM32_PAGE_SIZE/4);
			}
			
			// 2. 写入剩余的尾部数据 (修正了参数错误)
			if(OTA_Info.Firelen[UpDataA.W25Q64_BlockNB]%STM32_PAGE_SIZE !=0){
				// 计算剩余的字节数（必定是4的倍数，因为前面已经校验过总长度）
				uint32_t remaining_bytes = OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % STM32_PAGE_SIZE;
				uint32_t remaining_words = remaining_bytes / 4; // 转换为字数 (4字节)

				// 读取剩余字节
				flash.read_data(&flash,
								i*1024 + UpDataA.W25Q64_BlockNB*64*1024,
								UpDataA.Updatabuff,
								remaining_bytes);

				// 写入剩余字
				// 地址计算：i*STM32_PAGE_SIZE 正好指向所有完整页之后的起始地址
				STM32_WriteFlash(STM32_FLASH_SADDR + i * STM32_PAGE_SIZE,
								(uint32_t *)UpDataA.Updatabuff,
								remaining_words); // 修正：传入字数
			}
			
			// 3. 升级完成收尾
			if(UpDataA.W25Q64_BlockNB == 0){ 
				// 假设固件块总是从 BlockNB=0 开始，且整个升级流程只处理这一个块
				OTA_Info.OTA_flag = 0;
				eeprom.WriteOTA(&eeprom);
			}
			NVIC_SystemReset();
		}
		else{
			MyPrintf("长度错误");
			BootStaflag &= ~UPDATA_A_FLAG;
		}
	}
	
}

}
