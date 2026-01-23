/*
 * lcd_i2c.h
 *
 *  Created on: 26 dic 2025
 *      Author: almur
 */

#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_

#include "main.h"
#include <stdint.h>

void LCD_Init(I2C_HandleTypeDef *hi2c, uint8_t i2c_addr);
void LCD_Clear(void);
void LCD_Home(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *s);
void LCD_PrintChar(char c);

#endif /* INC_LCD_I2C_H_ */
