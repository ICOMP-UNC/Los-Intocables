/**
 * @file main.c
 * @brief
 * @authors Verstraete, Enzo - Campos, Mariano - Testa, Lisandro - Madrid, Santiago
 * @date 2024-11-17
 * \image html images/Diagrama_ED3_TPF.drawio.png "Diagrama de flujo de todo el sistema(main)"
 */

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
#define LED_CONTROL_1  ((uint32_t)(1 << 0))  /**< P2.00 LED 1 PARA CONTROL DE SYSTICK */
#define PIN_PWM        ((uint32_t)(1 << 1))  /**< P2.01 SALIDA PWM CANAL 2 */
#define LED_CONTROL_3  ((uint32_t)(1 << 2))  /**< P2.02 LED 3 PARA CONTROL DEL TIMER 0 */
#define LED_CONTROL_4  ((uint32_t)(1 << 3))  /**< P2.03 LED 4 PARA CONTROL DEL UART2 */
#define LED_CONTROL_5  ((uint32_t)(1 << 4))  /**< P2.04 LED 5 PARA CONTROL DE LA VENTILACION */
#define PIN_BOTON      ((uint32_t)(1 << 13)) /**< P2.10 BOTON */
#define PIN_ADC_C0     ((uint32_t)(1 << 23)) /**< P0.23 ADC CANAL 0 */
#define PIN_ADC_C1     ((uint32_t)(1 << 24)) /**< P0.24 ADC CANAL 1 */
#define PIN_ADC_C2     ((uint32_t)(1 << 25)) /**< P0.25 ADC CANAL 2 */
#define PIN_DAC        ((uint32_t)(1 << 26)) /**< P0.26 DAC */
#define PIN_DIRRECCION ((uint32_t)(1 << 5))  /**< P2.05 OIN DIRRECCION MOTOR */

// Definiciones Systick:
#define SYSTICK_TIME 100 /**< Tiempo del Systick en ms */

// Definiciones Timer:
#define TIMER0_PRESCALE_VALUE 100   /**< Valor del prescaler del timer en us */
#define TIMER0_MATCH0_VALUE   20000 /**< Valor del match 0 del timer en cantidad de veces */

// Definiciones ADC:
#define ADC_FREQ 200000 /**< Valor de la frecuencia de conversion del ADC en Hz */

// Definiciones DAC:
#define DAC_FREQ 25000000 /**< Valor de la frecuencia de conversion del DAC en Hz */

// Definiciones UART:
#define UART_BAUDIOS 9600

// Definiciones PWM:
#define PWM_PRESC          100 /**< PWM valor de prescaler */
#define PWM_MATCH_0_VALUE  10  /**< PWM valor del match 0 */
#define PWM_MATCH_2_VALUE  5   /**< PWM valor del match 2 */
#define PWM_PULSE_QUANTITY 49  /**< PWM catidad de ciclos */

// Definiciones de estados:
#define ON    1 /**< Estado del led - prender */
#define OFF   0 /**< Estado del led - apagar */
#define OPEN  1 /**< Accion de puerta - Abrir */
#define CLOSE 0 /**< Accion de puerta - Cerrar */

// Definiciones de mediciones de alerta
#define MAX_GAS_CONCENTRATION 50 /**< Limite de concentracion de gas */
#define MAX_TEMPERATURE       50 /**< Limite de temperatura */
#define MIN_TEMPERATURE       5  /**< Minimo de temperatura */

// Definiciones de alerta:
#define WARNING 1 /**< Estado de advertencia */
#define SAFE    0 /**< Estado seguro */

// Declaracion de variables:
volatile uint32_t DAC_Value = 0;  /**< Valor que va a ser transferido por el DAC */
volatile uint32_t ADC_Results[3]; /**< Valores obtenidos de las convversiones del ADC */
volatile uint8_t Data[4];         /**< Arreglo para almacenar datos a enviar por UART */
volatile uint8_t PWM_count = 0;   /**< Contador de pulsos de PWM */
GPDMA_LLI_Type ADCList;           /**< Declaracion lista del GPDMA */

