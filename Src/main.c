#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#ifdef __USE_MCUEXPRESSO
#include <cr_section_macros.h> /* The cr_section_macros is specific to the MCUXpresso delivered toolchain */
#endif

// Librerias:
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "stdio.h"
#include "system_LPC17xx.h"

// Definicionde de pines:
#define LED_CONTROL_1                                                          \
  ((uint32_t)(1 << 0))               // P2.00 LED 1 PARA CONTROL DE SYSTICK
#define PIN_PWM ((uint32_t)(1 << 1)) // P2.01 LED 2 PARA CONTROL DE TIMER
#define LED_CONTROL_3 ((uint32_t)(1 << 2)) // P2.02 LED 3 PARA CONTROL DE ADC
#define LED_CONTROL_4 ((uint32_t)(1 << 3)) // P2.03 LED 4 PARA CONTROL DE DAC
#define LED_CONTROL_5                                                          \
  ((uint32_t)(1 << 4)) // P2.04 LED 5 PARA CONTROL DE INTERRUPCION EXTERNA
#define PIN_BOTON ((uint32_t)(1 << 13))     // P2.10 BOTON
#define PIN_ADC_C0 ((uint32_t)(1 << 23))    // P0.23 ADC CANAL 0
#define PIN_ADC_C1 ((uint32_t)(1 << 24))    // P0.24 ADC CANAL 1
#define PIN_ADC_C2 ((uint32_t)(1 << 25))    // P0.25 ADC CANAL 2
#define PIN_DAC ((uint32_t)(1 << 26))       // P0.26 DAC
#define PIN_DIRRECCION ((uint32_t)(1 << 5)) // P2.05 OIN DIRRECCION MOTOR

// Definiciones Systick:
#define SYSTICK_TIME 100 // Tiempo del Systick en ms

// Definiciones Timer:
#define TIMER0_PRESCALE_VALUE 100 // Valor del prescaler del timer en us
#define TIMER0_MATCH0_VALUE                                                    \
  20000 // Valor del match 0 del timer en cantidad de veces

// Definiciones ADC:
#define ADC_FREQ 200000 // Valor de la frecuencia de conversion del ADC en Hz

// Definiciones DAC:
#define DAC_FREQ 25000000 // Valor de la frecuencia de conversion del DAC en Hz

// Definiciones UART:
#define UART_BAUDIOS 9600

// Definiciones varias:
#define ON 1    // Estado del led - prender
#define OFF 0   // Estado del led - apagar
#define OPEN 1  // Accion de puerta - Abrir
#define CLOSE 0 // Accion de puerta - Cerrar

// Definiciones PWM:
#define PWM_PRESC 100
#define PWM_MATCH_1 20
#define PWM_MATCH_2 10

// Definiciones de mediciones de alerta
#define MAX_GAS_CONCENTRATION 50
#define MAX_TEMPERATURE 50
#define MIN_TEMPERATURE 5

// Definiciones de alerta:
#define WARNING 1
#define SAFE 0

// Declaracion de variables:
volatile uint32_t DAC_Value = 0; // Valor que va a ser transferido por el DAC
volatile uint32_t
    ADC_Results[3]; // Valores obtenidos de las convversiones del ADC
volatile uint8_t Data[4] = {} ;

// Declaracion de banderas:
volatile uint8_t DOOR_Flag = 0;
volatile uint8_t SYSTICK_Flag = 0;
volatile uint8_t TIMER0_Flag = 0;
volatile uint8_t ADC_Flag = 0;
volatile uint8_t UART_Flag = 0;
volatile uint8_t PWM_count = 0;
volatile uint8_t WARNING_Open_Flag = 0;
volatile uint8_t WARNING_Close_Flag = 0;
GPDMA_LLI_Type ADCList;

// Declaracion de funciones:
void Config_GPIO();
void Config_EINT();
void Config_PWM();
void Config_SYSTICK();
void Config_TIMER0();
void Config_ADC();
void Config_DAC();
void Config_UART();
void Config_GPDMA();
void Led_Control(uint8_t estado, uint32_t PIN_led);
void Motor_Activate(uint8_t action);
void Check_Measures();

