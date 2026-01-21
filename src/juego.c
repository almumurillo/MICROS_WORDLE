/*
 * juego.c
 *  Lógica principal del Wordle: máquina de estados y gestión de rondas
 */

#include "juego.h"
#include "ui.h"
#include "input.h"
#include "palabras.h"
#include "main.h"
#include "lcd_i2c.h"
#include <stdio.h>

#include <string.h>
#include <stdint.h>

#define MAX_INTENTOS 6
#define TURNO_MS     60000

typedef enum {
    JUEGO_WAIT_START = 0,
    JUEGO_INGRESANDO,
    JUEGO_FIN_GANADO,
    JUEGO_FIN_PERDIDO
} JuegoEstado;

static JuegoEstado s_estado = JUEGO_WAIT_START;
static volatile uint8_t s_mostrar_palabra_final = 0;

static char s_objetivo[LONGITUD_PALABRA + 1];
static char s_intento[LONGITUD_PALABRA + 1];
static uint8_t s_len = 0;	//cuantas letras llevo metidas en el intento actual

static uint8_t s_intentoN = 0;		//contador intentos
static uint32_t s_tiempoRestanteMs = TURNO_MS;		//contandor tiempo, baja en Juego_OnTick1ms()

// Resultado para la UI (LEDs, uno por letra)
static uint8_t s_resultado[LONGITUD_PALABRA];

// Flags (para NO tocar LCD en interrupciones)
static volatile uint8_t s_start_request = 0;
static volatile uint8_t s_lcd_update_request = 0;     // refrescar línea 2 (tiempo/intentos)
static volatile uint8_t s_lcd_full_refresh_request = 0; // refrescar todo LCD

// ================= FUNCIONES INTERNAS =================

static void UI_ClearLetters(void)		//apagar los LEDS de las letras
{
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
        s_resultado[i] = 0xFF;   // Apagado		LETRA_MAL->rojo; LETRA_MAL_POS->azul; LETRA_OK->verde; 0xFF->apagado

    UI_MostrarResultado(s_resultado);
}

static void LCD_MostrarIntento(const char intento[LONGITUD_PALABRA], uint8_t len)
{
    LCD_SetCursor(0,0);		//linea 1
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        if (i < len) LCD_PrintChar(intento[i]);
        else LCD_PrintChar('_');
    }
}

static void LCD_MostrarEstado(void)
{
    char buf[17];

    uint8_t intentos_restantes =
        (s_intentoN < MAX_INTENTOS) ? (MAX_INTENTOS - s_intentoN) : 0;

    uint32_t tiempo_seg = (s_tiempoRestanteMs + 999) / 1000;

    uint8_t pagina = (uint8_t) Input_GetPagina() + 1;

    LCD_SetCursor(1, 0); // segunda línea

    if (s_estado == JUEGO_FIN_GANADO)
       {
           LCD_Print("   GANASTE!     ");
           return;
       }

       if (s_estado == JUEGO_FIN_PERDIDO)
       {
           if (s_tiempoRestanteMs == 0)
               LCD_Print(" TIEMPO AGOTADO ");
           else
               LCD_Print("  SIN INTENTOS  ");
           return;
       }

       snprintf(buf, sizeof(buf),
                "I:%u  T:%02lu  P:%u     ",
                (unsigned)intentos_restantes,
                (unsigned long)tiempo_seg,
                (unsigned)pagina);

    LCD_Print(buf);
}

void Juego_RequestLcdUpdate(void)
{
    s_lcd_update_request = 1;
}

static void NuevaPartida_EnMainLoop(void)
{
    const char* palabra = Palabras_GetRandom();

    strncpy(s_objetivo, palabra, LONGITUD_PALABRA);
    s_objetivo[LONGITUD_PALABRA] = '\0';

    s_intentoN = 0;
    s_len = 0;
    s_intento[0] = '\0';

    s_tiempoRestanteMs = TURNO_MS;

    Input_ResetPagina();
    UI_ClearLetters();

    s_estado = JUEGO_INGRESANDO;

    HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET); // empieza apagado

    // Pedimos refresco completo del LCD (en main loop)
        s_lcd_full_refresh_request = 1;

}

static void CalcularResultado(const char* intento,
                              const char* objetivo,
                              uint8_t out[LONGITUD_PALABRA])
{
    uint8_t usado_obj[LONGITUD_PALABRA] = {0};	//¿Esta letra de la palabra objetivo ya ha sido utilizada para dar un resultado?
    uint8_t usado_int[LONGITUD_PALABRA] = {0};	//¿Esta letra del intento del jugador ya ha sido evaluada?

    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
        out[i] = LETRA_MAL;

    // Letras correctas en posición correcta
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        if (intento[i] == objetivo[i])
        {
            out[i] = LETRA_OK;
            usado_obj[i] = 1;
            usado_int[i] = 1;
        }
    }

    // Letras presentes en otra posición
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        if (usado_int[i]) continue;

        for (uint8_t j = 0; j < LONGITUD_PALABRA; j++)
        {
            if (usado_obj[j]) continue;

            if (intento[i] == objetivo[j])
            {
                out[i] = LETRA_MAL_POS;
                usado_obj[j] = 1;
                break;
            }
        }
    }
}

