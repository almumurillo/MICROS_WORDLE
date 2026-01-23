/*
 * ws2812.h
 *
 *  Created on: 18 dic 2025
 *      Author: almur
 */

#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

void WS2812_Init(GPIO_TypeDef* gpioPort, uint16_t gpioPin, uint16_t numLeds);
void WS2812_SetPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
void WS2812_Show(void);
void WS2812_Clear(void);


#endif /* INC_WS2812_H_ */
