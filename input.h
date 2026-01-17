#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#include <stdint.h>

// Inicializa estructuras internas de input (si hace falta)
void Input_Init(void);

// Escanea el teclado 4x4.
void Input_ScanKeypad(void);// Debe llamarse periódicamente (por ejemplo cada 5–10 ms desde el timer).
void Input_ResetPagina(void);//Resetea la pagina de letras al principio y actualiza el LED de paginas

uint8_t Input_GetPagina(void);

#endif /* INC_INPUT_H_ */
