/*
 * printf.h
 *
 *  Created on: 2026年3月5日
 *      Author: jt
 */

#ifndef INC_PRINTF_H_
#define INC_PRINTF_H_

#include <stdint.h>
#include "LCD.h"

void LCD_InitContext(uint8_t y, uint8_t count_of_line, const ASCIIFont *font, uint16_t text_color, uint16_t bg_color);
void LCD_Printf(const char *format, ...);


#endif /* INC_PRINTF_H_ */
