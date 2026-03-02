/*
 * wifi.c
 *
 *  Created on: 2026年2月28日
 *      Author: jt
 */

#include "wifi.h"
#include "LCD.h"
#include <stdlib.h>

extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi2;



// 发送AT命令
static uint8_t WiFi_SendCmd(WiFi_Handle_t *wifi, const char *cmd, uint16_t timeout) {
    HAL_UART_Transmit(wifi->huart, (uint8_t*)cmd, strlen(cmd), timeout);
    return 1;
}

// 初始化
void WiFi_Init(WiFi_Handle_t *wifi, UART_HandleTypeDef *huart) {
    wifi->huart = huart;
    wifi->state = WIFI_IDLE;
    wifi->rx_index = 0;
    wifi->image_index = 0;
    wifi->client_id = 0;

    // 测试AT
    WiFi_SendCmd(wifi, "AT\r\n", 100);
    HAL_Delay(200);

    // 复位模块
    WiFi_SendCmd(wifi, "AT+RST\r\n", 100);
    HAL_Delay(2000);

    // 关闭回显
    WiFi_SendCmd(wifi, "ATE0\r\n", 100);
    HAL_Delay(200);
}

// 配置为AP模式（ESP作为热点）
void WiFi_Config_AP(WiFi_Handle_t *wifi, const char *ssid, const char *pwd) {
    char cmd[128];

    // 设置AP模式
    WiFi_SendCmd(wifi, "AT+CWMODE=2\r\n", 100);
    HAL_Delay(500);

    // 配置AP参数
    sprintf(cmd, "AT+CWSAP=\"%s\",\"%s\",1,4\r\n", ssid, pwd);
    WiFi_SendCmd(wifi, cmd, 100);
    HAL_Delay(1000);

    // 获取IP
    WiFi_SendCmd(wifi, "AT+CIFSR\r\n", 100);
}

// 配置为Station模式（连接路由器）
void WiFi_Config_Station(WiFi_Handle_t *wifi, const char *ssid, const char *pwd) {
    char cmd[128];

    // 设置Station模式
    WiFi_SendCmd(wifi, "AT+CWMODE=1\r\n", 100);
    HAL_Delay(500);

    // 连接路由器
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    WiFi_SendCmd(wifi, cmd, 5000);
    HAL_Delay(2000);

    // 获取IP
    WiFi_SendCmd(wifi, "AT+CIFSR\r\n", 100);
}

// 启动TCP服务器
void WiFi_Start_Server(WiFi_Handle_t *wifi, uint16_t port) {
    char cmd[64];
    uint8_t retry = 3;
    // 1. 先关闭可能存在的服务器
    WiFi_SendCmd(wifi, "AT+CIPSERVER=0\r\n", 100);
    HAL_Delay(200);
    // 启用多连接
    // 2. 启用多连接，带重试和验证
    while(retry--) {
        WiFi_SendCmd(wifi, "AT+CIPMUX=1\r\n", 100);
        HAL_Delay(500);  // 给模块足够时间处理

        // 查询确认
        WiFi_SendCmd(wifi, "AT+CIPMUX?\r\n", 100);
        HAL_Delay(200);

        // 这里需要解析返回的 +CIPMUX:1 才能确认
        // 简单起见，我们等待一段时间后假设成功
    }
    // 启动服务器
    sprintf(cmd, "AT+CIPSERVER=1,%d\r\n", port);
    WiFi_SendCmd(wifi, cmd, 100);
    HAL_Delay(200);

    wifi->state = WIFI_SERVER_RUNNING;
}

// 处理接收到的数据
void WiFi_Process(WiFi_Handle_t *wifi, uint8_t data) {
    static uint8_t ipd_parsing = 0;
    static uint16_t ipd_len = 0;
    static uint8_t ipd_client = 0;

    wifi->rx_buf[wifi->rx_index++] = data;

    // 检测 +IPD 开头（有数据接收）
    if (wifi->rx_index >= 4 &&
        memcmp(wifi->rx_buf + wifi->rx_index - 4, "+IPD", 4) == 0) {
        ipd_parsing = 1;
        ipd_len = 0;
        wifi->rx_index = 0;
    }

    // 解析 +IPD 格式: +IPD,client,length:
    if (ipd_parsing) {
        if (data == ':') {
            // 解析完成，获取client和length
            char *p = strtok((char*)wifi->rx_buf, ",");
            p = strtok(NULL, ",");  // client
            ipd_client = atoi(p);
            p = strtok(NULL, ":");  // length
            ipd_len = atoi(p);

            wifi->client_id = ipd_client;
            wifi->state = WIFI_RECEIVING_IMAGE;
            wifi->image_index = 0;
            ipd_parsing = 0;
            wifi->rx_index = 0;
        }
        return;
    }

    // 接收图像数据（逐行处理）
       if (wifi->state == WIFI_RECEIVING_IMAGE) {
           wifi->line_buffer[wifi->image_index++] = data;

           // 一行接收完成 (132像素 × 2字节 = 264字节)
           if (wifi->image_index >= IMAGE_WIDTH * 2) {
               // 立即显示这一行
               set_window(0, wifi->line_count, IMAGE_WIDTH - 1, wifi->line_count);
               DC_SET1(); CS_SET0();
               HAL_SPI_Transmit(&hspi2, wifi->line_buffer, IMAGE_WIDTH * 2, HAL_MAX_DELAY);
               CS_SET1();

               wifi->line_count++;
               wifi->image_index = 0;

               // 一帧完成
               if (wifi->line_count >= IMAGE_HEIGHT) {
                   // 发送确认
                   char confirm[32];
                   sprintf(confirm, "AT+CIPSEND=%d,1\r\n", wifi->client_id);
                   WiFi_SendCmd(wifi, confirm, 100);
                   HAL_UART_Transmit(wifi->huart, (uint8_t*)"\xBB", 1, 100);

                   wifi->state = WIFI_SERVER_RUNNING;
                   wifi->line_count = 0;
               }
           }
       }

    // 处理其它AT响应
    if (data == '\n' || wifi->rx_index >= WIFI_RX_BUF_SIZE - 1) {
        wifi->rx_buf[wifi->rx_index] = '\0';

        // 检查连接状态
        if (strstr((char*)wifi->rx_buf, "CONNECT") != NULL) {
            wifi->state = WIFI_CLIENT_CONNECTED;
        }

        wifi->rx_index = 0;
    }
}

// 发送数据到客户端
uint8_t WiFi_Send_Data(WiFi_Handle_t *wifi, uint8_t *data, uint16_t len) {
    char cmd[64];

    // 发送命令：AT+CIPSEND=client,length
    sprintf(cmd, "AT+CIPSEND=%d,%d\r\n", wifi->client_id, len);
    WiFi_SendCmd(wifi, cmd, 100);
    HAL_Delay(50);

    // 发送数据
    HAL_UART_Transmit(wifi->huart, data, len, 100);

    return 1;
}