// Declaracion de banderas:
volatile uint8_t DOOR_Flag = 0;          /**< Bandera de la ventilacion */
volatile uint8_t SYSTICK_Flag = 0;       /**< Bandera del SYSTICK */
volatile uint8_t TIMER0_Flag = 0;        /**< Bandera del TIMER 0 */
volatile uint8_t ADC_Flag = 0;           /**< Bandera del ADC */
volatile uint8_t UART_Flag = 0;          /**< Bandera del UART2 */
volatile uint8_t WARNING_Open_Flag = 0;  /**< Bandera de apertura de ventilacion */
volatile uint8_t WARNING_Close_Flag = 0; /**< Bandera de cierre de ventilacion */

// Declaración de funciones de configuración de los periféricos y control
void Config_GPIO();                                 // Configuración de GPIO
void Config_EINT();                                 // Configuración de interrupciones externas
void Config_PWM();                                  // Configuración del PWM
void Config_SYSTICK();                              // Configuración del Systick
void Config_TIMER0();                               // Configuración del Timer 0
void Config_ADC();                                  // Configuración del ADC
void Config_DAC();                                  // Configuración del DAC
void Config_UART();                                 // Configuración del UART
void Config_GPDMA();                                // Configuración del GPDMA (DMA de datos)
void Led_Control(uint8_t estado, uint32_t PIN_led); // Función para controlar los LEDs
void Motor_Activate(uint8_t action);                // Función para activar el motor (abrir/cerrar puerta)
void Check_Measures();                              // Función para verificar las mediciones y condiciones de alerta

// Función principal: configuración inicial y ejecución continua
int main(void)
{
    SystemInit(); // Inicialización del sistema (frecuencia del reloj y demás configuraciones)

    // Configuración de periféricos
    Config_GPIO();    // Configura los pines GPIO
    Config_EINT();    // Configura las interrupciones externas
    Config_ADC();     // Configura el ADC
    Config_DAC();     // Configura el DAC
    Config_UART();    // Configura la UART
    Config_SYSTICK(); // Configura el Systick
    Config_TIMER0();  // Configura el Timer 0

    // Apagar los LEDs de control al inicio
    Led_Control(OFF, LED_CONTROL_1);
    Led_Control(OFF, LED_CONTROL_3);
    Led_Control(OFF, LED_CONTROL_4);
    Led_Control(OFF, LED_CONTROL_5);

    // Habilitar el Timer 0 y Systick para su ejecución
    TIM_Cmd(LPC_TIM0, ENABLE);
    SYSTICK_Cmd(ENABLE);

    // Configurar el GPDMA (DMA para ADC)
    Config_GPDMA();

    // Bucle principal: ejecuta el sistema de forma continua
    while (TRUE);

    return 0;
}

/**
 * @brief Configura los pines GPIO para los LEDs y la dirección del motor.
 *
 * Configura los pines del puerto 2 para el control de los LEDs y la señal de dirección
 * del motor como salidas. Establece la configuración del PINSEL para seleccionar el
 * modo adecuado de los pines.
 */
void Config_GPIO(void)
{
    PINSEL_CFG_Type Pincfg;

    // Configuración general del PINSEL para los LEDs (función de GPIO, con modo pull-down):
    Pincfg.Portnum = PINSEL_PORT_2;
    Pincfg.Funcnum = PINSEL_FUNC_0;
    Pincfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
    Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;

    // Configuración PINSEL para el LED 1 (P2.0)
    Pincfg.Pinnum = PINSEL_PIN_0;
    PINSEL_ConfigPin(&Pincfg);

    // Configuración PINSEL para el LED 3 (P2.2)
    Pincfg.Pinnum = PINSEL_PIN_2;
    PINSEL_ConfigPin(&Pincfg);

    // Configuración PINSEL para el LED 4 (P2.3)
    Pincfg.Pinnum = PINSEL_PIN_3;
    PINSEL_ConfigPin(&Pincfg);

    // Configuración PINSEL para el LED 5 (P2.4)
    Pincfg.Pinnum = PINSEL_PIN_4;
    PINSEL_ConfigPin(&Pincfg);

    // Configuración PINSEL para la salida de dirección del motor (P2.5)
    Pincfg.Pinnum = PINSEL_PIN_5;
    PINSEL_ConfigPin(&Pincfg);

    // Configuración GPIO para los LEDs y la salida de dirección del motor:
    GPIO_SetDir(
        PINSEL_PORT_2, LED_CONTROL_1 | LED_CONTROL_3 | LED_CONTROL_4 | LED_CONTROL_5 | PIN_DIRRECCION, GPIO_DIR_OUTPUT);
}

