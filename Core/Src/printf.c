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



//
///**
// * @brief 物理光标 -> 历史记录同步
// * 根据当前的物理光标位置更新历史记录中的行和列
// */
//void LCD_SyncHistoryFromCursor(void) {
//    if (!lcd_ctx.font) return;
//
//    uint8_t char_width = lcd_ctx.font->w;
//    uint8_t char_height = lcd_ctx.font->h;
//
//    // 1. 根据Y坐标计算行号
//    if (lcd_ctx.cursor_y >= lcd_ctx.start_y) {
//        uint8_t display_line = (lcd_ctx.cursor_y - lcd_ctx.start_y) / char_height;
//
//        if (display_line < lcd_ctx.line_count) {
//            uint16_t new_line = lcd_ctx.history_head + display_line;
//
//            if (new_line < MAX_HISTORY_LINES) {
//                lcd_ctx.current_line = new_line;
//
//                if (lcd_ctx.current_line >= lcd_ctx.history_count) {
//                    lcd_ctx.history_count = lcd_ctx.current_line + 1;
//                }
//            }
//        }
//    }
//
//    // 2. 根据X坐标计算列位置
//    if (lcd_ctx.cursor_x >= 8) {
//        uint8_t new_pos = (lcd_ctx.cursor_x - 8) / char_width;
//        if (new_pos <= MAX_CHARS_PER_LINE) {
//            lcd_ctx.current_line_pos = new_pos;
//        }
//    }
//
//    // 3. 确保字符串终止
//    lcd_ctx.history[lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';
//}
//
///**
// * @brief 历史记录 -> 物理光标同步
// * 根据当前的历史记录位置更新物理光标
// */
//void LCD_SyncCursorFromHistory(void) {
//    if (!lcd_ctx.font) return;
//
//    uint8_t char_width = lcd_ctx.font->w;
//    uint8_t char_height = lcd_ctx.font->h;
//
//    // 计算当前行在显示区域的位置
//    int16_t display_line = lcd_ctx.current_line - lcd_ctx.history_head;
//
//    if (display_line >= 0 && display_line < lcd_ctx.line_count) {
//        // 当前行可见
//        lcd_ctx.cursor_y = lcd_ctx.start_y + display_line * char_height;
//    } else if (lcd_ctx.current_line < lcd_ctx.history_head) {
//        // 当前行在上方，滚动上去
//        lcd_ctx.history_head = lcd_ctx.current_line;
//        lcd_ctx.cursor_y = lcd_ctx.start_y;
//    } else {
//        // 当前行在下方，滚动下来
//        lcd_ctx.history_head = lcd_ctx.current_line - lcd_ctx.line_count + 1;
//        lcd_ctx.cursor_y = lcd_ctx.start_y + (lcd_ctx.line_count - 1) * char_height;
//    }
//
//    // 更新X坐标
//    lcd_ctx.cursor_x = 8 + lcd_ctx.current_line_pos * char_width;
//}
//
//void LCD_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg_color, const ASCIIFont *font) {
//    if (!font) return;
//
//    uint8_t char_width = font->w;
//    uint8_t char_height = font->h;
//    uint8_t bytes_per_char = char_height;
//
//    // 处理不可见字符
//    if (ch < 32 || ch > 126) {
//        if (ch != '\n' && ch != '\r' && ch != '\b' && ch != '\t') {
//            ch = '?';
//        } else {
//            return;  // 控制字符由上层函数处理
//        }
//    }
//
//    // 设置显示窗口
//    set_window(x, y, x + char_width - 1, y + char_height - 1);
//
//    // 获取字模数据指针
//    uint8_t (*font_data)[bytes_per_char] = (uint8_t (*)[bytes_per_char])font->chars;
//
//    uint8_t high = color >> 8;
//    uint8_t low = color & 0xFF;
//    uint8_t bg_high = bg_color >> 8;
//    uint8_t bg_low = bg_color & 0xFF;
//
//    uint8_t buffer[256];  // 足够存放一个字符的像素数据
//    uint16_t idx = 0;
//
//    for (uint8_t row = 0; row < char_height; row++) {
//        for (uint8_t col = 0; col < char_width; col++) {
//            uint8_t row_data = font_data[ch - 32][row];
//            uint8_t bit_mask = (char_width == 6) ? (0x40 >> col) : (0x80 >> col);
//
//            if (row_data & bit_mask) {
//                buffer[idx++] = high;
//                buffer[idx++] = low;
//            } else {
//                buffer[idx++] = bg_high;
//                buffer[idx++] = bg_low;
//            }
//        }
//    }
//
//    // 发送数据
//    DC_SET1();
//    CS_SET0();
//    HAL_SPI_Transmit(&hspi2, buffer, idx, HAL_MAX_DELAY);
//    CS_SET1();
//}
//
///* @brief 初始化LCD文本框
// * @parameter y 起始位置
// * @parameter count_of_line 显示区行数
// * @parameter font 字体
// * @parameter text_color 文本颜色
// * @parameter bg_color 背景颜色
// */
//void LCD_InitContext(uint8_t y, uint8_t count_of_line, const ASCIIFont *font, uint16_t text_color, uint16_t bg_color) {
//    uint8_t h = count_of_line*font->h;
//    uint8_t w = X_MAX_PIXEL-16;
//    if(h>96){h=96;}
//
//    lcd_ctx.font = font;
//    lcd_ctx.text_color = text_color;
//    lcd_ctx.bg_color = bg_color;
//    lcd_ctx.line_height = font->h;
//    lcd_ctx.cursor_x = 12;
//    lcd_ctx.cursor_y = y;
//    lcd_ctx.start_y = y;
//    lcd_ctx.line_count = count_of_line;
//    lcd_ctx.height = h;
//    lcd_ctx.auto_wrap = 1;
//
//    lcd_ctx.bg_height = h;
//
//    LCD_ClearArea(6, y-2, w+4, h+4, WHITE);
//    LCD_ClearArea(X_MAX_PIXEL-7,y-1,1,h+2,GRAY1);
//    LCD_ClearArea(6,y-2,1,h+4,GRAY0);
//    LCD_ClearArea(7,y+1+h,w+3,1,GRAY2);
//    LCD_ClearArea(8, y, w, h, bg_color);
//}
//
//// 清除从当前光标到行尾的内容
//void LCD_ClearToEndOfLine(void) {
//    if (!lcd_ctx.font) return;
//
//    uint8_t char_width = lcd_ctx.font->w;
//    uint8_t char_height = lcd_ctx.font->h;
//    uint16_t remaining_width = lcd_ctx.width - lcd_ctx.cursor_x;
//    uint16_t chars_to_clear = remaining_width / char_width;
//
//    for (uint16_t i = 0; i < chars_to_clear; i++) {
//        LCD_ShowChar(lcd_ctx.cursor_x + i * char_width,
//                     lcd_ctx.cursor_y,
//                     ' ',
//                     lcd_ctx.text_color,
//                     lcd_ctx.bg_color,
//                     lcd_ctx.font);
//    }
//}
//
//// 处理退格
//void LCD_HandleBackspace(void) {
//    if (!lcd_ctx.font) return;
//
//    uint8_t char_width = lcd_ctx.font->w;
//
//    if (lcd_ctx.cursor_x-8 >= char_width) {
//        lcd_ctx.cursor_x -= char_width;
//    } else if (lcd_ctx.cursor_y - lcd_ctx.start_y >= lcd_ctx.line_height) {
//        // 退格到上一行
//        lcd_ctx.cursor_y -= lcd_ctx.line_height;
//        lcd_ctx.cursor_x = lcd_ctx.width - char_width;
//    } else {
//        return;  // 已经到开头
//    }
//
//    // 在当前位置显示空格（擦除字符）
//    LCD_ShowChar(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
//                 ' ', lcd_ctx.text_color, lcd_ctx.bg_color, lcd_ctx.font);
//}
//
//// 处理换行
//void LCD_HandleNewLine(void) {
//    if (!lcd_ctx.font) return;
//
//    lcd_ctx.cursor_x = 8;
//
//    lcd_ctx.cursor_y += lcd_ctx.line_height;
//
//    // 检查是否需要滚动
//    if (lcd_ctx.cursor_y - lcd_ctx.start_y + lcd_ctx.line_height > lcd_ctx.height) {
//        // 屏幕向上滚动一行
//        LCD_ScrollUp(lcd_ctx.line_height);
//        lcd_ctx.cursor_y -= lcd_ctx.line_height;
//    }
//}
//
//// 处理回车
//void LCD_HandleCarriageReturn(void) {
//    lcd_ctx.cursor_x = 8;
//}
//
//// 处理制表符
//void LCD_HandleTab(void) {
//    if (!lcd_ctx.font) return;
//
//    uint8_t char_width = lcd_ctx.font->w;
//    uint8_t tab_stop = 4;  // 4个字符的制表位
//
//    // 计算下一个制表位位置
//    uint8_t current_char_pos = lcd_ctx.cursor_x / char_width;
//    uint8_t next_tab = ((current_char_pos / tab_stop) + 1) * tab_stop;
//    uint16_t new_x = next_tab * char_width;
//
//    // 检查是否超出屏幕宽度
//    if (new_x + char_width <= lcd_ctx.width) {
//        lcd_ctx.cursor_x = new_x;
//    } else {
//        // 换行
//        LCD_HandleNewLine();
//    }
//}
//
//void LCD_PrintString(const char *str) {
//    if (!str || !lcd_ctx.font) return;
//
//    uint8_t char_width = lcd_ctx.font->w;
//
//    while (*str) {
//        char ch = *str++;
//
//        switch (ch) {
//            case '\n':  // 换行
//                LCD_HandleNewLine();
//                break;
//
//            case '\r':  // 回车
//                LCD_HandleCarriageReturn();
//                break;
//
//            case '\b':  // 退格
//                LCD_HandleBackspace();
//                break;
//
//            case '\t':  // 制表符
//                LCD_HandleTab();
//                break;
//
//            case '\f':  // 换页（清屏）
////                LCD_Clear(lcd_ctx.bg_color);
//            	LCD_ClearArea(8, lcd_ctx.start_y, X_MAX_PIXEL-16, lcd_ctx.bg_height, lcd_ctx.bg_color);
//                lcd_ctx.cursor_x = 8;
//                lcd_ctx.cursor_y = lcd_ctx.start_y;
//                break;
//
//            case '\v':  // 垂直制表
//                lcd_ctx.cursor_y += lcd_ctx.line_height;
//                break;
//
//            default:  // 普通字符
//                // 检查是否需要自动换行
//                if (lcd_ctx.auto_wrap &&
//                    lcd_ctx.cursor_x + char_width > lcd_ctx.width) {
//                    LCD_HandleNewLine();
//                }
//
//                // 检查是否需要滚动
//                if (lcd_ctx.cursor_y - lcd_ctx.start_y + lcd_ctx.line_height > lcd_ctx.height) {
//                    LCD_ScrollUp(lcd_ctx.line_height);
//                    lcd_ctx.cursor_y -= lcd_ctx.line_height;
//                }
//
//                // 显示字符
//                LCD_ShowChar(lcd_ctx.cursor_x, lcd_ctx.cursor_y,
//                           ch, lcd_ctx.text_color, lcd_ctx.bg_color, lcd_ctx.font);
//
//                // 移动光标
//                lcd_ctx.cursor_x += char_width;
//                break;
//        }
//    }
//}
//
//// 主显示函数（支持转义序列）
//void LCD_Printf(const char *format, ...) {
//    if (!lcd_ctx.font) return;
//
//    char buffer[256];
//    va_list args;
//    va_start(args, format);
//    vsnprintf(buffer, sizeof(buffer), format, args);
//    va_end(args);
//
//    LCD_PrintString(buffer);
//}
//
//


