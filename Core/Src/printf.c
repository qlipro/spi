/*
 * printf.c
 *
 *  Created on: 2026年3月5日
 *      Author: jt
 */

#include "printf.h"
#include "LCD.h"
#include "cursor.h"
#include "spi.h"
#include <stdarg.h>
#include <stdio.h>

void LCD_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg_color, const ASCIIFont *font) {
    if (!font) return;

    uint8_t char_width = font->w;
    uint8_t char_height = font->h;
    uint8_t bytes_per_char = char_height;

    // 处理不可见字符
    if (ch < 32 || ch > 126) {
        if (ch != '\n' && ch != '\r' && ch != '\b' && ch != '\t') {
            ch = '?';
        } else {
            return;  // 控制字符由上层函数处理
        }
    }

    // 设置显示窗口
    set_window(x, y, x + char_width - 1, y + char_height - 1);

    // 获取字模数据指针
    uint8_t (*font_data)[bytes_per_char] = (uint8_t (*)[bytes_per_char])font->chars;

    uint8_t high = color >> 8;
    uint8_t low = color & 0xFF;
    uint8_t bg_high = bg_color >> 8;
    uint8_t bg_low = bg_color & 0xFF;

    uint8_t buffer[256];  // 足够存放一个字符的像素数据
    uint16_t idx = 0;

    for (uint8_t row = 0; row < char_height; row++) {
        for (uint8_t col = 0; col < char_width; col++) {
            uint8_t row_data = font_data[ch - 32][row];
            uint8_t bit_mask = (char_width == 6) ? (0x40 >> col) : (0x80 >> col);

            if (row_data & bit_mask) {
                buffer[idx++] = high;
                buffer[idx++] = low;
            } else {
                buffer[idx++] = bg_high;
                buffer[idx++] = bg_low;
            }
        }
    }

    // 发送数据
    DC_SET1();
    CS_SET0();
    HAL_SPI_Transmit(&hspi2, buffer, idx, HAL_MAX_DELAY);
    CS_SET1();
}

/* @brief 初始化LCD文本框
 * @parameter y 起始位置
 * @parameter count_of_line 显示区行数
 * @parameter font 字体
 * @parameter text_color 文本颜色
 * @parameter bg_color 背景颜色
 */
void LCD_InitContext(uint8_t y, uint8_t count_of_line, const ASCIIFont *font, uint16_t text_color, uint16_t bg_color) {
    uint8_t h = count_of_line*font->h;
    uint8_t w = X_MAX_PIXEL-16;
    if(h>96){h=96;}

    lcd_ctx.font = font;
    lcd_ctx.text_color = text_color;
    lcd_ctx.bg_color = bg_color;
    lcd_ctx.line_height = font->h;
    lcd_ctx.cursor_x = 12;
    lcd_ctx.cursor_y = y;
    lcd_ctx.start_y = y;
    lcd_ctx.line_count = count_of_line;
    lcd_ctx.height = h;
    lcd_ctx.auto_wrap = 1;

    lcd_ctx.bg_height = h;

    LCD_ClearArea(6, y-2, w+4, h+4, WHITE);
    LCD_ClearArea(X_MAX_PIXEL-7,y-1,1,h+2,GRAY1);
    LCD_ClearArea(6,y-2,1,h+4,GRAY0);
    LCD_ClearArea(7,y+1+h,w+3,1,GRAY2);
    LCD_ClearArea(8, y, w, h, bg_color);
}

// 清除从当前光标到行尾的内容
void LCD_ClearToEndOfLine(void) {
    if (!lcd_ctx.font) return;

    uint8_t char_width = lcd_ctx.font->w;
    uint8_t char_height = lcd_ctx.font->h;
    uint16_t remaining_width = lcd_ctx.width - lcd_ctx.cursor_x;
    uint16_t chars_to_clear = remaining_width / char_width;

    for (uint16_t i = 0; i < chars_to_clear; i++) {
        LCD_ShowChar(lcd_ctx.cursor_x + i * char_width,
                     lcd_ctx.cursor_y,
                     ' ',
                     lcd_ctx.text_color,
                     lcd_ctx.bg_color,
                     lcd_ctx.font);
    }
}

