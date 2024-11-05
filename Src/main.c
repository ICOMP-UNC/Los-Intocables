
#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "system_LPC17xx.h"         /* Handler del sistema de LPC1769 */
#include "lpc17xx_pinsel.h"         /* Handler PINSEL */
#include "lpc17xx_gpio.h"           /* Handler GPIO */
#include "lpc17xx_systick.h"        /* Handler SYSTICK */
#include "lpc17xx_timer.h"          /* Handler TIMER */
#include "lpc17xx_adc.h"            /* Handler ADC */
#include "lpc17xx_dac.h"            /* Handler DAC */
#include "lpc17xx_gpdma.h"          /* Handler GPDMA */
#include "lpc17xx_nvic.h"           /* Handler NVIC */
#include "lpc17xx_pwm.h"            /* Handler PWM */
#include "lpc17xx_uart.h"           /* Handler UART */
#include "stdio.h"

#define PIN_ADC0_CH0        ((uint32_t)(1<<23))     // P0.23 Pin ADC canal 0 - Sensor temperatura
#define PIN_ADC0_CH1        ((uint32_t)(1<<24))     // P0.24 Pin ADC canal 1 - Sensor de luz
#define PIN_ADC0_CH2        ((uint32_t)(1<<25))     // P2.10 Pin ADC canal 2 - Sensor monoxido de carbono
#define PIN_PWM1            ((uint32_t)(1<<18))     // P1.18 Pin PWM
#define PIN_DAC             ((uint32_t)(1<<26))     // P0.26 Pin DAC
#define PIN_LED_VERDE       ((uint32_t)(1<<4))      // P0.04 Pin led verde
#define PIN_LED_ROJO        ((uint32_t)(1<<5))      // P0.05 Pin led rojo
#define PIN_LED_UART        ((uint32_t)(1<<6))      // P0.06 Pin led UART
#define PIN_BOTON_PUERTA    ((uint32_t)(1<<13))     // P2.13 Pin puerta
#define PIN_SALIDA_UART     ((uint32_t)(1<<2))      // P0.02 Pin salida UART
#define PIN_DIR_MPAP        ((uint32_t)(1<<7))      // P0.07 Pin control de direccion motor paso a paso

// Definiciones de tiempos:
#define VALOR_PRESCALER     100                     // Valor de prescaler - 100 uS
#define MATCH0_TIM0         10000                   // Valor del match0 - 10000 - 1S
#define VAL_SYSTICK         100                     // Valor del systick - 100 mS
#define VAL_TIEMPO_DAC      250000                  // Valor del tiempo de salida del DAC - 10mS

// Estados posibles:
#define ON                  1
#define OFF                 0
#define ABRIR               1
#define CERRAR              0

// Definiciones de frecuencia:
#define FREQ_ADC            100000                  // Frecuencia ADC - 100 kHz
#define UART_BAUDIOS        9600                    // Baudios UART - 9600 bps

// Variables globales:
uint8_t Datos[4];                                   // Buffer de datos - Estado puerta/Temperatura/luz/concentracion de gas

// Definicion de funciones:
void ToggleStatusDoor(void);
void DriverDoor(uint8_t opcion);
void LedRed(uint8_t estado);
void LedGreen(uint8_t estado);
void BlinkLed(void);
void CleanData(void);
void EnableFan(void);
void DisableFan(void);
void Config_GPIO(void);
void Config_SYSTICK(void);
void Config_TIMER0(void);
void Config_ADC(void);
void Config_DAC(void);
void Config_PWM(void);
void Config_UART(void);
void Config_GPDMA(void);

int main(void){

    SystemInit();

    Config_GPIO();
    Config_SYSTICK();
    Config_TIMER0();
    Config_ADC();
    Config_DAC();
    Config_PWM();
    Config_UART();
    Config_GPDMA();

    while (TRUE)
    {
        /* code */
    }
    
    return 0;

}




