#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "lpc17xx_adc.h"     /* Handler ADC */
#include "lpc17xx_dac.h"     /* Handler DAC */
#include "lpc17xx_gpdma.h"   /* Handler GPDMA */
#include "lpc17xx_gpio.h"    /* Handler GPIO */
#include "lpc17xx_nvic.h"    /* Handler NVIC */
#include "lpc17xx_pinsel.h"  /* Handler PINSEL */
#include "lpc17xx_pwm.h"     /* Handler PWM */
#include "lpc17xx_systick.h" /* Handler SYSTICK */
#include "lpc17xx_timer.h"   /* Handler TIMER */
#include "lpc17xx_uart.h"    /* Handler UART */
#include "stdio.h"
#include "system_LPC17xx.h" /* Handler del sistema de LPC1769 */

#define PIN_ADC0_CH0                                                           \
  ((uint32_t)(1 << 23)) // P0.23 Pin ADC canal 0 - Sensor temperatura
#define PIN_ADC0_CH1                                                           \
  ((uint32_t)(1 << 24)) // P0.24 Pin ADC canal 1 - Sensor de luz
#define PIN_ADC0_CH2                                                           \
  ((uint32_t)(1 << 25)) // P2.10 Pin ADC canal 2 - Sensor monoxido de carbono
#define PIN_PWM1 ((uint32_t)(1 << 18))         // P1.18 Pin PWM
#define PIN_DAC ((uint32_t)(1 << 26))          // P0.26 Pin DAC
#define PIN_LED_VERDE ((uint32_t)(1 << 4))     // P0.04 Pin led verde
#define PIN_LED_ROJO ((uint32_t)(1 << 5))      // P0.05 Pin led rojo
#define PIN_LED_UART ((uint32_t)(1 << 6))      // P0.06 Pin led UART
#define PIN_BOTON_PUERTA ((uint32_t)(1 << 13)) // P2.13 Pin puerta
#define PIN_SALIDA_UART ((uint32_t)(1 << 2))   // P0.02 Pin salida UART
#define PIN_DIR_MPAP                                                           \
  ((uint32_t)(1 << 7)) // P0.07 Pin control de direccion motor paso a paso

// Definiciones de tiempos:
#define VALOR_PRESCALER 100   // Valor de prescaler - 100 uS
#define MATCH0_TIM0 10000     // Valor del match0 - 10000 - 1S
#define VAL_SYSTICK 100       // Valor del systick - 100 mS
#define VAL_TIEMPO_DAC 250000 // Valor del tiempo de salida del DAC - 10mS

// Estados posibles:
#define ON 1
#define OFF 0
#define ENABLE 1
#define DISABLE 0
#define OPEN 1
#define CLOSE 0

// Tipos de flanco
#define FALLING_EDGE 1
#define RISING_EDGE 0

// Tipos de salida
#define OUTPUT 1
#define INPUT 0

// Definiciones de frecuencia:
#define FREQ_ADC 100000   // Frecuencia ADC - 100 kHz
#define FREQ_DAC 25000000 // Frecuencia por defecto del DAC 25MHz
#define UART_BAUDIOS 9600 // Baudios UART - 9600 bps

// Variables globales:
uint8_t Data[4]; // Buffer de datos - Estado
                 // puerta/Temperatura/luz/concentracion de gas

// Definicion de funciones:
void ToggleStatusDoor(void);
void DriverDoor(uint8_t estado);
void LedRed();
void LedGreen();
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
void Config_NVIC(void);

void ToggleStatusDoor() { // Alternamos el estado de la puerta

  if (Data[0] == OPEN) {
    Data[0] = CLOSE;
  } else {
    Data[0] = OPEN;
  }
}

void DriverDoor(uint8_t estado) {
  // implementar el pwm
}

void LedRed(void) {
  // Maneja el estado del led rojo
  GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_5);
}

void LedGreen(void) {
  // Maneja el estado del led verde
  GPIO_SetValue(PINSEL_PORT_0, PINSEL_PIN_4);
}

