#ifndef INC_JUEGO_H_
#define INC_JUEGO_H_

#define LETRA_MAL        0   // rojo
#define LETRA_MAL_POS    1   // azul
#define LETRA_OK         2   // verde

#include <stdint.h>
#include "palabras.h"        // Aquí se define LONGITUD_PALABRA

// Inicializa variables internas del juego (se llama una vez al inicio)
void Juego_Init(void);

// Se llama en el while(1) del main para ejecutar la máquina de estados
void Juego_Task(void);

// Eventos externos que vienen del hardware / interrupciones:

// Llamar cada 1 ms desde HAL_TIM_PeriodElapsedCallback(TIM2)
void Juego_OnTick1ms(void);

// Llamar desde HAL_GPIO_EXTI_Callback cuando se pulse el botón de inicio
void Juego_OnStartButton(void);

// Llamar cuando el jugador introduzca una letra (desde input.c)
void Juego_OnLetter(char c);

// Llamar cuando pulse "borrar"; letra C
void Juego_OnBackspace(void);

// Llamar cuando pulse "ENTER" letra D
void Juego_OnEnter(void);

void Juego_RequestLcdUpdate(void);

#endif /* INC_JUEGO_H_ */
