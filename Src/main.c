/**
 * @file main.c
 * @brief Configuración y control de periféricos del microcontrolador LPC1769.
 *
 * Este código configura y controla diversos periféricos del LPC1769,
 * incluyendo ADC, DAC, GPIO, PWM, UART, SYSTICK, TIMER, y GPDMA.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "lpc17xx_adc.h"     /**< Librería para manejo del ADC */
#include "lpc17xx_dac.h"     /**< Librería para manejo del DAC */
#include "lpc17xx_gpdma.h"   /**< Librería para manejo de GPDMA */
#include "lpc17xx_gpio.h"    /**< Librería para manejo de GPIO */
#include "lpc17xx_nvic.h"    /**< Librería para manejo de NVIC */
#include "lpc17xx_pinsel.h"  /**< Librería para configuración de pines */
#include "lpc17xx_pwm.h"     /**< Librería para manejo de PWM */
#include "lpc17xx_systick.h" /**< Librería para manejo de SYSTICK */
#include "lpc17xx_timer.h"   /**< Librería para manejo de temporizadores */
#include "lpc17xx_uart.h"    /**< Librería para manejo de UART */
#include "stdio.h"
#include "system_LPC17xx.h" /**< Librería para manejo del sistema del LPC1769 */

/** @defgroup Pines Pines asociados a periféricos
 * @{
 */
#define PIN_ADC0_CH0 ((uint32_t)(1 << 23))     /**< P0.23 - Canal 0 del ADC */
#define PIN_ADC0_CH1 ((uint32_t)(1 << 24))     /**< P0.24 - Canal 1 del ADC */
#define PIN_ADC0_CH2 ((uint32_t)(1 << 25))     /**< P2.10 - Canal 2 del ADC */
#define PIN_PWM1 ((uint32_t)(1 << 18))         /**< P1.18 - PWM */
#define PIN_DAC ((uint32_t)(1 << 26))          /**< P0.26 - DAC */
#define PIN_LED_VERDE ((uint32_t)(1 << 4))     /**< P0.04 - LED verde */
#define PIN_LED_ROJO ((uint32_t)(1 << 5))      /**< P0.05 - LED rojo */
#define PIN_LED_UART ((uint32_t)(1 << 6))      /**< P0.06 - LED UART */
#define PIN_BOTON_PUERTA ((uint32_t)(1 << 13)) /**< P2.13 - Botón de puerta */
#define PIN_SALIDA_UART ((uint32_t)(1 << 2))   /**< P0.02 - Salida UART */
#define PIN_DIR_MPAP                                                           \
  ((uint32_t)(1 << 7)) /**< P0.07 - Dirección motor paso a paso */
/** @} */

/** @defgroup Valores de configuración de tiempos
 * @{
 */
#define VALOR_PRESCALER 100    /**< Prescaler para temporizador - 100 us */
#define MATCH0_TIM0 10000      /**< Match0 - 1 segundo */
#define VAL_SYSTICK 100        /**< Valor de SYSTICK - 100 ms */
#define VAL_TIEMPO_DAC 250000  /**< Tiempo de salida del DAC - 10 ms */
#define VAL_PRESCALER_PWM 1    /**< Prescaler del PWM - 1 us */
#define VAL_PERIODO_PWM 2000   /**< Periodo del PWM - 2 ms */
#define VAL_DUTYCICLE_PWM 1000 /**< Duty cycle del PWM - 1 ms */
/** @} */

/** @defgroup Estados del sistema
 * @{
 */
#define ON 1       /**< Encendido */
#define OFF 0      /**< Apagado */
#define ABRIR 1    /**< Abrir */
#define CERRAR 0   /**< Cerrar */
#define PWM1_MR0 0 /**< Canal PWM 0 */
#define PWM1_MR1 1 /**< Canal PWM 1 */
/** @} */

/** @defgroup Frecuencias del sistema
 * @{
 */
#define FREQ_ADC 100000   /**< Frecuencia del ADC - 100 kHz */
#define UART_BAUDIOS 9600 /**< Frecuencia UART - 9600 bps */
/** @} */

