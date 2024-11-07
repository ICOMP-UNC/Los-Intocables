
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

#define PIN_ADC0_CH0 ((uint32_t)(1 << 23))     // P0.23 Pin ADC canal 0
#define PIN_ADC0_CH1 ((uint32_t)(1 << 24))     // P0.24 Pin ADC canal 1
#define PIN_ADC0_CH2 ((uint32_t)(1 << 25))     // P2.10 Pin ADC canal 2
#define PIN_PWM1 ((uint32_t)(1 << 18))         // P1.18 Pin PWM
#define PIN_DAC ((uint32_t)(1 << 26))          // P0.26 Pin DAC
#define PIN_LED_VERDE ((uint32_t)(1 << 4))     // P0.04 Pin led verde
#define PIN_LED_ROJO ((uint32_t)(1 << 5))      // P0.05 Pin led rojo
#define PIN_LED_UART ((uint32_t)(1 << 6))      // P0.06 Pin led UART
#define PIN_BOTON_PUERTA ((uint32_t)(1 << 13)) // P2.13 Pin puerta
#define PIN_SALIDA_UART ((uint32_t)(1 << 2))   // P0.02 Pin salida UART
#define PIN_DIR_MPAP ((uint32_t)(1 << 7))      // P0.07 Pin dir motor

// Definiciones de tiempos:
#define VALOR_PRESCALER 100    // Valor de prescaler - 100 uS
#define MATCH0_TIM0 10000      // Valor del match0 - 10000 - 1S
#define VAL_SYSTICK 100        // Valor del systick - 100 mS
#define VAL_TIEMPO_DAC 250000  // Valor del tiempo de salida del DAC - 10mS
#define VAL_PRESCALER_PWM 1    // Valor de prescales del PWM - 1us
#define VAL_PERIODO_PWM 2000   // Valor del periodo 2 ms
#define VAL_DUTYCICLE_PWM 1000 // Valor del duty cicle 1 ms

// Estados posibles:
#define ON 1
#define OFF 0
#define ABRIR 1
#define CERRAR 0
#define PWM1_MR0 0
#define PWM1_MR1 1

// Definiciones de frecuencia:
#define FREQ_ADC 100000   // Frecuencia ADC - 100 kHz
#define UART_BAUDIOS 9600 // Baudios UART - 9600 bps

// Variables globales:
volatile uint8_t Datos[4]; // Buffer de datos - Estado
                           // puerta/Temperatura/luz/concentracion de gas
volatile uint16_t Conversiones[3]; // Buffer de datos recien convertidos
volatile uint16_t Valor_DAC = 0;   // Variable del valor del DAC
volatile uint8_t Count_PWM = 0;    // Contador de pulsos

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

void Config_GPDMA(void) {

  GPDMA_Channel_CFG_Type ChannelCfg0; // Estructura para canal 0
  GPDMA_Channel_CFG_Type ChannelCfg1; // Estructura para canal 1

  GPDMA_LLI_Type ListADC; // Estructura para lista del ADC

  // Configuración de la Lista de Transferencias:
  ListADC.DstAddr = (uint16_t)&Conversiones[0];
  ListADC.SrcAddr = GPDMA_CONN_ADC;
  ListADC.NextLLI = 0;
  ListADC.Control = (3 << 0) | (1 << 18) | (1 << 21) | (1 << 27);

  // Inicialización del GPDMA:
  GPDMA_Init();

  // Configuración de Canales de GPDMA:
  ChannelCfg0.ChannelNum = 0;
  ChannelCfg0.TransferType = GPDMA_TRANSFERTYPE_P2M;
  ChannelCfg0.TransferSize = 0;
  ChannelCfg0.SrcConn = GPDMA_CONN_ADC;
  ChannelCfg0.DstMemAddr = (uint16_t)&Conversiones[0];
  ChannelCfg0.DMALLI = (uint32_t)&ListADC;

  ChannelCfg1.ChannelNum = 1;
  ChannelCfg1.TransferType = GPDMA_TRANSFERTYPE_M2P;
  ChannelCfg1.TransferSize = 1;
  ChannelCfg1.SrcMemAddr = (uint16_t)&Valor_DAC;
  ChannelCfg1.DstConn = GPDMA_CONN_DAC;
  ChannelCfg1.DMALLI = 0;

  // Carga de canales de GPDMA:
  GPDMA_Setup(&ChannelCfg0);
  GPDMA_Setup(&ChannelCfg1);

  // Hablititacion de canales de GPDMA:
  GPDMA_ChannelCmd(0, ENABLE);
  GPDMA_ChannelCmd(1, ENABLE);
}

void Config_PWM(void) {
  PWM_TIMERCFG_Type PWMCfg;
  PWM_MATCHCFG_Type match0;
  PINSEL_CFG_Type PinCgf;

  // Cofiguracion pin PWM:
  PinCgf.Portnum = PINSEL_PORT_1;
  PinCgf.Pinnum = PINSEL_PIN_18;
  PinCgf.Funcnum = PINSEL_FUNC_2;
  PinCgf.Pinmode = PINSEL_PINMODE_PULLDOWN;
  PinCgf.OpenDrain = PINSEL_PINMODE_NORMAL;

  PINSEL_ConfigPin(&PinCgf);

  // Configuracion PWM:
  PWMCfg.PrescaleOption = PWM_TIMER_PRESCALE_USVAL;
  PWMCfg.PrescaleValue = VAL_PRESCALER_PWM;

  PWM_Init(LPC_PWM1, PWM_MODE_TIMER, &PWMCfg);

  PWM_MatchUpdate(LPC_PWM1, PWM1_MR0, VAL_PERIODO_PWM, PWM_MATCH_UPDATE_NOW);
  PWM_MatchUpdate(LPC_PWM1, PWM1_MR1, VAL_DUTYCICLE_PWM, PWM_MATCH_UPDATE_NOW);

  match0.MatchChannel = 0;
  match0.IntOnMatch = ENABLE;
  match0.ResetOnMatch = ENABLE;

  PWM_ConfigMatch(LPC_PWM1, &match0);

  PWM_ChannelConfig(LPC_PWM1, 1, PWM_CHANNEL_SINGLE_EDGE);

  PWM_ChannelCmd(LPC_PWM1, 1, ENABLE);
}

void PWM1_IRQHandler(void) {

  Count_PWM++;

  if (Count_PWM == 100) {
    PWM_Cmd(LPC_PWM1, DISABLE);
    Count_PWM = 0;
  }

  PWM_ClearIntPending(LPC_PWM1, PWM_INTSTAT_MR0);
}

void Systick_Handler(void) {

  UART_Send(LPC_UART0, &Datos, 4, NONE_BLOCKING);

  SYSTICK_ClearCounterFlag();
}
