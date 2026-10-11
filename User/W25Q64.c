#include "W25Q64.h"
#define W25Q64_CMD_READ_ID 0x9F //读 JEDEC ID​	读取芯片的制造商ID、内存类型和容量ID（3字节）
#define W25Q64_CMD_READ_DATA 0x03 //​读取数据 从指定地址读取数据（可连续读取）
#define W25Q64_CMD_PAGE_PROGRAM 0x02 //​页编程 向指定地址写入数据（最多256字节）
#define W25Q64_CMD_SECTOR_ERASE 0x20 //擦除4KB的扇区（擦除后全为0xFF）
#define W25Q64_CMD_CHIP_ERASE 0xC7 //​整片擦除
#define W25Q64_CMD_READ_STATUS 0x05 //​读状态寄存器​	读取状态寄存器（判断忙状态、写使能等）
#define W25Q64_CMD_WRITE_ENABLE 0x06 //​写使能​	允许写入或擦除操作（临时生效）

#define W25Q64_PAGE_SIZE         256 //字节
#define W25Q64_SECTOR_SIZE       4096 //字节

void CS_LOW(W25Q64 *self){
	HAL_GPIO_WritePin(self->CS_Port,GPIO_PIN_4,GPIO_PIN_RESET);
}

void CS_HIGH(W25Q64 *self){
	HAL_GPIO_WritePin(self->CS_Port,GPIO_PIN_4,GPIO_PIN_SET);
}

//写使能

void write_enable(W25Q64 *self){
	uint8_t cmd = W25Q64_CMD_WRITE_ENABLE;
	self->cs_low(self);
	HAL_SPI_Transmit(self->hspi,&cmd,1,100);
	self->cs_high(self);
}

//等待写完成
void wait_busy(W25Q64 *self){
    uint8_t cmd = W25Q64_CMD_READ_STATUS;
    uint8_t status = 0;
    uint8_t dummy = 0xFF;
    do {
        self->cs_low(self);
        HAL_SPI_Transmit(self->hspi, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_TransmitReceive(self->hspi, &dummy, &status, 1, HAL_MAX_DELAY);
        self->cs_high(self);
        HAL_Delay(1);
    } while (status & 0x01);
}

//读JEDEC ID 

uint32_t read_id(W25Q64 *self){
uint8_t tx_buf[4] = {W25Q64_CMD_READ_ID, 0xFF, 0xFF, 0xFF};
uint8_t rx_buf[4] = {0};

self->cs_low(self);
HAL_SPI_TransmitReceive(self->hspi, tx_buf, rx_buf, 4, 100);
self->cs_high(self);

self->device_id = (rx_buf[1] << 16) | (rx_buf[2] << 8) | rx_buf[3];
}
//读数据
void read_data(W25Q64 *self, uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd[4];
    cmd[0] = W25Q64_CMD_READ_DATA;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    self->cs_low(self);
    HAL_SPI_Transmit(self->hspi, cmd, 4, HAL_MAX_DELAY);
	    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t dummy = 0xFF;
        HAL_SPI_TransmitReceive(self->hspi, &dummy, &buf[i], 1, HAL_MAX_DELAY);
    }
    self->cs_high(self);
}

// 页写（最多256字节）
void page_program(W25Q64 *self, uint32_t addr, uint8_t *buf, uint16_t len)
{
    if (len > W25Q64_PAGE_SIZE) len = W25Q64_PAGE_SIZE;
    self->write_enable(self);

    uint8_t cmd[4];
    cmd[0] = W25Q64_CMD_PAGE_PROGRAM;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    self->cs_low(self);
    HAL_SPI_Transmit(self->hspi, cmd, 4, 100);
    HAL_SPI_Transmit(self->hspi, buf, len, 1000);
    self->cs_high(self);

    self->wait_busy(self);
}

// 擦除4KB扇区
void sector_erase(W25Q64 *self, uint32_t addr)
{
    self->write_enable(self);
    uint8_t cmd[4];
    cmd[0] = W25Q64_CMD_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    self->cs_low(self);
    HAL_SPI_Transmit(self->hspi, cmd, 4, 100);
    self->cs_high(self);

    self->wait_busy(self);
}

// 构造函数
W25Q64 W25Q64_Create(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_Port)
{
    W25Q64 dev;
    dev.hspi = hspi;
    dev.CS_Port = CS_Port;
    dev.device_id = 0;

    dev.cs_low       = CS_LOW;
    dev.cs_high      = CS_HIGH;
    dev.write_enable = write_enable;
    dev.wait_busy    = wait_busy;
    dev.read_id      = read_id;
    dev.read_data    = read_data;
    dev.page_program = page_program;
    dev.sector_erase = sector_erase;

    dev.cs_high(&dev); // 初始化拉高CS
    return dev;
}
