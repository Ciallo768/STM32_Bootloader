#include "bootloader.h"

#define APP_START_ADDRESS         0x08005000U
// APP 向量表的偏移量 (0x08005000 - 0x08000000 = 0x5000)
#define APP_VECT_TAB_OFFSET       0x5000U
typedef void (*pFunction)(void);


void Jump_To_Application(void)
{
    uint32_t AppStackPointer = *((uint32_t *)APP_START_ADDRESS);
    uint32_t AppEntryAddress = *((uint32_t *)(APP_START_ADDRESS + 4));
    pFunction AppEntry = (pFunction)AppEntryAddress;

    Bootloader_DeInit();    // 外设关闭
    __disable_irq();        // 关闭全局中断

    SCB->VTOR = APP_START_ADDRESS;
    __DSB();
    __ISB();

    __set_MSP(AppStackPointer);
    __set_CONTROL(0);

    __enable_irq();         // 再开中断
    AppEntry();             // 跳转
}


void BootLoader_Branch(){
	if(Bootloader_Enter(20)==0){
	
	if(OTA_Info.OTA_flag == OTA_SET_FLAG){
		MyPrintf("OTA update!\r\n");
		BootStaflag |= UPDATA_A_FLAG;
		UpDataA.W25Q64_BlockNB = 0;
	}else{
		MyPrintf("go to A\r\n");
		Jump_To_Application();
	}
	}
	else{
		MyPrintf("进入Bootloader命令行\r\n");
		Bootloader_Info();
	}

	
}

void Bootloader_DeInit(){
	
	SysTick->CTRL = 0; // 关闭 SysTick 计时器
    SysTick->LOAD = 0; // 清除重载值
    SysTick->VAL = 0;  // 清除当前值
	// 清理 I2C (用于 EEPROM)
	HAL_I2C_DeInit(&hi2c2); 
	
	// 清理 SPI (用于 Flash)
	HAL_SPI_DeInit(&hspi1);
	
	// 清理 UART1 及 DMA (用于打印)
	// 注意：UART DMA 必须先于 UART DeInit
	HAL_DMA_DeInit(huart1.hdmarx); // 清理 RX DMA 句柄 (如果定义了)
	HAL_DMA_DeInit(huart1.hdmatx); // 清理 TX DMA 句柄 (如果定义了)
	HAL_UART_DeInit(&huart1);	
	// 重置时钟：关闭所有用过的外设时钟
	// 这样可以确保 App 启动时，这些外设是干净的。
	__HAL_RCC_I2C2_FORCE_RESET();
	__HAL_RCC_I2C2_RELEASE_RESET();
	__HAL_RCC_SPI1_FORCE_RESET();
	__HAL_RCC_SPI1_RELEASE_RESET();
	__HAL_RCC_USART1_FORCE_RESET();
	__HAL_RCC_USART1_RELEASE_RESET();
	
	
}


uint8_t Bootloader_Enter(uint8_t timeout){
	uint8_t c; // 用于存储读取到的字符
	MyPrintf("%dms内,输入小写字母w,进入bootloader命令行\r\n",timeout);
	while(timeout--){
		HAL_Delay(100);
		// 使用 Read 从环形缓冲区中读取并消耗数据
		if(uart1_dev.Read(&uart1_dev, &c, 1) == 1){
            if(c == 'w'){
                return 1;//进入命令行
            }
		}
	}
	return 0;
}
void Bootloader_Info(void){
	
	MyPrintf("\r\n");
	MyPrintf("[1]擦除A区\r\n");
	MyPrintf("[2]串口IAP下载A区程序\r\n");
	MyPrintf("[3]设置OTA版本号\r\n");
	MyPrintf("[4]查询OTA版本号\r\n");
	MyPrintf("[5]向外部Flash下载程序\r\n");
	MyPrintf("[6]从外部Flash内下载程序\r\n");
	MyPrintf("[7]重启\r\n");

}

