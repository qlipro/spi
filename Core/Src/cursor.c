/*
 * cursor.c
 *
 *  Created on: 2026年3月5日
 *      Author: jt
 */

#include "cursor.h"
#include "LCD.h"
#include "spi.h"

// 向上滚动
void LCD_ScrollUp(uint8_t lines) {
    // 将屏幕内容向上移动 lines 行
    uint8_t char_height = lcd_ctx.font->h;
    uint16_t scroll_height = lines * char_height;

    // 从屏幕底部向上读取数据并移动到上部
    for (uint16_t y = scroll_height; y < lcd_ctx.height; y++) {
        // 这里需要实现从LCD读取数据的函数
        // 简化处理：直接清空最后一行
    }

    // 清空最后 lines 行
    set_window(0, lcd_ctx.height - scroll_height,
               X_MAX_PIXEL - 1, lcd_ctx.height - 1);

    uint8_t high = lcd_ctx.bg_color >> 8;
    uint8_t low = lcd_ctx.bg_color & 0xFF;
    uint8_t buffer[256];

    for (uint16_t i = 0; i < sizeof(buffer); i += 2) {
        buffer[i] = high;
        buffer[i + 1] = low;
    }

    uint32_t total_pixels = X_MAX_PIXEL * scroll_height;
    uint32_t remaining = total_pixels * 2;

    DC_SET1();
    CS_SET0();

    while (remaining > 0) {
        uint32_t send_size = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;
        HAL_SPI_Transmit(&hspi2, buffer, send_size, HAL_MAX_DELAY);
        remaining -= send_size;
    }

    CS_SET1();
}

// 设置光标位置
void LCD_SetCursor(uint8_t x, uint8_t y) {
    lcd_ctx.cursor_x = x;
    lcd_ctx.cursor_y = y;
}

// 获取当前光标位置
void LCD_GetCursor(uint8_t *x, uint8_t *y) {
    *x = lcd_ctx.cursor_x;
    *y = lcd_ctx.cursor_y;
}

// 设置自动换行
void LCD_SetAutoWrap(uint8_t enable) {
    lcd_ctx.auto_wrap = enable;
}

