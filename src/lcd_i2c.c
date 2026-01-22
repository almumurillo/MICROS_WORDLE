/*
 * lcd_i2c.c
 *
 *  Created on: 26 dic 2025
 *      Author: almur
 */
#include "lcd_i2c.h"
#include <string.h>

// PCF8574 -> LCD en modo 4-bit
// P0=RS, P1=RW, P2=E, P3=BACKLIGHT, P4=D4, P5=D5, P6=D6, P7=D7
#define LCD_RS  (1<<0)
#define LCD_RW  (1<<1)
#define LCD_EN  (1<<2)
#define LCD_BL  (1<<3)
#define LCD_ADDR 0x27	//0x3F

static I2C_HandleTypeDef *s_hi2c = NULL;
static uint8_t s_addr = 0; // dirección 7-bit <<1 incluida para HAL

static void i2c_write(uint8_t data)
{
    HAL_I2C_Master_Transmit(s_hi2c, s_addr, &data, 1, 50);
}

static void pulse_enable(uint8_t data)
{
    i2c_write(data | LCD_EN);
    HAL_Delay(1);
    i2c_write(data & ~LCD_EN);
    HAL_Delay(1);
}

static void write4bits(uint8_t nibble, uint8_t control)
{
    uint8_t data = LCD_BL | control | (nibble << 4);
    i2c_write(data);
    pulse_enable(data);
}

static void send(uint8_t value, uint8_t mode_rs)
{
    uint8_t hi = (value >> 4) & 0x0F;
    uint8_t lo = value & 0x0F;

    write4bits(hi, mode_rs);
    write4bits(lo, mode_rs);
}

static void cmd(uint8_t c) { send(c, 0); }
static void data(uint8_t d) { send(d, LCD_RS); }

void LCD_Init(I2C_HandleTypeDef *hi2c, uint8_t i2c_addr)
{
    s_hi2c = hi2c;

    // En HAL, la dirección se pasa desplazada 1 bit
    s_addr = (uint8_t)(i2c_addr << 1);

    HAL_Delay(50);

    // Secuencia init 4-bit
    write4bits(0x03, 0);
    HAL_Delay(5);
    write4bits(0x03, 0);
    HAL_Delay(5);
    write4bits(0x03, 0);
    HAL_Delay(2);
    write4bits(0x02, 0); // 4-bit

    cmd(0x28); // 4-bit, 2 líneas, 5x8
    cmd(0x0C); // display ON, cursor OFF
    cmd(0x06); // entry mode
    cmd(0x01); // clear
    HAL_Delay(2);
}

void LCD_Clear(void)
{
    cmd(0x01);
    HAL_Delay(2);
}

void LCD_Home(void)
{
    cmd(0x02);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    static const uint8_t row_offsets[] = {0x00, 0x40};
    if (row > 1) row = 1;
    cmd(0x80 | (row_offsets[row] + col));
}

void LCD_Print(const char *s)
{
    while (*s) data((uint8_t)*s++);
}

void LCD_PrintChar(char c)
{
    data((uint8_t)c);
}