// ==================== 内部辅助函数 ====================

/**
 * @brief 在指定物理位置显示单个字符（不修改历史记录）
 */
void LCD_ShowChar(uint8_t x, uint8_t y, char ch,
                          uint16_t color, uint16_t bg_color,
                          const ASCIIFont *font) {
    if (!font) return;

    uint8_t char_width = font->w;
    uint8_t char_height = font->h;
    uint8_t bytes_per_char = char_height;

    // 处理不可见字符
    if (ch < 32 || ch > 126) {
        ch = '?';
    }

    // 设置显示窗口
    set_window(x, y, x + char_width - 1, y + char_height - 1);

    // 获取字模数据指针
    uint8_t (*font_data)[bytes_per_char] = (uint8_t (*)[bytes_per_char])font->chars;

    uint8_t high = color >> 8;
    uint8_t low = color & 0xFF;
    uint8_t bg_high = bg_color >> 8;
    uint8_t bg_low = bg_color & 0xFF;

    // 计算缓冲区大小并分配
    uint16_t buffer_size = char_width * char_height * 2;
    uint8_t buffer[buffer_size];
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
    HAL_SPI_Transmit(&hspi2, buffer, buffer_size, HAL_MAX_DELAY);
    CS_SET1();
}



// ==================== 核心同步函数 ====================

