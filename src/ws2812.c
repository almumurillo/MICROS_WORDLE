/*
 * ws2812.c
 *
 *  Created on: 18 dic 2025
 *      Author: almur
 */
#include "ws2812.h"
#include <stdlib.h>

static GPIO_TypeDef* s_port = NULL;
static uint16_t      s_pin  = 0;
static uint16_t      s_num  = 0;
static uint8_t*      s_buf  = NULL;

static inline void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void delay_cycles(uint32_t cycles)
{
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles) {;}
}

static inline void pin_high(void) { s_port->BSRR = (uint32_t)s_pin; }
static inline void pin_low(void)  { s_port->BSRR = (uint32_t)s_pin << 16U; }

static inline void send_bit(uint8_t bit)
{
    const uint32_t T0H  = 30;
    const uint32_t T1H  = 60;
    const uint32_t TBIT = 105;

    pin_high();
    if (bit) { delay_cycles(T1H); pin_low(); delay_cycles(TBIT - T1H); }
    else     { delay_cycles(T0H); pin_low(); delay_cycles(TBIT - T0H); }
}

static void send_byte(uint8_t b)
{
    for (int8_t i = 7; i >= 0; i--) send_bit((b >> i) & 0x01);
}

void WS2812_Init(GPIO_TypeDef* gpioPort, uint16_t gpioPin, uint16_t numLeds)
{
    s_port = gpioPort; s_pin = gpioPin; s_num = numLeds;

    if (s_buf) { free(s_buf); s_buf = NULL; }
    s_buf = (uint8_t*)calloc((size_t)s_num * 3U, 1U);

    dwt_init();
}

void WS2812_SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    if (!s_buf || idx >= s_num) return;
    uint16_t base = (uint16_t)(idx * 3U);
    s_buf[base + 0] = g; // GRB
    s_buf[base + 1] = r;
    s_buf[base + 2] = b;
}

void WS2812_Show(void)
{
    if (!s_buf) return;

    __disable_irq();
    for (uint16_t i = 0; i < (uint16_t)(s_num * 3U); i++) send_byte(s_buf[i]);
    __enable_irq();

    delay_cycles(5000); // >50us latch
}

void WS2812_Clear(void)
{
    if (!s_buf) return;
    for (uint16_t i = 0; i < (uint16_t)(s_num * 3U); i++) s_buf[i] = 0;
    WS2812_Show();
}