/**
 * @brief Configura la interrupción externa para el botón (P2.10).
 *
 * Configura el pin 2.10 para generar una interrupción externa por flanco ascendente
 * cuando se presiona el botón. Además, se configuran los pines para la interrupción
 * y se inicializa el NVIC para manejarla.
 */
void Config_EINT(void)
{
    PINSEL_CFG_Type Pincfg;

    // Configuración PINSEL para la interrupción externa en P2.10 (función 1 = EINT3):
    Pincfg.Portnum = PINSEL_PORT_2;
    Pincfg.Pinnum = PINSEL_PIN_13;
    Pincfg.Funcnum = PINSEL_FUNC_1;
    Pincfg.Pinmode = PINSEL_PINMODE_PULLUP;
    Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&Pincfg);

    // Configuramos el pin como entrada:
    GPIO_SetDir(PINSEL_PORT_0, PIN_BOTON, GPIO_DIR_INPUT);

    // Habilitamos la interrupción por flanco ascendente:
    GPIO_IntCmd(PINSEL_PORT_0, PIN_BOTON, ENABLE);

    // Inicialización de las interrupciones externas:
    EXTI_Init();

    // Configuración de la interrupción externa (EINT3):
    EXTI_InitTypeDef Eint3Cfg;
    Eint3Cfg.EXTI_Line = EXTI_EINT3;
    Eint3Cfg.EXTI_Mode = EXTI_MODE_LEVEL_SENSITIVE;
    Eint3Cfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
    EXTI_Config(&Eint3Cfg);

    // Habilitación de la interrupción externa en el NVIC:
    NVIC_EnableIRQ(EINT3_IRQn);
}

/**
 * @brief Configura el temporizador SYSTICK.
 *
 * Esta función inicializa el temporizador SYSTICK con un período de tiempo
 * predefinido, habilita la interrupción del SYSTICK para generar interrupciones
 * periódicas según el valor definido.
 */
void Config_SYSTICK(void)
{

    // Inicializa el SYSTICK con el valor de tiempo especificado (en ms)
    SYSTICK_InternalInit(SYSTICK_TIME);

    // Habilita las interrupciones del SYSTICK, lo que permitirá que se ejecute la rutina de interrupción cuando el
    // temporizador se agote.
    SYSTICK_IntCmd(ENABLE);
}

/**
 * @brief Configura el Timer 0 del LPC1769.
 *
 * Esta función configura el Timer 0 para que funcione con un prescaler dado
 * y un valor de match0, lo que generará una interrupción cuando el temporizador
 * alcance el valor especificado. Además, habilita la interrupción de Timer 0
 * en el NVIC.
 */
void Config_TIMER0(void)
{

    // Configuración del Timer 0
    TIM_TIMERCFG_Type TIM0;

    TIM0.PrescaleOption =
        TIM_PRESCALE_USVAL; // Se configura el prescaler del Timer 0 para que el conteo se realice en microsegundos
    TIM0.PrescaleValue = TIMER0_PRESCALE_VALUE; // El valor del prescaler se toma de una constante predefinida
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM0);  // Inicializa el Timer 0 en modo de temporizador

    // Configuración de la coincidencia (match) del Timer 0
    TIM_MATCHCFG_Type Match0;

    Match0.MatchChannel = 0;      // Se configura el canal de match en el canal 0
    Match0.IntOnMatch = ENABLE;   // Se habilita la interrupción cuando el contador del Timer alcance el valor de match
    Match0.ResetOnMatch = ENABLE; // Se reinicia el temporizador cuando se alcanza el valor de match
    Match0.ExtMatchOutputType =
        TIM_EXTMATCH_NOTHING;     // No se genera una salida de coincidencia (no se conecta a un pin de salida)
    Match0.StopOnMatch = DISABLE; // El temporizador no se detendrá automáticamente al alcanzar el valor de match
    Match0.MatchValue = TIMER0_MATCH0_VALUE; // Se establece el valor de match en una constante predefinida (el valor
                                             // con el que el temporizador genera una interrupción)

    // Configura el match del Timer 0 con los parámetros definidos
    TIM_ConfigMatch(LPC_TIM0, &Match0);

    // Habilita la interrupción del Timer 0 en el NVIC (manejador de interrupciones)
    NVIC_EnableIRQ(TIMER0_IRQn);
}