void LCD_SyncCursor(void) {
    if (!lcd_ctx.font) return;

    uint8_t char_width = lcd_ctx.font->w;
    uint8_t char_height = lcd_ctx.font->h;

    // 1. 边界检查
    if (lcd_ctx.current_line >= MAX_HISTORY_LINES) {
        lcd_ctx.current_line = MAX_HISTORY_LINES - 1;
    }

    if (lcd_ctx.current_line_pos > lcd_ctx.max_charsperline) {
        lcd_ctx.current_line_pos = lcd_ctx.max_charsperline;
    }

    // 2. 确保字符串终止
    lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';

    // 3. 计算当前行在显示区域中的位置
    int16_t display_line = lcd_ctx.current_line - lcd_ctx.history_head;

    if (display_line >= 0 && display_line < lcd_ctx.line_count) {
        // 当前行在显示区域内
        lcd_ctx.cursor_y = lcd_ctx.start_y + display_line * char_height;
    } else
    	// 当前行不在显示区域，自动调整 history_head
    	if (lcd_ctx.current_line < lcd_ctx.history_head) {
        // 当前行在显示区域上方，自动滚动
        lcd_ctx.history_head = lcd_ctx.current_line;
        lcd_ctx.cursor_y = lcd_ctx.start_y;
    } else {
        // 当前行在显示区域下方，自动滚动

        if (lcd_ctx.history_count > lcd_ctx.line_count) {
            lcd_ctx.history_head = lcd_ctx.current_line - lcd_ctx.line_count + 1;
        } else {
            lcd_ctx.history_head = 0;
        }
        lcd_ctx.cursor_y = lcd_ctx.start_y + (lcd_ctx.line_count - 1) * char_height;
    }


    // 4. 更新X坐标
    lcd_ctx.cursor_x = LEFT_MARGIN + lcd_ctx.current_line_pos * char_width;
}