int main(void) {

  SystemInit();
  Config_GPIO();
  Config_EINT();
  Config_ADC();
  Config_DAC();
  Config_UART();
  Config_SYSTICK();
  Config_TIMER0();

  TIM_Cmd(LPC_TIM0, ENABLE);
  SYSTICK_Cmd(ENABLE);

  Led_Control(OFF, LED_CONTROL_1);
  Led_Control(OFF, LED_CONTROL_3);
  Led_Control(OFF, LED_CONTROL_4);
  Led_Control(OFF, LED_CONTROL_5);

  Config_GPDMA();

  while (TRUE);

  return 0;
}

void Config_GPIO(void) {

  PINSEL_CFG_Type Pincfg;

  // Configuracion general del pinsel para los leds:
  Pincfg.Portnum = PINSEL_PORT_2;
  Pincfg.Funcnum = PINSEL_FUNC_0;
  Pincfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
  Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;

  // Configuracion pinsel led 1
  Pincfg.Pinnum = PINSEL_PIN_0;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion pinsel led 3
  Pincfg.Pinnum = PINSEL_PIN_2;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion pinsel led 4
  Pincfg.Pinnum = PINSEL_PIN_3;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion pinsel led 5
  Pincfg.Pinnum = PINSEL_PIN_4;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion pinsel salida de dirreccion:
  Pincfg.Pinnum = PINSEL_PIN_5;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion GPIO para los leds:
  GPIO_SetDir(PINSEL_PORT_2,
              LED_CONTROL_1 | LED_CONTROL_3 | LED_CONTROL_4 | LED_CONTROL_5 |
                  PIN_DIRRECCION,
              GPIO_DIR_OUTPUT);
}

void Config_EINT(void) {

  PINSEL_CFG_Type Pincfg;

  // Configuracion pinsel del pin dela interrupcion:
  Pincfg.Portnum = PINSEL_PORT_2;
  Pincfg.Pinnum = PINSEL_PIN_13;
  Pincfg.Funcnum = PINSEL_FUNC_1;
  Pincfg.Pinmode = PINSEL_PINMODE_PULLUP;
  Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&Pincfg);

  // Seteamos el pin como entarda
  GPIO_SetDir(PINSEL_PORT_0, PIN_BOTON, GPIO_DIR_INPUT);

  // Habilitamos la interrupcion por flanco ascendente
  GPIO_IntCmd(PINSEL_PORT_0, PIN_BOTON, ENABLE);

  // Inicializacion de las interrupciones externas:
  EXTI_Init();

  // Configuracion de la Iext:
  EXTI_InitTypeDef Eint3Cfg;

  Eint3Cfg.EXTI_Line = EXTI_EINT3;
  Eint3Cfg.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
  Eint3Cfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;

  EXTI_Config(&Eint3Cfg);

  // Habilitacion de las interrupciones esternas del NVIC:
  NVIC_EnableIRQ(EINT3_IRQn);
}

void Config_SYSTICK(void) {

  SYSTICK_InternalInit(SYSTICK_TIME);
  SYSTICK_IntCmd(ENABLE);
}

void Config_TIMER0(void) {

  // Configuracion TIMER0:
  TIM_TIMERCFG_Type TIM0;

  TIM0.PrescaleOption = TIM_PRESCALE_USVAL;
  TIM0.PrescaleValue = TIMER0_PRESCALE_VALUE;
  TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM0);

  // Configuracion del match0:
  TIM_MATCHCFG_Type Match0;

  Match0.MatchChannel = 0;
  Match0.IntOnMatch = ENABLE;
  Match0.ResetOnMatch = ENABLE;
  Match0.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
  Match0.StopOnMatch = DISABLE;
  Match0.MatchValue = TIMER0_MATCH0_VALUE;
  TIM_ConfigMatch(LPC_TIM0, &Match0);

  // Habilitacion de la interrupciones externas por parte del TIMER 0:
  NVIC_EnableIRQ(TIMER0_IRQn);
}

