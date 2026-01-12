/*
 * juego.c
 *  Lógica principal del Wordle: máquina de estados y gestión de rondas
 */

#include "juego.h"
#include "palabras.h"      // Para Palabras_Init y Palabras_GetRandom
#include "main.h"          // Para HAL_GetTick, etc.
#include "ui.h"

// ===== Variables estáticas internas =====

static JuegoState_t s_estadoActual;                         // Estado de la FSM
static char         s_palabraSecreta[LONGITUD_PALABRA + 1]; // Palabra secreta actual
static char         s_intento[LONGITUD_PALABRA + 1];        // Intento del jugador
static uint8_t      s_posicionLetra;                        // Índice 0..4 dentro del intento
static uint8_t      s_intentoNumero;                        // Número de intento (1..6 por ej.)
static uint32_t     s_tickMs;                               // Contador de milisegundos

static uint32_t s_tiempoTurnoTicks = 0;
static uint8_t  s_turnoActivo      = 0;

// ===== Funciones estáticas auxiliares =====
extern UART_HandleTypeDef huart2;

void Debug_PutChar(char c)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
}
// Compara intento vs. palabra secreta al estilo Wordle.
static void CalcularResultadoIntento(uint8_t resultado[LONGITUD_PALABRA])
{
    uint8_t usadasSecreta[LONGITUD_PALABRA] = {0};
    uint8_t usadasIntento[LONGITUD_PALABRA] = {0};

    // 1ª pasada: marcar verdes (letra y posición correctas)
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        if (s_intento[i] == s_palabraSecreta[i])
        {
            resultado[i] = LETRA_OK;      // verde
            usadasSecreta[i] = 1;
            usadasIntento[i] = 1;
        }
        else
        {
            resultado[i] = LETRA_MAL;     // suponemos roja
        }
    }

    // 2ª pasada: marcar amarillas (letra existe pero en otra posición)
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        if (usadasIntento[i]) continue;   // esta posición ya era verde

        for (uint8_t j = 0; j < LONGITUD_PALABRA; j++)
        {
            if (usadasSecreta[j]) continue;
            if (s_intento[i] == s_palabraSecreta[j])
            {
                resultado[i] = LETRA_MAL_POS;  // amarillo
                usadasSecreta[j] = 1;
                usadasIntento[i] = 1;
                break;
            }
        }
    }
}

// Copia la palabra secreta elegida al buffer local
static void CargarNuevaPalabraSecreta(const char* src)
{
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        s_palabraSecreta[i] = src[i];
    }
    s_palabraSecreta[LONGITUD_PALABRA] = '\0';
}

// ===== API pública =====

void Juego_Init(void)
{
    // Semilla básica: por ahora usamos el tick del sistema; más tarde, ADC
    uint32_t seed = HAL_GetTick();
    Palabras_Init(seed);

    // Elegimos la primera palabra secreta
    const char* palabra = Palabras_GetRandom();
    CargarNuevaPalabraSecreta(palabra);

    // Inicializamos intento y contadores
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        s_intento[i] = ' ';
    }
    s_intento[LONGITUD_PALABRA] = '\0';

    s_posicionLetra = 0;
    s_intentoNumero = 1;
    s_tickMs        = 0;

    // Estado inicial del juego: esperando botón de inicio
    s_estadoActual = JUEGO_STATE_IDLE;

    // Inicializar interfaz (apagar LEDs, etc.)
    UI_Init();
}