void LCD_RefreshDisplay(void) {
    if (!lcd_ctx.font) return;

    uint8_t char_width = lcd_ctx.font->w;
    uint8_t char_height = lcd_ctx.font->h;

    // 调整history_head
    if (lcd_ctx.history_count > lcd_ctx.line_count) {
        if (lcd_ctx.history_head > lcd_ctx.history_count - lcd_ctx.line_count) {
            lcd_ctx.history_head = lcd_ctx.history_count - lcd_ctx.line_count;
        }
    } else {
        lcd_ctx.history_head = 0;
    }

    // 清除显示区域
    LCD_ClearArea(LEFT_MARGIN, lcd_ctx.start_y,
                  lcd_ctx.width, lcd_ctx.height, lcd_ctx.bg_color);

    // 显示历史记录

    for (uint8_t i = 0; i < lcd_ctx.line_count; i++) {
        uint16_t history_idx = lcd_ctx.history_head + i;
        if (history_idx >= lcd_ctx.history_count) break;

        char *line = lcd_ctx.history[lcd_ctx.current_page][history_idx];
        if (line[0] == '\0') continue;

        uint8_t y = lcd_ctx.start_y + i * char_height;

        for (uint8_t j = 0; j < lcd_ctx.max_charsperline && line[j] != '\0'; j++) {
            uint8_t x = LEFT_MARGIN + j * char_width;
            LCD_ShowChar(x, y, line[j],
                          lcd_ctx.text_color, lcd_ctx.bg_color, lcd_ctx.font);
        }
    }


    // 同步光标位置
    LCD_SyncCursor();

}

// ==================== 历史记录操作函数 ====================

