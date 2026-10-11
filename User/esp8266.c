#include "esp8266.h"
#define DEBUG_PRINT(...) do { char dbg_buf[128]; snprintf(dbg_buf, sizeof(dbg_buf), __VA_ARGS__); self->status_callback(dbg_buf); } while(0)

static int esp_send_cmd(ESP8266 *self,const char *cmd,const char *expect,uint32_t timeout){
	char recv_buf[512] = {0};
	uint32_t start = HAL_GetTick();
	//发送命令
	if(self->uart && self->uart->Send){
        self->uart->Send(self->uart,(const uint8_t *)cmd, (uint16_t)strlen(cmd));
    } else {
        // 兜底：直接通过 HAL 发送
        if(self->status_callback) self->status_callback("? Warning: uart->Send not available, using HAL directly");
        HAL_UART_Transmit(self->uart->huart, (uint8_t *)cmd, (uint16_t)strlen(cmd), 200);
    }
	if(self->status_callback){
		DEBUG_PRINT("Sending: %s",cmd);
	}
	//等待期望接收到的数据
	while(HAL_GetTick()-start < timeout){
		if(self->uart->Available(self->uart)>0){
			uint16_t len = self->uart->Read(self->uart,(uint8_t *)recv_buf,sizeof(recv_buf) - 1);
			recv_buf[len] = '\0';
			if(self->status_callback){
				DEBUG_PRINT("Received: %s",recv_buf);
			}
			
			if(strstr(recv_buf,"WIFI CONNECTED")){
				self->wifi_connected = 1;
				if(self->status_callback){
					DEBUG_PRINT("? WiFi connected!");
				}
			}
			else if(strstr(recv_buf,"WIFI DISCONNECT")){
				self->wifi_connected = 0;
				self->tcp_connected = 0;
				if(self->status_callback){
					DEBUG_PRINT("? WiFi disconnected!");
				}
			}
			else if(strstr(recv_buf,"CONNECT")){
				self->tcp_connected = 1;
				if(self->status_callback){
					DEBUG_PRINT("? TCP connected!");
				}
			}
			else if(strstr(recv_buf, "CLOSED")){
			self->tcp_connected = 0;
			if(self->status_callback){
				DEBUG_PRINT("? TCP disconnected!");
			}
			}
			if(strstr(recv_buf,expect)){
			return 0;
		}	
		}
	HAL_Delay(10);
	}
	if (self->status_callback) {
        DEBUG_PRINT("? Command timeout or FAIL: %s", cmd);
    }
    return -1; 
	
}
//流程 ：发送状态回调 发送命令

static int esp_connected_wifi(ESP8266 *self,const char *ssid,const char *password){
	if(self->status_callback){
		DEBUG_PRINT("Connecting to WIFI:%s",ssid);
	}
	char cmd[128];
	snprintf(cmd,sizeof(cmd),"AT+CWJAP=\"%s\",\"%s\"",ssid,password);
	int result = esp_send_cmd(self,cmd,"OK",15000);
	if(result == 0){
		self->wifi_connected =1;
		DEBUG_PRINT(">>> WiFi connection SUCCESS!");
	}
	else{
		self->wifi_connected = 0;
        DEBUG_PRINT("!!! WiFi connection FAILED!");
	}
	return result;
}
/* TCP 连接 */
static int esp_tcp_connect(ESP8266 *self, const char *ip, uint16_t port) {
    if (self->status_callback) {
        DEBUG_PRINT("Connecting to TCP: %s:%d...", ip, port);
    }
    
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ip, port);
    
    int result = esp_send_cmd(self, cmd, "OK", 5000);
    
    if (result == 0) {
        self->tcp_connected = 1;
        DEBUG_PRINT(">>> TCP connection SUCCESS!");
    } else {
        self->tcp_connected = 0;
        DEBUG_PRINT("!!! TCP connection FAILED!");
    }
    return result;
}

/* TCP 发送 */
static int esp_tcp_send(ESP8266 *self, const uint8_t *data, uint16_t len) {
    if (!self->tcp_connected) {
        DEBUG_PRINT("!!! TCP not connected. Send aborted.");
        return -1;
    }
    
    DEBUG_PRINT("Sending data (%d bytes)...", len);
    
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d\r\n", len);
    
    // 1. 发送 CIPSEND 命令
    if (esp_send_cmd(self, cmd, ">", 3000) != 0) return -1;
    
    // 2. 发送实际数据
    self->uart->Write(self->uart, (uint8_t *)data, len); 
    
    // 3. 等待 SEND OK
    int result = esp_send_cmd(self, "", "SEND OK", 5000);
    
    if (result == 0) {
        DEBUG_PRINT(">>> Data sent successfully!");
    } else {
        DEBUG_PRINT("!!! Data send FAILED!");
    }
    return result;
}

void ESP8266_Init(ESP8266 *self,UART_Device *uart,ConnectionCallback callback){
	memset(self,0,sizeof(ESP8266));
	self->uart = uart;
	self->status_callback = callback;
	
	self->Send_Cmd = esp_send_cmd;
	self->Wifi_Connect = esp_connected_wifi;
	self->Tcp_Connect = esp_tcp_connect;
	self->Tcp_Send = esp_tcp_send;
	
	if(self->status_callback){
		DEBUG_PRINT("Initializing ESP8266...");
	}
	// 基础初始化指令
    self->Send_Cmd(self, "AT\r\n", "OK", 1000);
    self->Send_Cmd(self, "ATE0\r\n", "OK", 1000); // 关闭回显
    self->Send_Cmd(self, "AT+CWMODE=1\r\n", "OK", 1000); // 设置为 STA 模式
	
	if(self->status_callback){
		DEBUG_PRINT("? ESP8266 initialization complete.");
	}
}

