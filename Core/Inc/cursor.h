/*
 * cursor.h
 *
 *  Created on: 2026年3月5日
 *      Author: jt
 */

#ifndef INC_CURSOR_H_
#define INC_CURSOR_H_

#include "LCD.h"

#define X_MAX_PIXEL	        128
#define Y_MAX_PIXEL	        160

// 显示上下文结构体
typedef struct {
    uint8_t cursor_x;           // 当前光标X坐标
    uint8_t cursor_y;           // 当前光标Y坐标
    uint8_t line_height;        // 行高
    uint16_t text_color;        // 文字颜色
    uint16_t bg_color;          // 背景颜色
    uint8_t width;              // 显示区域宽度
    uint8_t height;             // 显示区域高度
    const ASCIIFont *font;      // 当前字体
    uint8_t auto_wrap;          // 自动换行标志
    uint8_t start_y;            // 起始Y坐标
    uint8_t line_count;         // 行数
    uint16_t *line_buffer;  // 指向行缓冲区的指针
    uint8_t buffer_rows;    // 缓冲区行数
    uint8_t buffer_head;    // 当前显示的起始行在缓冲区中的索引
} LCD_Context;

static LCD_Context lcd_ctx = {
    .cursor_x = 0,
    .cursor_y = 0,
    .line_height = 16,
    .text_color = 0xFFFF,       // 白色
    .bg_color = 0x0000,          // 黑色
    .width = X_MAX_PIXEL-16,
    .height = Y_MAX_PIXEL-32,
    .font = NULL,
    .auto_wrap = 1
};

void LCD_ScrollUp(uint8_t lines);
void LCD_SetCursor(uint8_t x, uint8_t y);
void LCD_GetCursor(uint8_t *x, uint8_t *y);

#endif /* INC_CURSOR_H_ */