void Config_ADC(void) {

  // Configuracion pinsel del adc:
  PINSEL_CFG_Type Pincfg;
  Pincfg.Portnum = PINSEL_PORT_0;
  Pincfg.Funcnum = PINSEL_FUNC_1;
  Pincfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
  Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;

  // Configuracion pinsel ADC canal 0:
  Pincfg.Pinnum = PINSEL_PIN_23;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion pinsel ADC canal 1:
  Pincfg.Pinnum = PINSEL_PIN_24;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion pinsel ADC canal 2:
  Pincfg.Pinnum = PINSEL_PIN_25;
  PINSEL_ConfigPin(&Pincfg);

  // Inicializacion ADC:
  ADC_Init(LPC_ADC, ADC_FREQ);

  // Habilitacion de los canales:
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
  ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);

  // Habilitamos las interrupciones:
  ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, ENABLE);

  // Habilitamos el modo burst:
  ADC_BurstCmd(LPC_ADC, ENABLE);
}

void Config_DAC(void) {

  // Configuracion pinsel del dac:
  PINSEL_CFG_Type Pincfg;

  Pincfg.Portnum = PINSEL_PORT_0;
  Pincfg.Pinnum = PINSEL_PIN_26;
  Pincfg.Funcnum = PINSEL_FUNC_2;
  Pincfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
  Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&Pincfg);

  // Configuracion de la corriente de salida:
  DAC_SetBias(LPC_DAC, DAC_MAX_CURRENT_700uA);

  // Inicializacion del DAC:
  DAC_Init(LPC_DAC);
}

void Config_UART(void) {

  PINSEL_CFG_Type PinCfg;

  PinCfg.Portnum = PINSEL_PORT_0;
  PinCfg.Pinnum = PINSEL_PIN_10;
  PinCfg.Funcnum = PINSEL_FUNC_1;
  PinCfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
  PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&PinCfg);

  PinCfg.Pinnum = PINSEL_PIN_11;
  PINSEL_ConfigPin(&PinCfg);

  UART_CFG_Type uart;

  uart.Baud_rate = UART_BAUDIOS;
  uart.Databits = UART_DATABIT_8;
  uart.Parity = UART_PARITY_NONE;
  uart.Stopbits = UART_STOPBIT_1;

  UART_Init(LPC_UART2, &uart);

  UART_FIFO_CFG_Type fifo;

  fifo.FIFO_DMAMode = DISABLE;
  fifo.FIFO_Level = UART_FIFO_TRGLEV0;
  fifo.FIFO_ResetTxBuf = ENABLE;
  fifo.FIFO_ResetRxBuf = ENABLE;

  UART_FIFOConfig(LPC_UART2, &fifo);

  UART_TxCmd(LPC_UART2, ENABLE);

  UART_IntConfig(LPC_UART2, UART_INTCFG_THRE, ENABLE);

  NVIC_EnableIRQ(UART2_IRQn);
}

