#ifndef INC_UI_H_
#define INC_UI_H_

#include <stdint.h>
#include "juego.h"   // para LETRA_MAL, LETRA_MAL_POS, LETRA_OK

// Inicializa el módulo de interfaz (si hace falta algo extra)
void UI_Init(void);

// Muestra el resultado de un intento en los 5 LEDs
// resultado[i] = LETRA_MAL / LETRA_MAL_POS / LETRA_OK
void UI_MostrarResultado(const uint8_t resultado[LONGITUD_PALABRA]);

void UI_SetPaginaIndicador(uint8_t pagina);

#endif /* INC_UI_H_ */