/** @brief Variables globales del sistema */
volatile uint8_t Datos[4]; /**< Buffer de datos del sistema (puerta,
                              temperatura, luz, gas) */
volatile uint16_t
    Conversiones[3];             /**< Buffer para valores convertidos por ADC */
volatile uint16_t Valor_DAC = 0; /**< Valor actual del DAC */
volatile uint8_t Count_PWM = 0;  /**< Contador para PWM */

/** @brief Prototipos de funciones */
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

/**
 * @brief Punto de entrada principal.
 *
 * Configura los periféricos y ejecuta el ciclo principal.
 *
 * @return 0 Si se ejecuta correctamente.
 */
int main(void) {
  SystemInit(); /**< Inicializa el sistema */

  Config_GPIO();
  Config_SYSTICK();
  Config_TIMER0();
  Config_ADC();
  Config_DAC();
  Config_PWM();
  Config_UART();
  Config_GPDMA();

  while (TRUE) {
    /* Ciclo principal */
  }

  return 0;
}

/**
 * @brief Configuración del GPDMA.
 *
 * Esta función inicializa el DMA del LPC1769, configurando los canales
 * y estructuras necesarias para la transferencia de datos entre periféricos.
 */
void Config_GPDMA(void) {
  GPDMA_Channel_CFG_Type ChannelCfg0; /**< Configuración para el canal 0 */
  GPDMA_Channel_CFG_Type ChannelCfg1; /**< Configuración para el canal 1 */

  GPDMA_LLI_Type ListADC; /**< Configuración de lista vinculada para el ADC */

  // Configuración de la Lista de Transferencias
  ListADC.DstAddr = (uint16_t)&Conversiones[0];
  ListADC.SrcAddr = GPDMA_CONN_ADC;
  ListADC.NextLLI = 0;
  ListADC.Control = (3 << 0) | (1 << 18) | (1 << 21) | (1 << 27);

  // Inicialización del GPDMA
  GPDMA_Init();

  // Configuración del Canal 0 (Periférico a memoria)
  ChannelCfg0.ChannelNum = 0;
  ChannelCfg0.TransferType = GPDMA_TRANSFERTYPE_P2M;
  ChannelCfg0.TransferSize = 0;
  ChannelCfg0.SrcConn = GPDMA_CONN_ADC;
  ChannelCfg0.DstMemAddr = (uint32_t)&Conversiones[0];
  ChannelCfg0.DMALLI = (uint32_t)&ListADC;

  // Configuración del Canal 1 (Memoria a periférico)
  ChannelCfg1.ChannelNum = 1;
  ChannelCfg1.TransferType = GPDMA_TRANSFERTYPE_M2P;
  ChannelCfg1.TransferSize = 1;
  ChannelCfg1.SrcMemAddr = (uint32_t)&Valor_DAC;
  ChannelCfg1.DstConn = GPDMA_CONN_DAC;
  ChannelCfg1.DMALLI = 0;

  // Configuración de los canales
  GPDMA_Setup(&ChannelCfg0);
  GPDMA_Setup(&ChannelCfg1);

  // Habilitación de los canales
  GPDMA_ChannelCmd(0, ENABLE);
  GPDMA_ChannelCmd(1, ENABLE);
}

/**
 * @brief Configuración del ADC.
 *
 * Esta función inicializa y habilita los canales del ADC para la adquisición de
 * datos analógicos, además de habilitar las interrupciones correspondientes.
 */
void Config_ADC(void) {
  ADC_Init(
      LPC_ADC,
      FREQ_ADC); /**< Inicialización del ADC con una frecuencia especificada */

  // Habilitar canales ADC
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);

  // Configurar interrupciones de los canales
  ADC_IntConfig(LPC_ADC, ADC_CHANNEL_0, ENABLE);
  ADC_IntConfig(LPC_ADC, ADC_CHANNEL_1, ENABLE);
  ADC_IntConfig(LPC_ADC, ADC_CHANNEL_2, ENABLE);
}