void LCD_InsertChar(char ch) {
    if (!lcd_ctx.font) return;

    // 检查是否需要自动换行
    if (lcd_ctx.auto_wrap && lcd_ctx.current_line_pos >= lcd_ctx.max_charsperline) {
        LCD_NewLine();
    }

    // 插入字符
    if (lcd_ctx.current_line_pos < lcd_ctx.max_charsperline) {
        lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = ch;
        lcd_ctx.current_line_pos++;
        lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';
    }


    // 同步并显示
    LCD_SyncCursor();
    LCD_RefreshDisplay();
}

void LCD_Backspace(void) {
    if (!lcd_ctx.font) return;

    if (lcd_ctx.current_line_pos > 0) {
        // 当前行退格
        lcd_ctx.current_line_pos--;
        lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';
    } else if (lcd_ctx.current_line > 0) {
        // 合并到上一行
        uint8_t prev_line_len = strlen(lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line - 1]);
        uint8_t current_line_len = strlen(lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line]);

        // 检查是否会超出最大长度
        if (prev_line_len + current_line_len <= lcd_ctx.max_charsperline) {
            // 将当前行内容追加到上一行
            strcat(lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line - 1],
                   lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line]);

            // 删除当前行
            for (uint16_t i = lcd_ctx.current_line; i < lcd_ctx.history_count - 1; i++) {
                strcpy(lcd_ctx.history[lcd_ctx.current_page][i], lcd_ctx.history[lcd_ctx.current_page][i + 1]);
            }
            lcd_ctx.history_count--;
            lcd_ctx.current_line--;
            lcd_ctx.current_line_pos = prev_line_len + current_line_len;
        }
    }

    // 同步并显示
    LCD_SyncCursor();
    LCD_RefreshDisplay();
}

void LCD_NewLine(void) {
    if (!lcd_ctx.font) return;

    // 结束当前行
    lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';

    // 增加新行
    lcd_ctx.history_count++;
    lcd_ctx.current_line = lcd_ctx.history_count - 1;
    lcd_ctx.current_line_pos = 0;
    lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][0] = '\0';

    // 历史记录上限处理（循环覆盖）
    if (lcd_ctx.history_count > MAX_HISTORY_LINES) {
        // 移动所有行向前
        for (uint16_t i = 1; i < MAX_HISTORY_LINES; i++) {
            strcpy(lcd_ctx.history[lcd_ctx.current_page][i - 1], lcd_ctx.history[lcd_ctx.current_page][i]);
        }
        lcd_ctx.history_count = MAX_HISTORY_LINES;
        lcd_ctx.current_line = MAX_HISTORY_LINES - 1;
    }

    // 检查是否需要进入新页（仅当不是第0页）
    if (lcd_ctx.current_page > 0 && lcd_ctx.current_line + 1 >= MAX_HISTORY_LINES) {
        // 非第0页且当前页已满，自动进入下一页
        LCD_FormFeed();
        return;
    }

    // 同步并显示
    LCD_SyncCursor();
    LCD_RefreshDisplay();
}

void LCD_CarriageReturn(void) {
    if (!lcd_ctx.font) return;

    lcd_ctx.current_line_pos = 0;


    // 同步并显示
    LCD_SyncCursor();
    LCD_RefreshDisplay();
}

void LCD_Tab(void) {
    if (!lcd_ctx.font) return;

    uint8_t tab_stop = 4;
    uint8_t current_pos = lcd_ctx.current_line_pos;
    uint8_t next_tab = ((current_pos / tab_stop) + 1) * tab_stop;

    if (next_tab < lcd_ctx.max_charsperline) {
        // 插入空格到下一个制表位
        for (uint8_t i = current_pos; i < next_tab; i++) {
            lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][i] = ' ';
        }
        lcd_ctx.current_line_pos = next_tab;
        lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][next_tab] = '\0';
    } else {
        // 超出最大长度，换行
        LCD_NewLine();
    }


    // 同步并显示
    LCD_SyncCursor();
    LCD_RefreshDisplay();
}

