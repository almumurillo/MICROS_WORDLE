/*
 * palabras.c
 *Vamos a empezar con estas palabras, podremos cambiarlas o añadir otras
 *  Implementación del módulo de gestión de palabras
 *  Cada línea tiene su comentario para que quede muy claro en la memoria:
 *  el array es static para que solo sea visible dentro del módulo, Palabras_Init se usa una vez al inicio de la partida (con una semilla tomada de un HAL_GetTick() o de una lectura ADC) y Palabras_GetRandom te da la palabra secreta para cada ronda.
 */

#include "palabras.h"   // Cabecera del propio módulo
#include <stdlib.h>     // rand(), srand()

// Lista fija de palabras de 5 letras, en mayúsculas y sin tildes/ñ
static const char* s_palabras[NUM_PALABRAS] =
{
    "CASAS", "PERRO", "GATOS", "LUCES", "RADAR",
    "PLAZA", "LIBRO", "PISTA", "CABLE", "FICHA",
    "NIVEL", "PARED", "ROBOT", "TECLA", "DATOS",
    "JUEGO", "METAL", "RUIDO", "VALOR", "SIGMA"
};

// Carga la semilla del generador pseudoaleatorio
void Palabras_Init(uint32_t seed)
{
    srand(seed);
}

// Devuelve un puntero a una palabra elegida al azar
const char* Palabras_GetRandom(void)
{
    uint32_t idx = rand() % NUM_PALABRAS;  // Índice entre 0 y NUM_PALABRAS-1
    return s_palabras[idx];                // Devuelve la palabra elegida
}