// 处理退格
void LCD_HandleBackspace(void) {
    if (!lcd_ctx.font) return;

    uint8_t char_width = lcd_ctx.font->w;

    if (lcd_ctx.cursor_x-8 >= char_width) {
        lcd_ctx.cursor_x -= char_width;
    } else if (lcd_ctx.cursor_y - lcd_ctx.start_y >= lcd_ctx.line_height) {
        // 退格到上一行
        lcd_ctx.cursor_y -= lcd_ctx.line_height;
        lcd_ctx.cursor_x = lcd_ctx.width - char_width;
    } else {
        return;  // 已经到开头
    }

    // 在当前位置显示空格（擦除字符）
    LCD_ShowChar(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
                 ' ', lcd_ctx.text_color, lcd_ctx.bg_color, lcd_ctx.font);
}

// 处理换行
void LCD_HandleNewLine(void) {
    if (!lcd_ctx.font) return;

    lcd_ctx.cursor_x = 8;

    lcd_ctx.cursor_y += lcd_ctx.line_height;

    // 检查是否需要滚动
    if (lcd_ctx.cursor_y - lcd_ctx.start_y + lcd_ctx.line_height > lcd_ctx.height) {
        // 屏幕向上滚动一行
        LCD_ScrollUp(lcd_ctx.line_height);
        lcd_ctx.cursor_y -= lcd_ctx.line_height;
    }
}

// 处理回车
void LCD_HandleCarriageReturn(void) {
    lcd_ctx.cursor_x = 8;
}

// 处理制表符
void LCD_HandleTab(void) {
    if (!lcd_ctx.font) return;

    uint8_t char_width = lcd_ctx.font->w;
    uint8_t tab_stop = 4;  // 4个字符的制表位

    // 计算下一个制表位位置
    uint8_t current_char_pos = lcd_ctx.cursor_x / char_width;
    uint8_t next_tab = ((current_char_pos / tab_stop) + 1) * tab_stop;
    uint16_t new_x = next_tab * char_width;

    // 检查是否超出屏幕宽度
    if (new_x + char_width <= lcd_ctx.width) {
        lcd_ctx.cursor_x = new_x;
    } else {
        // 换行
        LCD_HandleNewLine();
    }
}

void LCD_PrintString(const char *str) {
    if (!str || !lcd_ctx.font) return;

    uint8_t char_width = lcd_ctx.font->w;

    while (*str) {
        char ch = *str++;

        switch (ch) {
            case '\n':  // 换行
                LCD_HandleNewLine();
                break;

            case '\r':  // 回车
                LCD_HandleCarriageReturn();
                break;

            case '\b':  // 退格
                LCD_HandleBackspace();
                break;

            case '\t':  // 制表符
                LCD_HandleTab();
                break;

            case '\f':  // 换页（清屏）
//                LCD_Clear(lcd_ctx.bg_color);
            	LCD_ClearArea(8, lcd_ctx.start_y, X_MAX_PIXEL-16, lcd_ctx.bg_height, lcd_ctx.bg_color);
                lcd_ctx.cursor_x = 8;
                lcd_ctx.cursor_y = lcd_ctx.start_y;
                break;

            case '\v':  // 垂直制表
                lcd_ctx.cursor_y += lcd_ctx.line_height;
                break;

            default:  // 普通字符
                // 检查是否需要自动换行
                if (lcd_ctx.auto_wrap &&
                    lcd_ctx.cursor_x + char_width > lcd_ctx.width) {
                    LCD_HandleNewLine();
                }

                // 检查是否需要滚动
                if (lcd_ctx.cursor_y - lcd_ctx.start_y + lcd_ctx.line_height > lcd_ctx.height) {
                    LCD_ScrollUp(lcd_ctx.line_height);
                    lcd_ctx.cursor_y -= lcd_ctx.line_height;
                }

                // 显示字符
                LCD_ShowChar(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
                           ch, lcd_ctx.text_color, lcd_ctx.bg_color, lcd_ctx.font);

                // 移动光标
                lcd_ctx.cursor_x += char_width;
                break;
        }
    }
}

// 主显示函数（支持转义序列）
void LCD_Printf(const char *format, ...) {
    if (!lcd_ctx.font) return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    LCD_PrintString(buffer);
}


