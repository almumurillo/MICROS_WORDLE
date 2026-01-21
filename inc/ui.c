/*
 * ui.c
 *  Control de los LEDs de resultado del Wordle
 */

#include "ui.h"
#include "main.h"   // para los puertos y pines PE8..PE12

//#define NUM_LEDS LONGITUD_PALABRA
/*
// Asignación de LEDs a pines
#define LED0_GPIO_PORT   GPIOE
#define LED0_PIN         GPIO_PIN_8   // primera letra

#define LED1_GPIO_PORT   GPIOE
#define LED1_PIN         GPIO_PIN_9   // segunda letra

#define LED2_GPIO_PORT   GPIOE
#define LED2_PIN         GPIO_PIN_10  // tercera letra

#define LED3_GPIO_PORT   GPIOE
#define LED3_PIN         GPIO_PIN_11  // cuarta letra

#define LED4_GPIO_PORT   GPIOE
#define LED4_PIN         GPIO_PIN_12  // quinta letra*/

#define PAGE_LED0_GPIO_PORT GPIOD
#define PAGE_LED0_GPIO_PIN  LD3_Pin   // Pagina 0 (PD12)

#define PAGE_LED1_GPIO_PORT GPIOD
#define PAGE_LED1_GPIO_PIN  LD4_Pin   // Pagina 1 (PD13)

#define PAGE_LED2_GPIO_PORT GPIOD
#define PAGE_LED2_GPIO_PIN  LD5_Pin   // Pagina 2 (PD14)

// ===== 5 LEDs RGB (cátodo común) =====
// Cada letra: 3 pines (R,G,B). Encender = pin HIGH.

typedef struct {
    GPIO_TypeDef* portR; uint16_t pinR;
    GPIO_TypeDef* portG; uint16_t pinG;
    GPIO_TypeDef* portB; uint16_t pinB;
} RGBLedPins;

static const RGBLedPins s_leds[LONGITUD_PALABRA] = {
    // LED 1 (Letra 1)
    {GPIOE, GPIO_PIN_8,  GPIOE, GPIO_PIN_9,  GPIOE, GPIO_PIN_10},
    // LED 2 (Letra 2)
    {GPIOE, GPIO_PIN_11, GPIOE, GPIO_PIN_12, GPIOE, GPIO_PIN_13},
    // LED 3 (Letra 3)
    {GPIOE, GPIO_PIN_14, GPIOE, GPIO_PIN_15, GPIOB, GPIO_PIN_10},
    // LED 4 (Letra 4)  --- ROJO en PC5 (porque PB11 era NC)
    {GPIOC, GPIO_PIN_5,  GPIOB, GPIO_PIN_12, GPIOC, GPIO_PIN_0},
    // LED 5 (Letra 5)
    {GPIOC, GPIO_PIN_1,  GPIOC, GPIO_PIN_2,  GPIOC, GPIO_PIN_3},
};