//  API PÚBLICA

void Juego_Init(void)
{
    // Inicializa el módulo de palabras con semilla temporal
    Palabras_Init(HAL_GetTick());

    s_estado = JUEGO_WAIT_START;
    UI_ClearLetters();

    LCD_Clear();
    LCD_SetCursor(0,0);
    LCD_Print("PULSA START");
    LCD_SetCursor(1,0);
    LCD_Print("I:6  T:60     ");
}

void Juego_Task(void)
{
	// 1) Si se pulsó Start en interrupción, arrancar partida aquí (main loop)
	    if (s_start_request)
	    {
	        s_start_request = 0;
	        if (s_estado == JUEGO_WAIT_START ||
	            s_estado == JUEGO_FIN_GANADO ||
	            s_estado == JUEGO_FIN_PERDIDO)
	        {
	            NuevaPartida_EnMainLoop();
	        }
	    }

	    // 2) Refresco LCD completo si se pidió
	    if (s_lcd_full_refresh_request)
	    {
	        s_lcd_full_refresh_request = 0;
	        LCD_Clear();
	        LCD_MostrarIntento(s_intento, s_len);
	        LCD_MostrarEstado();
	    }
	    if (s_estado == JUEGO_FIN_PERDIDO && s_mostrar_palabra_final)
	    {
	        LCD_Clear();

	        LCD_SetCursor(0, 0);
	        LCD_Print("HAS PERDIDO");

	        LCD_SetCursor(1, 0);
	        LCD_Print("ERA: ");
	        LCD_Print(s_objetivo);

	        s_mostrar_palabra_final = 0;
	        s_lcd_full_refresh_request = 0;
	        return;
	    }

	    // 3) Refresco de estado (tiempo/intentos) si se pidió
	    if (s_lcd_update_request)
	    {
	        s_lcd_update_request = 0;
	        LCD_MostrarEstado();
	    }

}

void Juego_OnStartButton(void)
{
	s_start_request = 1;
}

void Juego_OnTick1ms(void)
{
	static uint16_t blink_cnt = 0;
	static uint16_t sec_cnt = 0;


	if (s_estado != JUEGO_INGRESANDO){
		HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
		return;
	}

        if (s_tiempoRestanteMs > 0)
            s_tiempoRestanteMs--;

        // Parpadeo LD6 cada 500 ms
                blink_cnt++;
                if (blink_cnt >= 500)
                {
                    blink_cnt = 0;
                    HAL_GPIO_TogglePin(LD6_GPIO_Port, LD6_Pin);
                }

        // Cada 1 segundo pedir actualización de LCD
               sec_cnt++;
                  if (sec_cnt >= 1000)
                  {
                      sec_cnt = 0;
                      s_lcd_update_request = 1;
                  }

        if (s_tiempoRestanteMs == 0)
        {
            s_estado = JUEGO_FIN_PERDIDO;
        // Apagar LD6 al terminar el tiempo
                   HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
                   s_lcd_update_request = 1;
        }

}

void Juego_OnLetter(char c)
{
    if (s_estado != JUEGO_INGRESANDO) return;
    if (s_len >= LONGITUD_PALABRA) return;

    s_intento[s_len++] = c;
    s_intento[s_len] = '\0';

    LCD_MostrarIntento(s_intento, s_len);
}

void Juego_OnBackspace(void)
{
    if (s_estado != JUEGO_INGRESANDO) return;
    if (s_len == 0) return;

    s_len--;
    s_intento[s_len] = '\0';

    LCD_MostrarIntento(s_intento, s_len);
}

void Juego_OnEnter(void)
{
    if (s_estado != JUEGO_INGRESANDO) return;
    if (s_len != LONGITUD_PALABRA) return;

    CalcularResultado(s_intento, s_objetivo, s_resultado);
    UI_MostrarResultado(s_resultado);

    if (strncmp(s_intento, s_objetivo, LONGITUD_PALABRA) == 0)
    {
        s_estado = JUEGO_FIN_GANADO;
        HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
        s_lcd_update_request = 1;
        return;
    }

    s_intentoN++;
    if (s_intentoN >= MAX_INTENTOS)
    {
        s_estado = JUEGO_FIN_PERDIDO;
        s_mostrar_palabra_final = 1;
        HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
        s_lcd_full_refresh_request = 1;
        return;
    }

    // Preparar siguiente intento
    s_len = 0;
    s_intento[0] = '\0';
    s_tiempoRestanteMs = TURNO_MS;

    LCD_MostrarIntento(s_intento, s_len);
        s_lcd_update_request = 1;
}