void Config_PWM(void) {

  // Configuracion pin PWM P0.01
  PINSEL_CFG_Type PinCfg;

  PinCfg.Portnum = PINSEL_PORT_2;
  PinCfg.Pinnum = PINSEL_PIN_1;
  PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
  PinCfg.Funcnum = PINSEL_FUNC_1;
  PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
  PINSEL_ConfigPin(&PinCfg);

  // Inicializacion PWM:
  PWM_TIMERCFG_Type PwmCfg;
  PwmCfg.PrescaleOption = PWM_TIMER_PRESCALE_USVAL;
  PwmCfg.PrescaleValue = PWM_PRESC;
  PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void *)&PwmCfg);

  // Configuracion de matchs:
  PWM_MATCHCFG_Type PwmMatch0;
  PwmMatch0.IntOnMatch = ENABLE;
  PwmMatch0.MatchChannel = 0;
  PwmMatch0.ResetOnMatch = ENABLE;
  PwmMatch0.StopOnMatch = DISABLE;
  PWM_ConfigMatch(LPC_PWM1, &PwmMatch0);

  PWM_MATCHCFG_Type PwmMatch2;
  PwmMatch2.IntOnMatch = DISABLE;
  PwmMatch2.MatchChannel = 2;
  PwmMatch2.ResetOnMatch = DISABLE;
  PwmMatch2.StopOnMatch = DISABLE;
  PWM_ConfigMatch(LPC_PWM1, &PwmMatch2);
  PWM_ChannelCmd(LPC_PWM1, 2, ENABLE);

  PWM_ChannelConfig(LPC_PWM1, 2, PWM_CHANNEL_SINGLE_EDGE);
  PWM_MatchUpdate(LPC_PWM1, 0, PWM_MATCH_1, PWM_MATCH_UPDATE_NOW);
  PWM_MatchUpdate(LPC_PWM1, 2, PWM_MATCH_2, PWM_MATCH_UPDATE_NOW);
  PWM_ResetCounter(LPC_PWM1);
  PWM_CounterCmd(LPC_PWM1, ENABLE);
  NVIC_EnableIRQ(PWM1_IRQn);
  PWM_Cmd(LPC_PWM1, ENABLE);
}

void Config_GPDMA(void) {

	// Inicializacion GPDMA:
	GPDMA_Init();
  // Configuracion de lista:

  ADCList.SrcAddr = (uint32_t)&(LPC_ADC->ADDR0);
  ADCList.DstAddr = (uint32_t)&ADC_Results[0];
  ADCList.NextLLI = (uint32_t)&ADCList;
  ADCList.Control = (3 << 0) | (2 << 18) | (2 << 21) | (1 << 26) | (1 << 27);

  // Configuracion canal:
  GPDMA_Channel_CFG_Type DMAChannel0;
  DMAChannel0.ChannelNum = 0;
  DMAChannel0.SrcMemAddr = (uint32_t)&(LPC_ADC->ADDR0);
  DMAChannel0.DstMemAddr = (uint32_t)&ADC_Results[0];
  DMAChannel0.TransferSize = 3;
  DMAChannel0.TransferWidth = 0;
  DMAChannel0.TransferType = GPDMA_TRANSFERTYPE_P2M;
  DMAChannel0.SrcConn = GPDMA_CONN_ADC;
  DMAChannel0.DstConn = 0;
  DMAChannel0.DMALLI = &ADCList; // (uint32_t)&ADCList;
  GPDMA_Setup(&DMAChannel0);
  GPDMA_ChannelCmd(0, ENABLE);

}

/* Funciones agregadas:
 *  Funcion encendido/apagado de los leds:
 */

void Led_Control(uint8_t estado, uint32_t PIN_led) {
  // Maneja el estado del led rojo
  if (estado == ON) {
    GPIO_SetValue(PINSEL_PORT_2, PIN_led);
  } else {
    GPIO_ClearValue(PINSEL_PORT_2, PIN_led);
  }
}

// Funcion de motor:
void Motor_Activate(uint8_t action) {
  if (action == OPEN && WARNING_Close_Flag == 0) {
    Config_PWM();
    Led_Control(ON, LED_CONTROL_5);
    DOOR_Flag = !DOOR_Flag;
    GPIO_SetValue(PINSEL_PORT_2,
                 PIN_DIRRECCION); // Agregar pin de salida de dirreccion.
  } else if (action == CLOSE && WARNING_Open_Flag == 0) {
    Config_PWM();
    Led_Control(OFF, LED_CONTROL_5);
    DOOR_Flag = !DOOR_Flag;
    GPIO_ClearValue(PINSEL_PORT_2,
                    PIN_DIRRECCION); // Agregar pin de salida de dirreccion.
  }
}