/**
 * @brief Configura el ADC para leer los canales 0, 1 y 2.
 *
 * Configura los pines de los canales del ADC, habilita los canales, y configura el ADC
 * para operar con una frecuencia especificada. Se habilitan las interrupciones y se configura
 * el modo de conversión en burst.
 */
void Config_ADC(void)
{
    // Configuración de los pines para los canales del ADC (función 1):
    PINSEL_CFG_Type Pincfg;
    Pincfg.Portnum = PINSEL_PORT_0;
    Pincfg.Funcnum = PINSEL_FUNC_1;
    Pincfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
    Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;

    // Configuración de los pines para los canales 0, 1 y 2:
    Pincfg.Pinnum = PINSEL_PIN_23; // Canal 0
    PINSEL_ConfigPin(&Pincfg);

    Pincfg.Pinnum = PINSEL_PIN_24; // Canal 1
    PINSEL_ConfigPin(&Pincfg);

    Pincfg.Pinnum = PINSEL_PIN_25; // Canal 2
    PINSEL_ConfigPin(&Pincfg);

    // Inicialización del ADC con la frecuencia especificada:
    ADC_Init(LPC_ADC, ADC_FREQ);

    // Habilitación de los canales del ADC:
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_1, ENABLE);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_2, ENABLE);

    // Habilitación de las interrupciones del ADC:
    ADC_IntConfig(LPC_ADC, ADC_ADGINTEN, ENABLE);

    // Activación del modo Burst para conversiones rápidas:
    ADC_BurstCmd(LPC_ADC, ENABLE);
}

/**
 * @brief Configura el DAC para la salida de señal analógica.
 *
 * Configura el pin del DAC (P0.26), establece la corriente de salida a 700uA y
 * inicializa el DAC para la conversión digital-analógica.
 */
void Config_DAC(void)
{
    // Configuración del PINSEL para el DAC (P0.26, función 2):
    PINSEL_CFG_Type Pincfg;
    Pincfg.Portnum = PINSEL_PORT_0;
    Pincfg.Pinnum = PINSEL_PIN_26;
    Pincfg.Funcnum = PINSEL_FUNC_2;
    Pincfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
    Pincfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&Pincfg);

    // Configuración de la corriente de salida del DAC (700uA):
    DAC_SetBias(LPC_DAC, DAC_MAX_CURRENT_700uA);

    // Inicialización del DAC:
    DAC_Init(LPC_DAC);
}

/**
 * @brief Configura el UART2 para la comunicación serial.
 *
 * Configura los pines de UART2 (P0.10 y P0.11) y la configuración del UART2
 * con una tasa de baudios, bits de datos, paridad y bits de parada definidos.
 * Configura los buffers FIFO y habilita las interrupciones.
 */