void Juego_Task(void)
{
    switch (s_estadoActual)
    {
    case JUEGO_STATE_IDLE:
        // Esperando botón de inicio (Juego_OnStartButton cambiará a NUEVA_RONDA)
        break;

    case JUEGO_STATE_NUEVA_RONDA:
    {
        // Elegir nueva palabra y resetear variables
        const char* palabra = Palabras_GetRandom();
        CargarNuevaPalabraSecreta(palabra);

        s_posicionLetra = 0;
        s_intentoNumero = 1;

        for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
        {
            s_intento[i] = ' ';
        }
        s_intento[LONGITUD_PALABRA] = '\0';

        // Apagar LEDs de resultado
        uint8_t vacio[LONGITUD_PALABRA] = {
            LETRA_MAL, LETRA_MAL, LETRA_MAL, LETRA_MAL, LETRA_MAL
        };
        UI_MostrarResultado(vacio);
        s_tiempoTurnoTicks = 57;   // ~15 segundos
        s_turnoActivo      = 1;
        s_estadoActual     = JUEGO_STATE_INPUT;


    }
    break;

    case JUEGO_STATE_INPUT:
        // Solo reaccionamos a Juego_OnLetter, Juego_OnBackspace y Juego_OnEnter
        break;

    case JUEGO_STATE_CHECK:
    {
        uint8_t resultado[LONGITUD_PALABRA];
        s_turnoActivo = 0;  //que deje de contar
        // Calcular colores tipo Wordle
        CalcularResultadoIntento(resultado);

        // Actualizar los 5 LEDs de resultado
        UI_MostrarResultado(resultado);

        // Comprobar si todas las letras son verdes
        uint8_t todasVerdes = 1;
        for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
        {
            if (resultado[i] != LETRA_OK)
            {
                todasVerdes = 0;
                break;
            }
        }

        if (todasVerdes)
        {
            s_estadoActual = JUEGO_STATE_WIN;
        }
        else
        {
            s_intentoNumero++;
            if (s_intentoNumero > 6)   // máximo 6 intentos
            {
                s_estadoActual = JUEGO_STATE_LOSE;
            }
            else
            {
                // Preparar siguiente intento
                s_posicionLetra = 0;
                for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
                {
                    s_intento[i] = ' ';
                }
                s_intento[LONGITUD_PALABRA] = '\0';
                s_tiempoTurnoTicks = 57;
                s_turnoActivo      = 1;
                s_estadoActual = JUEGO_STATE_INPUT;
            }
        }
    }
    break;

    case JUEGO_STATE_SHOW_RESULT:
        // (opcional) mostrar resultado un tiempo usando s_tickMs
        break;

    case JUEGO_STATE_WIN:
        // Animación de victoria; Juego_OnStartButton pondrá NUEVA_RONDA
        break;

    case JUEGO_STATE_LOSE:
        // Animación de derrota; Juego_OnStartButton pondrá NUEVA_RONDA
        break;

    default:
        s_estadoActual = JUEGO_STATE_IDLE;
        break;
    }
}

void Juego_OnTick1ms(void)   // ahora cada 0,26 s
{
    s_tickMs++;  // si ya lo usas para otras cosas

    if (s_turnoActivo && s_tiempoTurnoTicks > 0)
    {
        s_tiempoTurnoTicks--;

        if (s_tiempoTurnoTicks == 0)
        {
            s_turnoActivo  = 0;
            s_estadoActual = JUEGO_STATE_LOSE;

            // Encender LED especial por tiempo agotado (por ejemplo LD5 azul interno)
            HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, GPIO_PIN_SET);
        }

    }
}


void Juego_OnStartButton(void)
{
    if (s_estadoActual == JUEGO_STATE_IDLE ||
        s_estadoActual == JUEGO_STATE_WIN  ||
        s_estadoActual == JUEGO_STATE_LOSE)
    {
        s_estadoActual = JUEGO_STATE_NUEVA_RONDA;
    }
}

void Juego_OnLetter(char c)
{
	HAL_GPIO_TogglePin(LD6_GPIO_Port, LD6_Pin);  // LED azul
	    // resto de tu código (uso de letra, etc.)
	Debug_PutChar(c);

    if (s_estadoActual != JUEGO_STATE_INPUT)
        return;

    if (s_posicionLetra < LONGITUD_PALABRA)
    {
        s_intento[s_posicionLetra] = c;
        s_posicionLetra++;
        // Aquí más tarde se puede avisar a UI de que se ha puesto una letra
    }
}

void Juego_OnBackspace(void)
{
    if (s_estadoActual != JUEGO_STATE_INPUT)
        return;

    if (s_posicionLetra > 0)
    {
        s_posicionLetra--;
        s_intento[s_posicionLetra] = ' ';
        // Aquí se podría avisar a UI para apagar el LED de esa posición
    }
}

void Juego_OnEnter(void)
{
    if (s_estadoActual != JUEGO_STATE_INPUT)
        return;

    if (s_posicionLetra == LONGITUD_PALABRA)
    {
        s_intento[LONGITUD_PALABRA] = '\0';
        s_estadoActual = JUEGO_STATE_CHECK;
    }
}