void LCD_FormFeed(void) {
    if (!lcd_ctx.font) return;

    // 结束当前行
    lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';

    // 创建新页
    lcd_ctx.current_page++;

    // 如果超出最大页数，循环覆盖
    if (lcd_ctx.current_page >= MAX_PAGES) {
        lcd_ctx.current_page = 0;
    }

    // 更新页计数
    if (lcd_ctx.current_page >= lcd_ctx.page_count) {
        lcd_ctx.page_count = lcd_ctx.current_page + 1;
    }

    // 重置新页的状态
    lcd_ctx.current_line = 0;
    lcd_ctx.current_line_pos = 0;
    lcd_ctx.history_count = 1;
    lcd_ctx.history_head = 0;
    lcd_ctx.page_history_head = 0;

    // 初始化新页的第一行
    lcd_ctx.history[lcd_ctx.current_page][0][0] = '\0';

    // 清屏并显示新页
    LCD_ClearArea(LEFT_MARGIN, lcd_ctx.start_y,
                  lcd_ctx.width, lcd_ctx.height, lcd_ctx.bg_color);

    lcd_ctx.cursor_x = LEFT_MARGIN;
    lcd_ctx.cursor_y = lcd_ctx.start_y;

        // 同步并显示
    LCD_SyncCursor();
    LCD_RefreshDisplay();

    // 显示页码
    char page_buf[30];
    if (lcd_ctx.current_page == 0) {
        sprintf(page_buf, "Page: Draft");
    } else {
        sprintf(page_buf, "Page: %d/%d", lcd_ctx.current_page, lcd_ctx.page_count - 1);
    }
    LCD_ShowString(8, 15, page_buf, CYAN, &afont8x6);
}

void LCD_VerticalTab(void) {
    if (!lcd_ctx.font) return;

    // 向下移动一行
    lcd_ctx.current_line++;
    lcd_ctx.current_line_pos = 0;

    // 确保有这一行
    if (lcd_ctx.current_line >= lcd_ctx.history_count) {
        lcd_ctx.history_count = lcd_ctx.current_line + 1;
        lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][0] = '\0';
    }

    // 同步并显示
    LCD_SyncCursor();
    LCD_RefreshDisplay();
}

// ==================== 初始化函数 ====================

void LCD_InitContext(uint8_t y, uint8_t count_of_line,
                     const ASCIIFont *font,
                     uint16_t text_color,
                     uint16_t bg_color) {

    uint8_t h = count_of_line * font->h;
    uint8_t w = X_MAX_PIXEL - 16;
    if (h > 96) { h = 96; }

    // 初始化所有字段
    lcd_ctx.font = font;
    lcd_ctx.text_color = text_color;
    lcd_ctx.bg_color = bg_color;
    lcd_ctx.start_y = y;
    lcd_ctx.line_count = count_of_line;
    lcd_ctx.width = w;
    lcd_ctx.height = h;
    lcd_ctx.auto_wrap = 1;
    lcd_ctx.max_charsperline = 112/lcd_ctx.font->w-1;

    // 初始化历史记录
    lcd_ctx.history_count = 1;
    lcd_ctx.history_head = 0;
    lcd_ctx.current_line = 0;
    lcd_ctx.current_line_pos = 0;
    for (uint16_t page = 0; page < MAX_PAGES; page++) {
    lcd_ctx.history[page][0][0] = '\0';
    }
    // 同步光标位置
    LCD_SyncCursor();

    // 绘制边框
    LCD_ClearArea(6, y-2, w+4, h+4, WHITE);
    LCD_ClearArea(X_MAX_PIXEL-7, y-1, 1, h+2, GRAY1);
    LCD_ClearArea(6, y-2, 1, h+4, GRAY0);
    LCD_ClearArea(7, y+1+h, w+3, 1, GRAY2);

    // 清除显示区域
    LCD_RefreshDisplay();
}


// ==================== 打印函数 ====================

void LCD_PrintString(const char *str) {
    if (!str || !lcd_ctx.font) return;

    while (*str) {
        char ch = *str++;

        switch (ch) {
            case '\n':  LCD_NewLine();          break;
            case '\b':  LCD_Backspace();        break;
            case '\r':  LCD_CarriageReturn();   break;
            case '\t':  LCD_Tab();              break;
            case '\f':  LCD_FormFeed();         break;
            case '\v':  LCD_VerticalTab();      break;
            default:    LCD_InsertChar(ch);     break;
        }
    }
}

void LCD_Printf(const char *format, ...) {
    if (!lcd_ctx.font) return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    LCD_PrintString(buffer);
}