void Config_UART(void)
{
    // Configuración de los pines de UART2 (P0.10, P0.11):
    PINSEL_CFG_Type PinCfg;
    PinCfg.Portnum = PINSEL_PORT_0;
    PinCfg.Pinnum = PINSEL_PIN_10;
    PinCfg.Funcnum = PINSEL_FUNC_1;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLDOWN;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&PinCfg);

    PinCfg.Pinnum = PINSEL_PIN_11;
    PINSEL_ConfigPin(&PinCfg);

    // Configuración del UART2:
    UART_CFG_Type uart;
    uart.Baud_rate = UART_BAUDIOS;  // Configuración de la tasa de baudios
    uart.Databits = UART_DATABIT_8; // 8 bits de datos
    uart.Parity = UART_PARITY_NONE; // Sin paridad
    uart.Stopbits = UART_STOPBIT_1; // 1 bit de parada
    UART_Init(LPC_UART2, &uart);

    // Configuración de los FIFO de UART2:
    UART_FIFO_CFG_Type fifo;
    fifo.FIFO_DMAMode = DISABLE;
    fifo.FIFO_Level = UART_FIFO_TRGLEV0;
    fifo.FIFO_ResetTxBuf = ENABLE;
    fifo.FIFO_ResetRxBuf = ENABLE;
    UART_FIFOConfig(LPC_UART2, &fifo);

    // Habilitación de la transmisión UART:
    UART_TxCmd(LPC_UART2, ENABLE);

    // Habilitación de interrupciones por THRE (transmisión completada):
    UART_IntConfig(LPC_UART2, UART_INTCFG_THRE, ENABLE);

    // Habilitación de la interrupción UART2 en el NVIC:
    NVIC_EnableIRQ(UART2_IRQn);
}

/**
 * @brief Configura el PWM para la señal de control.
 *
 * Configura el pin de PWM (P2.1), inicializa el temporizador PWM, establece los valores de
 * los canales PWM y habilita las interrupciones para la actualización de los valores de PWM.
 */
void Config_PWM(void)
{
    // Configuración del pin de PWM (P2.1, función 1):
    PINSEL_CFG_Type PinCfg;
    PinCfg.Portnum = PINSEL_PORT_2;
    PinCfg.Pinnum = PINSEL_PIN_1;
    PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    PinCfg.Funcnum = PINSEL_FUNC_1;
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&PinCfg);

    // Inicialización del PWM:
    PWM_TIMERCFG_Type PwmCfg;
    PwmCfg.PrescaleOption = PWM_TIMER_PRESCALE_USVAL; // Preescala en microsegundos
    PwmCfg.PrescaleValue = PWM_PRESC;                 // Valor de prescaler
    PWM_Init(LPC_PWM1, PWM_MODE_TIMER, (void*)&PwmCfg);

    // Configuración de los matchs:
    PWM_MATCHCFG_Type PwmMatch0;
    PwmMatch0.IntOnMatch = ENABLE;   // Interrupción en coincidencia
    PwmMatch0.MatchChannel = 0;      // Canal 0
    PwmMatch0.ResetOnMatch = ENABLE; // Reset al alcanzar la coincidencia
    PwmMatch0.StopOnMatch = DISABLE; // No detener el PWM al alcanzar la coincidencia
    PWM_ConfigMatch(LPC_PWM1, &PwmMatch0);

    PWM_MATCHCFG_Type PwmMatch2;
    PwmMatch2.IntOnMatch = DISABLE;   // No interrupción en coincidencia
    PwmMatch2.MatchChannel = 2;       // Canal 2
    PwmMatch2.ResetOnMatch = DISABLE; // No reset al alcanzar la coincidencia
    PwmMatch2.StopOnMatch = DISABLE;  // No detener el PWM
    PWM_ConfigMatch(LPC_PWM1, &PwmMatch2);
    PWM_ChannelCmd(LPC_PWM1, 2, ENABLE);

    PWM_ChannelConfig(LPC_PWM1, 2, PWM_CHANNEL_SINGLE_EDGE);               // Configuración de canal de un solo borde
    PWM_MatchUpdate(LPC_PWM1, 0, PWM_MATCH_0_VALUE, PWM_MATCH_UPDATE_NOW); // Actualización inmediata del valor
    PWM_MatchUpdate(LPC_PWM1, 2, PWM_MATCH_2_VALUE, PWM_MATCH_UPDATE_NOW); // Actualización inmediata del valor
    PWM_ResetCounter(LPC_PWM1);                                            // Reseteo del contador PWM
    PWM_CounterCmd(LPC_PWM1, ENABLE);                                      // Habilitación del contador PWM
    NVIC_EnableIRQ(PWM1_IRQn);                                             // Habilitación de la interrupción del PWM1
    PWM_Cmd(LPC_PWM1, ENABLE);                                             // Habilitación del PWM
}

