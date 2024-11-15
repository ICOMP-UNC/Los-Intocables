
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

void Config_ADC(void){
    ADC_Init (LPC_ADC, FREQ_ADC);

    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);

    ADC_IntConfig(LPC_ADC, ADC_CHANNEL_0, ENABLE);
    ADC_IntConfig(LPC_ADC, ADC_CHANNEL_1, ENABLE);
    ADC_IntConfig(LPC_ADC, ADC_CHANNEL_2, ENABLE);
}

void Config_UART(void){
    UART_CFG_Type uart;

    uart.Baud_rate = UART_BAUDIOS;
    uart.Databits = UART_DATABIT_8;
    uart.Parity = UART_PARITY_NONE;
    uart.Stopbits = UART_STOPBIT_1;

    UART_Init(LPC_UART2, &uart);

    UART_FIFO_CFG_Type fifo;

    fifo.FIFO_DMAMode = ENABLE;
    fifo.FIFO_Level = UART_FIFO_TRGLEV2;
    fifo.FIFO_ResetTxBuf = ENABLE;

    UART_FIFOConfig(LPC_UART2, &fifo);

    UART_TxCmd(LPC_UART2, ENABLE);

    UART_IntConfig(LPC_UART2, UART_INTCFG_THRE, ENABLE);

}






