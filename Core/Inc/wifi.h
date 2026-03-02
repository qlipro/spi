/*
 * wifi.h
 *
 *  Created on: 2026年2月28日
 *      Author: jt
 */

#ifndef INC_WIFI_H_
#define INC_WIFI_H_

#include "main.h"
#include <string.h>
#include <stdio.h>

#define WIFI_RX_BUF_SIZE 2048
#define IMAGE_WIDTH  132
#define IMAGE_HEIGHT 162
//#define IMAGE_SIZE   (IMAGE_WIDTH * IMAGE_HEIGHT * 2)

typedef enum {
    WIFI_IDLE = 0,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_SERVER_RUNNING,
    WIFI_CLIENT_CONNECTED,
    WIFI_RECEIVING_IMAGE
} WiFi_State_t;

// wifi.h 中添加
typedef struct {
    uint8_t buffer[2][IMAGE_WIDTH * 2];  // 双缓冲，每行264字节
    volatile uint8_t active_buf;          // 当前使用的缓冲区(0或1)
    volatile uint8_t dma_busy;            // DMA传输进行中
    volatile uint8_t line_ready[2];        // 缓冲区行数据就绪标志
    volatile uint16_t current_line;        // 当前正在显示的行号
    void (*line_done_callback)(uint16_t line); // 行完成回调
} LineDMA_t;

typedef struct {
    UART_HandleTypeDef *huart;
    WiFi_State_t state;
    uint8_t rx_buf[WIFI_RX_BUF_SIZE];
    uint16_t rx_index;
    uint8_t line_buffer[IMAGE_WIDTH * 2];  // 仅264字节 ✅
	uint16_t line_count;
//    uint8_t *image_buffer;
    uint16_t image_index;
    uint8_t client_id;
    LineDMA_t line_dma;                    // 新增：行DMA控制
    uint8_t frame_complete;                 // 帧完成标志
} WiFi_Handle_t;




void WiFi_Init(WiFi_Handle_t *wifi, UART_HandleTypeDef *huart);
void WiFi_Config_AP(WiFi_Handle_t *wifi, const char *ssid, const char *pwd);
void WiFi_Config_Station(WiFi_Handle_t *wifi, const char *ssid, const char *pwd);
void WiFi_Start_Server(WiFi_Handle_t *wifi, uint16_t port);
void WiFi_Process(WiFi_Handle_t *wifi, uint8_t data);
uint8_t WiFi_Send_Data(WiFi_Handle_t *wifi, uint8_t *data, uint16_t len);
//void WiFi_Diagnostic(WiFi_Handle_t *wifi);
#endif /* INC_WIFI_H_ */