/**
 * @brief Configuración de UART.
 *
 * Inicializa la UART con los parámetros de configuración especificados,
 * habilitando el modo FIFO y las interrupciones para transmisión.
 */
void Config_UART(void) {
  UART_CFG_Type uart; /**< Estructura para configuración de UART */

  // Configuración de parámetros básicos
  uart.Baud_rate = UART_BAUDIOS;
  uart.Databits = UART_DATABIT_8;
  uart.Parity = UART_PARITY_NONE;
  uart.Stopbits = UART_STOPBIT_1;

  UART_Init(LPC_UART2, &uart); /**< Inicialización de UART2 */

  UART_FIFO_CFG_Type fifo; /**< Configuración del FIFO */

  // Configuración del FIFO
  fifo.FIFO_DMAMode = ENABLE;
  fifo.FIFO_Level = UART_FIFO_TRGLEV2;
  fifo.FIFO_ResetTxBuf = ENABLE;

  UART_FIFOConfig(LPC_UART2, &fifo);

  // Habilitar transmisión e interrupciones
  UART_TxCmd(LPC_UART2, ENABLE);
  UART_IntConfig(LPC_UART2, UART_INTCFG_THRE, ENABLE);
}

/**
 * @brief Configuración del PWM.
 *
 * Configura el PWM para generar señales con un duty cycle y periodo específico,
 * utilizando un único canal en modo de borde simple.
 */
void Config_PWM(void) {
  PWM_TIMERCFG_Type PWMCfg; /**< Configuración del temporizador PWM */
  PWM_MATCHCFG_Type
      match0; /**< Configuración del registro de coincidencia PWM */
  PINSEL_CFG_Type PinCgf; /**< Configuración de los pines PWM */

  // Configuración del pin PWM
  PinCgf.Portnum = PINSEL_PORT_1;
  PinCgf.Pinnum = PINSEL_PIN_18;
  PinCgf.Funcnum = PINSEL_FUNC_2;
  PinCgf.Pinmode = PINSEL_PINMODE_PULLDOWN;
  PinCgf.OpenDrain = PINSEL_PINMODE_NORMAL;

  PINSEL_ConfigPin(&PinCgf);

  // Configuración del temporizador PWM
  PWMCfg.PrescaleOption = PWM_TIMER_PRESCALE_USVAL;
  PWMCfg.PrescaleValue = VAL_PRESCALER_PWM;

  PWM_Init(LPC_PWM1, PWM_MODE_TIMER, &PWMCfg);

  // Configuración del periodo y duty cycle
  PWM_MatchUpdate(LPC_PWM1, PWM1_MR0, VAL_PERIODO_PWM, PWM_MATCH_UPDATE_NOW);
  PWM_MatchUpdate(LPC_PWM1, PWM1_MR1, VAL_DUTYCICLE_PWM, PWM_MATCH_UPDATE_NOW);

  match0.MatchChannel = 0;
  match0.IntOnMatch = ENABLE;
  match0.ResetOnMatch = ENABLE;

  PWM_ConfigMatch(LPC_PWM1, &match0);

  // Configuración del canal y habilitación
  PWM_ChannelConfig(LPC_PWM1, 1, PWM_CHANNEL_SINGLE_EDGE);
  PWM_ChannelCmd(LPC_PWM1, 1, ENABLE);
}

/**
 * @brief Manejador de interrupción del PWM.
 *
 * Este manejador desactiva el PWM después de 100 pulsos.
 */
void PWM1_IRQHandler(void) {
  Count_PWM++;

  if (Count_PWM == 100) {
    PWM_Cmd(LPC_PWM1, DISABLE);
    Count_PWM = 0;
  }

  PWM_ClearIntPending(LPC_PWM1, PWM_INTSTAT_MR0);
}

/**
 * @brief Manejador de interrupción de SYSTICK.
 *
 * Envía los datos a través de UART cada vez que se genera una interrupción.
 */
void Systick_Handler(void) {
  UART_Send(LPC_UART0, &Datos, 4, NONE_BLOCKING);
  SYSTICK_ClearCounterFlag();
}