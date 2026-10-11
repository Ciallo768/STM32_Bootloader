#include "printf.h"
#include <stdarg.h>  
#include "usart.h"  
#include "stdio.h"
#include "stm32f1xx_hal_uart.h"
//void SendChar(char ch) {
//    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)&ch, 1);
//}
extern UART_HandleTypeDef huart1;
void SendChar(char ch) {
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 20); 
}


void SendString(const char *str) {
    while (*str) {
        SendChar(*str++);
    }
}
void SendHex(unsigned int num) {
    char buf[9]; // 32位整数最多8位十六进制 + '\0'
    sprintf(buf, "%X", num);  // %X 大写十六进制，%x 小写十六进制
    SendString(buf);
}

void SendNumber(int num) {
    char buf[12];
    sprintf(buf, "%d", num);  // 或者你手写 itoa
    SendString(buf);
}

void MyPrintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0') {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    int val = va_arg(args, int);
                    SendNumber(val);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char *);
                    SendString(str);
                    break;
                }
                case 'x':
                case 'X': {
                    unsigned int val = va_arg(args, unsigned int);
                    SendHex(val);
                    break;
                }
                default:
                    SendChar(*fmt);
            }
        } else {
            SendChar(*fmt);
        }
        fmt++;
    }

    va_end(args);
}