void Config_GPIO() {

  PINSEL_CFG_Type cfg_pin;

  // Configuracion led verde
  cfg_pin.Portnum = PINSEL_PORT_0;
  cfg_pin.Pinnum = PINSEL_PIN_4;
  cfg_pin.Funcnum = PINSEL_FUNC_0;
  cfg_pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
  cfg_pin.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&cfg_pin);

  // Configuracion led rojo
  cfg_pin.Portnum = PINSEL_PORT_0;
  cfg_pin.Pinnum = PINSEL_PIN_5;
  cfg_pin.Funcnum = PINSEL_FUNC_0;
  cfg_pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
  cfg_pin.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&cfg_pin);

  // Configuracion led transmision UART0
  cfg_pin.Portnum = PINSEL_PORT_0;
  cfg_pin.Pinnum = PINSEL_PIN_6;
  cfg_pin.Funcnum = PINSEL_FUNC_0;
  cfg_pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
  cfg_pin.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&cfg_pin);

  // Configuracion pin de direccion del motor paso a paso abrir/cerrar
  cfg_pin.Portnum = PINSEL_PORT_0;
  cfg_pin.Pinnum = PINSEL_PIN_7;
  cfg_pin.Funcnum = PINSEL_FUNC_0;
  cfg_pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
  cfg_pin.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&cfg_pin);

  // Seteamos los pines como salida
  GPIO_SetDir(PINSEL_PORT_0,
              PIN_LED_VERDE | PIN_LED_ROJO | PIN_LED_UART | PIN_DIR_MPAP,
              OUTPUT);

  // Configuracion del boton para la puerta
  cfg_pin.Portnum = PINSEL_PORT_2;
  cfg_pin.Pinnum = PINSEL_PIN_13;
  cfg_pin.Funcnum = PINSEL_FUNC_0;
  cfg_pin.Pinmode = PINSEL_PINMODE_PULLDOWN;
  cfg_pin.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&cfg_pin);

  // Seteamos el pin como entarda
  GPIO_SetDir(PINSEL_PORT_0, PIN_BOTON_PUERTA, INPUT);

  // Habilitamos la interrupcion por flanco ascendente
  GPIO_IntCmd(PINSEL_PORT_0, PIN_BOTON_PUERTA, RISING_EDGE);
}

void Config_DAC() {

  PINSEL_CFG_Type cfg_pin;
  DAC_CONVERTER_CFG_Type cfg_dac;

  uint32_t timeout;

  // Configuracion del pin como salida  DAC al ventilador
  cfg_pin.Portnum = PINSEL_PORT_0;
  cfg_pin.Pinnum = PINSEL_PIN_26;
  cfg_pin.Funcnum = PINSEL_FUNC_2;
  cfg_pin.Pinmode = PINSEL_PINMODE_TRISTATE; // no pull-up no pull-down
  cfg_pin.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&cfg_pin);

  // Configuracion general del DAC
  cfg_dac.DBLBUF_ENA = DISABLE;
  cfg_dac.CNT_ENA = ENABLE;
  cfg_dac.DMA_ENA = ENABLE;
  DAC_ConfigDAConverterControl(LPC_DAC, &cfg_dac);

  // Configuramos la salida del DAC en 350uA
  DAC_SetBias(LPC_ADC, DAC_MAX_CURRENT_350uA);

  // Configuramos el tiempo de actualizacion del DAC en 10ms
  timeout = (uint32_t)((1 / FREQ_DAC) * VAL_TIEMPO_DAC);
  DAC_SetDMATimeOut(LPC_DAC, timeout);

  // Inicializamos el DAC
  DAC_Init(LPC_DAC);
}

void EINT3_IRQHandler() {

  if (GPIO_GetIntStatus(
          PINSEL_PORT_2, PINSEL_PIN_13,
          RISING_EDGE)) { // Verificamos la bandera para el boton de la puerta

    GPIO_ClearInt(PINSEL_PORT_2, PINSEL_PIN_13); // Bajamos la bandera
    ToggleStatusDoor(); // Alternamos el valor del estado de la puerta

    if (Data[0] == OPEN) { // Si el estado esta en alto, abrimos la puerta

      DriverDoor(OPEN);
      LedRed();
      LedGreen();
    } else { // Si el estado esta en bajo, cerramos la puerta
      DriverDoor(CLOSE);
      LedRed();
      LedGreen();
    }
  }
}

int main(void) {

  SystemInit();

  Config_GPIO();
  Config_SYSTICK();
  Config_TIMER0();
  Config_ADC();
  Config_DAC();
  Config_PWM();
  Config_UART();
  Config_GPDMA();

  while (TRUE) {
    /* code */
  }

  return 0;
}
