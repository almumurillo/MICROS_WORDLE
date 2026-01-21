/*
 * palabras.h
 *
 *  Módulo que gestiona la lista de palabras del juego Wordle
 */

#ifndef INC_PALABRAS_H_
#define INC_PALABRAS_H_

#include <stdint.h>                 // Para usar tipos como uint32_t

// Longitud fija de cada palabra (5 letras)
#define LONGITUD_PALABRA   5

// Número total de palabras en la pool
#define NUM_PALABRAS       20

// Inicializa el generador pseudoaleatorio con una semilla
void Palabras_Init(uint32_t seed);

// Devuelve un puntero a una palabra aleatoria de la pool
const char* Palabras_GetRandom(void);

#endif /* INC_PALABRAS_H_ */