/**
 * @brief Configura el GPDMA para la transferencia de datos del ADC a un buffer en memoria.
 *
 * Inicializa el GPDMA y configura una lista enlazada de interrupción (LLI) para realizar
 * la transferencia de los resultados del ADC (canales 0, 1 y 2) hacia la memoria. Además, configura
 * el canal DMA para transferir datos desde el ADC hacia un arreglo de resultados.
 */
void Config_GPDMA(void)
{
    // Inicialización del GPDMA:
    GPDMA_Init();

    // Configuración de la lista de enlaces (LLI) para la transferencia de datos:
    ADCList.SrcAddr = (uint32_t)&(LPC_ADC->ADDR0); // Dirección de origen (registro ADDR0 del ADC)
    ADCList.DstAddr = (uint32_t)&ADC_Results[0];   // Dirección de destino (buffer de resultados)
    ADCList.NextLLI = (uint32_t)&ADCList;          // Apuntador al siguiente LLI (autocontinuación)
    ADCList.Control = (3 << 0) | (2 << 18) | (2 << 21) | (1 << 26) | (1 << 27); // Control de transferencia

    // Configuración del canal DMA:
    GPDMA_Channel_CFG_Type DMAChannel0;
    DMAChannel0.ChannelNum = 0;                           // Canal DMA 0
    DMAChannel0.SrcMemAddr = (uint32_t)&(LPC_ADC->ADDR0); // Dirección de origen del ADC
    DMAChannel0.DstMemAddr = (uint32_t)&ADC_Results[0];   // Dirección de destino (buffer de resultados)
    DMAChannel0.TransferSize = 3;                         // Tamaño de la transferencia (3 canales del ADC)
    DMAChannel0.TransferWidth = 0;                        // Ancho de datos (8 bits por defecto)
    DMAChannel0.TransferType = GPDMA_TRANSFERTYPE_P2M;    // Tipo de transferencia (periférico a memoria)
    DMAChannel0.SrcConn = GPDMA_CONN_ADC;                 // Conexión del origen (ADC)
    DMAChannel0.DstConn = 0;                              // No se usa conexión para el destino
    DMAChannel0.DMALLI = &ADCList;                        // Apunta a la lista de interrupciones (LLI)

    // Configura el canal DMA con los parámetros definidos:
    GPDMA_Setup(&DMAChannel0);

    // Habilita el canal DMA 0:
    GPDMA_ChannelCmd(0, ENABLE);
}

/**
 * @brief Controla el estado de un LED.
 *
 * Enciende o apaga el LED dependiendo del estado proporcionado.
 *
 * @param estado Estado del LED (ON o OFF).
 * @param PIN_led El pin específico para controlar el LED.
 */
void Led_Control(uint8_t estado, uint32_t PIN_led)
{
    if (estado == ON)
    {
        // Enciende el LED al ponerlo en alto:
        GPIO_SetValue(PINSEL_PORT_2, PIN_led);
    }
    else
    {
        // Apaga el LED al ponerlo en bajo:
        GPIO_ClearValue(PINSEL_PORT_2, PIN_led);
    }
}

/**
 * @brief Activa el motor y gestiona el control de la puerta.
 *
 * Dependiendo de la acción (OPEN o CLOSE), activa el motor de acuerdo con las señales
 * de dirección y controla el LED de estado. También cambia el estado de la puerta.
 *
 * @param action Acción a realizar (OPEN o CLOSE).
 */
void Motor_Activate(uint8_t action)
{
    if (action == OPEN && WARNING_Close_Flag == 0)
    {
        // Configura el pin de dirección y habilita el motor para abrir la puerta:
        GPIO_SetValue(PINSEL_PORT_2, PIN_DIRRECCION); // Dirección de apertura
        Config_PWM();                                 // Configuración del PWM para control del motor
        Led_Control(ON, LED_CONTROL_5);               // Enciende el LED de control
        DOOR_Flag = !DOOR_Flag;                       // Cambia el estado de la puerta
    }
    else if (action == CLOSE && WARNING_Open_Flag == 0)
    {
        // Configura el pin de dirección y habilita el motor para cerrar la puerta:
        GPIO_ClearValue(PINSEL_PORT_2, PIN_DIRRECCION); // Dirección de cierre
        Config_PWM();                                   // Configuración del PWM para control del motor
        Led_Control(OFF, LED_CONTROL_5);                // Apaga el LED de control
        DOOR_Flag = !DOOR_Flag;                         // Cambia el estado de la puerta
    }
}