static void RGB_SetOff(uint8_t i)
{
    HAL_GPIO_WritePin(s_leds[i].portR, s_leds[i].pinR, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(s_leds[i].portG, s_leds[i].pinG, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(s_leds[i].portB, s_leds[i].pinB, GPIO_PIN_RESET);
}

static void RGB_SetColor(uint8_t i, uint8_t r, uint8_t g, uint8_t b)
{
    HAL_GPIO_WritePin(s_leds[i].portR, s_leds[i].pinR, r ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(s_leds[i].portG, s_leds[i].pinG, g ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(s_leds[i].portB, s_leds[i].pinB, b ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void UI_Init(void)
{
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
        RGB_SetOff(i);

    UI_SetPaginaIndicador(0);
}

void UI_MostrarResultado(const uint8_t resultado[LONGITUD_PALABRA])
{
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
        uint8_t st = resultado[i];

        if (st == 0xFF) { RGB_SetOff(i); continue; }	//no led encendido

        switch (st)
        {
        case LETRA_OK:      RGB_SetColor(i, 0, 1, 0); break; // Verde
        case LETRA_MAL_POS: RGB_SetColor(i, 0, 0, 1); break; // Azul
        case LETRA_MAL:
        default:            RGB_SetColor(i, 1, 0, 0); break; // Rojo
        }
    }
}

void UI_SetPaginaIndicador(uint8_t pagina)
{
    HAL_GPIO_WritePin(PAGE_LED0_GPIO_PORT, PAGE_LED0_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PAGE_LED1_GPIO_PORT, PAGE_LED1_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PAGE_LED2_GPIO_PORT, PAGE_LED2_GPIO_PIN, GPIO_PIN_RESET);

    switch (pagina)
    {
    case 0: HAL_GPIO_WritePin(PAGE_LED0_GPIO_PORT, PAGE_LED0_GPIO_PIN, GPIO_PIN_SET); break;
    case 1: HAL_GPIO_WritePin(PAGE_LED1_GPIO_PORT, PAGE_LED1_GPIO_PIN, GPIO_PIN_SET); break;
    default: HAL_GPIO_WritePin(PAGE_LED2_GPIO_PORT, PAGE_LED2_GPIO_PIN, GPIO_PIN_SET); break;
    }
}

/*static void UI_SetLed(uint8_t index, uint8_t estado)
{
    GPIO_TypeDef* port;
    uint16_t pin;

    switch (index)
    {
    case 0: port = LED0_GPIO_PORT; pin = LED0_PIN; break;
    case 1: port = LED1_GPIO_PORT; pin = LED1_PIN; break;
    case 2: port = LED2_GPIO_PORT; pin = LED2_PIN; break;
    case 3: port = LED3_GPIO_PORT; pin = LED3_PIN; break;
    case 4: port = LED4_GPIO_PORT; pin = LED4_PIN; break;
    default: return;
    }

    // Por ahora usamos:
    // LETRA_OK      -> LED encendido
    // LETRA_MAL_POS -> LED parpadeo se puede hacer luego; de momento encendido también
    // LETRA_MAL     -> LED apagado
    if (estado == LETRA_MAL)
    {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    }
}*/
/*
static void set_color_from_estado(uint8_t idx, uint8_t estado)
{
    // 0xFF = sin información -> apagado
    if (estado == 0xFF) { WS2812_SetPixel(idx, 0, 0, 0); return; }

    switch (estado)
    {
    case LETRA_OK:      WS2812_SetPixel(idx, 0, 80, 0); break; // Verde
    case LETRA_MAL_POS: WS2812_SetPixel(idx, 0, 0, 80); break; // Azul
    case LETRA_MAL:
    default:            WS2812_SetPixel(idx, 80, 0, 0); break; // Rojo
    }
}


void UI_Init(void)
{
    // Nada especial: los pines ya están configurados por MX_GPIO_Init()
    // Apagamos todos por si acaso
   // uint8_t vacio[LONGITUD_PALABRA] = {LETRA_MAL, LETRA_MAL, LETRA_MAL, LETRA_MAL, LETRA_MAL};
    //UI_MostrarResultado(vacio);

	WS2812_Init(WS2812_GPIO_PORT, WS2812_GPIO_PIN, NUM_LEDS);
	WS2812_Clear();

    UI_SetPaginaIndicador(0);
}

void UI_MostrarResultado(const uint8_t resultado[LONGITUD_PALABRA])
{
    for (uint8_t i = 0; i < LONGITUD_PALABRA; i++)
    {
    	 set_color_from_estado(i, resultado[i]);  // IMPORTANTE: usar colores
    }
    WS2812_Show();
}

void UI_SetPaginaIndicador(uint8_t pagina)
{
    // Apaga todos
    HAL_GPIO_WritePin(PAGE_LED0_GPIO_PORT, PAGE_LED0_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PAGE_LED1_GPIO_PORT, PAGE_LED1_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PAGE_LED2_GPIO_PORT, PAGE_LED2_GPIO_PIN, GPIO_PIN_RESET);

    // Enciende el de la página
    switch (pagina)
    {
    case 0:
        HAL_GPIO_WritePin(PAGE_LED0_GPIO_PORT, PAGE_LED0_GPIO_PIN, GPIO_PIN_SET);
        break;
    case 1:
        HAL_GPIO_WritePin(PAGE_LED1_GPIO_PORT, PAGE_LED1_GPIO_PIN, GPIO_PIN_SET);
        break;
    default:
        HAL_GPIO_WritePin(PAGE_LED2_GPIO_PORT, PAGE_LED2_GPIO_PIN, GPIO_PIN_SET);
        break;
    }
}*/