void Bootloader_Event(uint8_t *data,uint16_t datalen){
	if(BootStaflag == 0){
		if((datalen==1)&&(data[0] == '1')){
		MyPrintf("擦除A区\r\n");
		STM32_EraseFlash(STM32_A_SADDR,STM32_A_PAGE_NUM);
		NVIC_SystemReset();
	}
else if((datalen==1)&&(data[0] == '2')){
    MyPrintf("通过Xmodem协议，串口IAP下载A区程序，请使用bin格式文件\r\n");
    STM32_EraseFlash(STM32_A_SADDR,STM32_A_PAGE_NUM);
    BootStaflag |= IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG;
    UpDataA.XmodemTimer = 0;
    UpDataA.XmodemNB = 0;

    // 立即发送 'C' 要求发送端进入 XMODEM-CRC 传输（更可靠触发发送端）
    SendChar('C');
    MyPrintf("已发送初始 'C' 请求 XMODEM (CRC)\r\n");
}
else if((datalen==1)&&(data[0] == '3')){
	
	MyPrintf("设置OTA版本号\r\n");
	BootStaflag |= SET_OTAVER_FLAG;
	
	}
	else if((datalen==1)&&(data[0] == '7')){
		MyPrintf("重启\r\n");
		HAL_Delay(100);
		NVIC_SystemReset();
	}
	
	}
	
if (BootStaflag & IAP_XMODEMD_FLAG) {
    
    // 1. 处理 Xmodem 数据包 (SOH = 0x01, 长度 133 字节)
    if (datalen == 133 && data[0] == 0x01) {
        
        // 关键修正：如果是第一个包，清除发送 'C' 的标志
        if (BootStaflag & IAP_XMODEMC_FLAG) {
            BootStaflag &= ~IAP_XMODEMC_FLAG; // 停止重发 'C'
            MyPrintf("Xmodem 传输开始。\r\n");
        }
        
        // 执行 CRC 校验
        UpDataA.XmodemCRC = Xmodem_CRC_16(&data[3], 128); 
        
        // CRC 校验成功
        if (UpDataA.XmodemCRC == (uint16_t)(data[131] << 8 | data[132])) {
            uint8_t ack = 0x06;
            HAL_UART_Transmit(&huart1, &ack, 1, 20); // 发送 ACK (0x06)
            UpDataA.XmodemNB++;
            
            // 将数据复制到缓冲区
            memcpy(UpDataA.Updatabuff + ((UpDataA.XmodemNB - 1) % (STM32_PAGE_SIZE / 128)) * 128, &data[3], 128);

            // 当缓冲区满一页时，写入 Flash(缓冲区达到1024个byte)(1KB)
            if ((UpDataA.XmodemNB % (STM32_PAGE_SIZE / 128)) == 0) {
                // 计算写入的 Flash 地址
                uint32_t write_addr = STM32_A_SADDR + (UpDataA.XmodemNB / (STM32_PAGE_SIZE / 128) - 1) * STM32_PAGE_SIZE;
                // 写入 STM32_PAGE_SIZE/4 个字 (Word)
                STM32_WriteFlash(write_addr, (uint32_t *)&UpDataA.Updatabuff, STM32_PAGE_SIZE / 4);             
            }
        } else {
            // CRC 校验失败
            uint8_t nak = 0x15;
            HAL_UART_Transmit(&huart1, &nak, 1, 20); // 发送 NAK (0x15)，请求重发
            MyPrintf("Xmodem CRC 校验失败，请求重发。\r\n");
        }
    }
    
    // 2. 处理 EOT (传输结束，0x04)
    else if (datalen == 1 && data[0] == 0x04) {
        SendChar(0x06); // 发送 ACK (0x06) 确认 EOT
        MyPrintf("Xmodem EOT 收到，传输结束。\r\n");
        
        // 写入剩余未满页的数据
        if ((UpDataA.XmodemNB % (STM32_PAGE_SIZE / 128)) != 0) {
            // 计算剩余的字节数
            uint32_t remaining_bytes = (UpDataA.XmodemNB % (STM32_PAGE_SIZE / 128)) * 128;
            // 计算写入的 Flash 地址
            uint32_t write_addr = STM32_A_SADDR + (UpDataA.XmodemNB / (STM32_PAGE_SIZE / 128)) * STM32_PAGE_SIZE;
            
            // 写入剩余的字数 (remaining_bytes / 4)
            STM32_WriteFlash(write_addr, (uint32_t *)&UpDataA.Updatabuff, remaining_bytes / 4); 
        }
        
        BootStaflag &= ~IAP_XMODEMD_FLAG; // 清除传输标志
        HAL_Delay(100);
        NVIC_SystemReset();
    }
    
    // 3. 处理意外情况（如 CAN 0x18）
    else if (datalen == 1 && data[0] == 0x18) {
        BootStaflag &= ~(IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG);
        MyPrintf("Xmodem 收到 CAN，传输被取消。\r\n");
    }
}
 

}
uint16_t Xmodem_CRC_16(uint8_t *data,uint16_t datlen){
	uint8_t i;
	uint16_t Crcinit = 0x0000;
	uint16_t Crcipoly = 0x1021;
	while(datlen--){
		Crcinit = (*data<<8)^Crcinit;
		for(i=0;i<8;i++){
			if(Crcinit & 0x8000)

				Crcinit = (Crcinit<<1)^Crcipoly;
			else 
				Crcinit = (Crcinit<<1);
		}
		data++;
	}
	
	return Crcinit;
}