/**
 * @brief Realiza el chequeo de las mediciones obtenidas de los sensores.
 *
 * Verifica si las mediciones de temperatura y concentración de gas se encuentran dentro de los
 * límites de seguridad. Si no es así, activa las advertencias correspondientes para abrir o cerrar la puerta.
 */
void Check_Measures(void)
{
    if (Data[2] > MAX_GAS_CONCENTRATION)
    {
        // Si la concentración de gas excede el límite, establece la advertencia de cierre:
        WARNING_Close_Flag = SAFE;
        WARNING_Open_Flag = WARNING;
    }
    else if (Data[0] < MIN_TEMPERATURE)
    {
        // Si la temperatura está por debajo del límite mínimo, establece la advertencia de apertura:
        WARNING_Close_Flag = WARNING;
        WARNING_Open_Flag = SAFE;
    }
    else if (Data[0] > MAX_TEMPERATURE)
    {
        // Si la temperatura está por encima del límite máximo, establece la advertencia de cierre:
        WARNING_Close_Flag = SAFE;
        WARNING_Open_Flag = WARNING;
    }
    else
    {
        // Si todo está dentro de los límites, establece ambos flags como seguros:
        WARNING_Close_Flag = SAFE;
        WARNING_Open_Flag = SAFE;
    }
}

/**
 * @brief Handler de la interrupción externa EINT3.
 *
 * Este handler se ejecuta cuando se detecta un evento en el pin asociado con la interrupción externa (EINT3).
 * Se comprueba el estado del botón y, dependiendo del estado de la puerta, se activa o desactiva el motor.
 *
 * @note Se limpia la bandera de la interrupción después de procesar el evento.
 */
void EINT3_IRQHandler(void)
{
    // Comprobación del estado del botón (si está presionado):
    if (GPIO_ReadValue(PINSEL_PORT_2) & PIN_BOTON)
    {
        // Si la puerta está cerrada, se abre, y viceversa
        if (DOOR_Flag == 0)
        {
            Motor_Activate(OPEN); // Abrir la puerta
        }
        else
        {
            Motor_Activate(CLOSE); // Cerrar la puerta
        }
    }

    // Limpiamos la bandera de la interrupción externa EINT3:
    EXTI_ClearEXTIFlag(EXTI_EINT3);
}

/**
 * @brief Handler del temporizador SysTick.
 *
 * Este handler se ejecuta cuando el temporizador SysTick alcanza el valor configurado.
 * Realiza el procesamiento de los resultados del ADC y actualiza el DAC con el valor calculado.
 * También gestiona el encendido y apagado del LED asociado al SysTick.
 *
 * @note Se limpia la bandera de la interrupción del SysTick después de procesar el evento.
 */
void SysTick_Handler(void)
{
    uint32_t adc_result_temp;

    // Se extrae el resultado del ADC para el canal 0, desplazando 4 bits a la derecha:
    adc_result_temp = (ADC_Results[0] & 0xFFF0) >> 4;

    // Se calcula el valor a enviar al DAC:
    DAC_Value = (uint32_t)((adc_result_temp) / 4);

    // Envío del valor calculado al DAC:
    DAC_UpdateValue(LPC_DAC, DAC_Value);

    // Control del LED asociado al SysTick:
    if (SYSTICK_Flag == 0)
    {
        Led_Control(ON, LED_CONTROL_1); // Enciende el LED
        SYSTICK_Flag = !SYSTICK_Flag;
    }
    else
    {
        Led_Control(OFF, LED_CONTROL_1); // Apaga el LED
        SYSTICK_Flag = !SYSTICK_Flag;
    }

    // Limpiamos la bandera del SysTick:
    SYSTICK_ClearCounterFlag();
}