void EINT3_IRQHandler(void) {

  // Comprobacion del puerto
  if (GPIO_ReadValue(PINSEL_PORT_2) & PIN_BOTON) {
    // Control de led de EINT0 y seteamos el estatus de la puerta
    if (DOOR_Flag == 0) {
      Motor_Activate(OPEN);
    }

    else {
      Motor_Activate(CLOSE);
    }
  }

  // Limpiamos la bandera de la EINT:
  EXTI_ClearEXTIFlag(EXTI_EINT3);
}

void SysTick_Handler(void) {

	uint32_t adc_result_temp;
	adc_result_temp = (ADC_Results[0] & 0xFFF0) >> 4;
  // Calculamos el valor para enviar al DAC:
  DAC_Value = (uint32_t)((adc_result_temp) / 4);
  // Mandamos el valor por el DAC:
  DAC_UpdateValue(LPC_DAC, DAC_Value);

  // Control de led de control del systick:
  if (SYSTICK_Flag == 0) {
    Led_Control(ON, LED_CONTROL_1);
    SYSTICK_Flag = !SYSTICK_Flag;
  } else {
    Led_Control(OFF, LED_CONTROL_1);
    SYSTICK_Flag = !SYSTICK_Flag;
  }
  // Limpiamos la bandera del Systick:
  SYSTICK_ClearCounterFlag();
}

void TIMER0_IRQHandler(void) {
  uint32_t temp;
  
  for(int i = 0; i < 3; i++){
	  temp = ((0xFFF0)& ADC_Results[i])>>4;
	  Data[i] =(temp*100)/4096;
  }
  //Ajuste del valor de la temperatura

  Data[3] = DOOR_Flag;

  Check_Measures();

  if(WARNING_Close_Flag == WARNING){
    Motor_Activate(CLOSE);
  }
  if(WARNING_Open_Flag == WARNING){
    Motor_Activate(OPEN);
  }

  // Mandamos los valores por UART:
  UART_Send(LPC_UART2, Data, 4, BLOCKING);

  // Control led de control del timer 0:
  if (TIMER0_Flag == 0) {
    Led_Control(ON, LED_CONTROL_3);
    TIMER0_Flag = !TIMER0_Flag;
  } else {
    Led_Control(OFF, LED_CONTROL_3);
    TIMER0_Flag = !TIMER0_Flag;
  }
  // Limpiamos bandera del timer:
  TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}

void UART2_IRQHandler(void) {

  if (UART_GetIntId(LPC_UART2) & (1 << 1)) {
    if (UART_Flag == 0){
      Led_Control(ON, LED_CONTROL_4);
      UART_Flag = !UART_Flag;
    } else {
      Led_Control(OFF, LED_CONTROL_4);
      UART_Flag = !UART_Flag;
    }
  }
}

void PWM1_IRQHandler(void) {
  if (PWM_GetIntStatus(LPC_PWM1, PWM_INTSTAT_MR0) == SET) {
    PWM_count++;
    if (PWM_count == 49) {
      PWM_count = 0;
      PWM_MATCHCFG_Type PwmMatch0;
      PwmMatch0.IntOnMatch = ENABLE;
      PwmMatch0.MatchChannel = 0;
      PwmMatch0.ResetOnMatch = DISABLE;
      PwmMatch0.StopOnMatch = ENABLE;
      PWM_ConfigMatch(LPC_PWM1, &PwmMatch0);
    }
  }
  PWM_ClearIntPending(LPC_PWM1, PWM_INTSTAT_MR0);
}

void Check_Measures(){
  if(Data[2] > MAX_GAS_CONCENTRATION){
    WARNING_Close_Flag = SAFE;
    WARNING_Open_Flag = WARNING;
  }
  else if(Data[0] < MIN_TEMPERATURE){
    WARNING_Close_Flag = WARNING;
    WARNING_Open_Flag = SAFE;
  }
  else if(Data[0] > MAX_TEMPERATURE){
    WARNING_Close_Flag = SAFE;
    WARNING_Open_Flag = WARNING;
  }
  else{
    WARNING_Close_Flag = SAFE;
    WARNING_Open_Flag = SAFE;
  }
}