/**
 * @brief Handler de la interrupción del temporizador TIMER0.
 *
 * Este handler se ejecuta cuando el temporizador TIMER0 alcanza el valor configurado.
 * Realiza la conversión de los datos del ADC y los envía por UART. También gestiona los LEDs asociados
 * al temporizador y verifica las condiciones de advertencia para la puerta.
 *
 * @note Se limpia la bandera de la interrupción del temporizador después de procesar el evento.
 */
void TIMER0_IRQHandler(void)
{
    uint32_t temp;

    // Procesamiento de los resultados del ADC para los tres canales:
    for (int i = 0; i < 3; i++)
    {
        temp = ((0xFFF0) & ADC_Results[i]) >> 4;
        Data[i] = (temp * 100) / 4096; // Conversión del valor ADC a un porcentaje
    }

    // Ajuste del valor de la puerta:
    Data[3] = DOOR_Flag;

    // Verificación de las mediciones de los sensores:
    Check_Measures();

    // Verificación de las banderas de advertencia y control de la puerta:
    if (WARNING_Close_Flag == WARNING)
    {
        Motor_Activate(CLOSE); // Cerrar la puerta si se detecta advertencia
    }
    if (WARNING_Open_Flag == WARNING)
    {
        Motor_Activate(OPEN); // Abrir la puerta si se detecta advertencia
    }

    // Enviar los datos por UART:
    UART_Send(LPC_UART2, Data, 4, BLOCKING);

    // Control de LED asociado al TIMER0:
    if (TIMER0_Flag == 0)
    {
        Led_Control(ON, LED_CONTROL_3); // Enciende el LED
        TIMER0_Flag = !TIMER0_Flag;
    }
    else
    {
        Led_Control(OFF, LED_CONTROL_3); // Apaga el LED
        TIMER0_Flag = !TIMER0_Flag;
    }

    // Limpiamos la bandera del temporizador TIMER0:
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}

/**
 * @brief Handler de la interrupción del UART2.
 *
 * Este handler se ejecuta cuando se recibe un dato a través del UART2.
 * Controla el estado de un LED en función de la recepción de datos.
 *
 * @note Se limpia la bandera de la interrupción del UART2 después de procesar el evento.
 */
void UART2_IRQHandler(void)
{
    // Verificación de si se ha recibido un dato:
    if (UART_GetIntId(LPC_UART2) & (1 << 1))
    {
        // Control de LED dependiendo de la bandera UART:
        if (UART_Flag == 0)
        {
            Led_Control(ON, LED_CONTROL_4); // Enciende el LED
            UART_Flag = !UART_Flag;
        }
        else
        {
            Led_Control(OFF, LED_CONTROL_4); // Apaga el LED
            UART_Flag = !UART_Flag;
        }
    }
}

/**
 * @brief Handler de la interrupción del PWM1.
 *
 * Este handler se ejecuta cuando el PWM alcanza un valor de coincidencia especificado.
 * Se realiza el conteo de los pulsos y se ajusta el comportamiento del PWM cuando se alcanzan ciertos valores.
 *
 * @note Se limpia la bandera de la interrupción del PWM1 después de procesar el evento.
 */
void PWM1_IRQHandler(void)
{
    if (PWM_GetIntStatus(LPC_PWM1, PWM_INTSTAT_MR0) == SET)
    {
        PWM_count++; // Incrementar el contador de pulsos

        // Si se alcanza la cantidad de pulsos esperada, se configura el PWM para detenerse:
        if (PWM_count == PWM_PULSE_QUANTITY)
        {
            PWM_count = 0; // Reinicia el contador de pulsos
            PWM_MATCHCFG_Type PwmMatch0;
            PwmMatch0.IntOnMatch = ENABLE;         // Habilita la interrupción en la coincidencia
            PwmMatch0.MatchChannel = 0;            // Canal de coincidencia
            PwmMatch0.ResetOnMatch = DISABLE;      // No se reinicia el PWM en la coincidencia
            PwmMatch0.StopOnMatch = ENABLE;        // Detiene el PWM en la coincidencia
            PWM_ConfigMatch(LPC_PWM1, &PwmMatch0); // Configura el PWM con la nueva configuración
        }
    }

    // Limpiamos la bandera de interrupción del PWM:
    PWM_ClearIntPending(LPC_PWM1, PWM_INTSTAT_MR0);
}
