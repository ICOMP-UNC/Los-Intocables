# Los-Intocables

# Plan del Proyecto

## Descripción del proyecto de alto nivel
Este proyecto tiene como objetivo desarrollar un sistema embebido utilizando la **placa LPC1769 de NXP** para el monitoreo y control de un entorno mediante el uso de varios sensores: **LM35** para temperatura, **MQ-2** como sensor concentracion de gases combustibles y un **sensor LDR** para luz. Los datos recopilados de estos sensores por el **ADC**, se procesarán para controlar un **LED** mediante el **DAC**, un motor paso a paso mediante un **PWM** y salidas **GPIO**, simulando la apertura y cierre de un sistema de ventilacion y purificacion. El sistema integrará módulos esenciales como **GPIO**, **TIMER**, **SYSTICK**, **ADC**, **DAC**, **DMA**, **PWM** y **UART** para lograr un funcionamiento eficiente y en tiempo real.

## Objetivos y criterios de éxito

### Objetivos del proyecto
1. Desarrollar un sistema que permita la lectura de datos de los sensores (temperatura, gases y luz) y su procesamiento.
2. Controlar un LED en función de la información obtenida de los sensores, simulando la apertura y cierre de puertas.
3. Implementar la adquisición de datos a través del **ADC** y sus salidas a un **DAC** y un **PWM**.
4. Optimizar la comunicación entre los módulos utilizando **DMA** y asegurar un manejo adecuado de interrupciones con el **TIMER** y **SYSTICK**.
5. Establecer una comunicación UART para la transmisión de datos o estado del sistema.

### Criterios de éxito
1. El sistema debe ser capaz de adquirir datos en tiempo real de los sensores y procesarlos para controlar el LED, las salidas GPIO y el motor paso a paso.
2. La respuesta del sistema al cambio en las condiciones de los sensores debe ser rápida y efectiva.
3. El uso de **DMA** debe demostrar una carga reducida en el procesador, manteniendo la eficiencia del sistema.
4. El código debe estar bien documentado y seguir las buenas prácticas de programación, asegurando su mantenibilidad.

## Cronograma y asignación de tareas

| Fase                          | Tarea                                                                                      | Responsable        | Duración   | Fecha de inicio | Fecha de fin |
|-------------------------------|--------------------------------------------------------------------------------------------|--------------------|------------|-----------------|---------------|
| **Fase 1: Planificación**     | Definir requisitos del proyecto y recursos necesarios                                      | Equipo completo     | 3 días     | 01/11/2024      | 03/11/2024    |
| **Fase 2: Diseño**           | Diseño de diagrama de flujo                                                                  | Marian1911          | 2 días     | 03/11/2024      | 05/11/2024    |
| **Fase 3: Implementación**    | Implementación de función de configuración del DAC                                           | Marian1911          | 1 día      | 05/11/2024      | 06/11/2024    |
|                               | Implementación de función de configuración de GPIO                                          | Marian1911          | 1 día      | 06/11/2024      | 07/11/2024    |
|                               | Implementación de función de configuración del módulo ADC                                   | LisandroTesta1     | 1 día      | 05/11/2024      | 06/11/2024    |
|                               | Implementación de función de configuración de UART                                          | LisandroTesta1     | 1 día      | 06/11/2024      | 07/11/2024    |
|                               | Implementación de función de configuración de timer                                         | Santi-Madrid        | 1 día      | 05/11/2024      | 06/11/2024    |
|                               | Implementación de función de Systick y función de interrupción del timer                    | Santi-Madrid        | 1 día      | 06/11/2024      | 07/11/2024    |
|                               | Implementación de función de configuración de DMA                                           | CGR-X               | 1 día      | 05/11/2024      | 06/11/2024    |
|                               | Implementación de función de configuración de PWM                                           | CGR-X               | 1 día      | 06/11/2024      | 07/11/2024    |
|                               | Implementación de función de interrupción del Systick                                        | CGR-X               | 1 día      | 06/11/2024      | 07/11/2024    |
| **Fase 4: Integración y pruebas** | Pruebas unitarias y de integración de todos los módulos                                   | Equipo completo     | 6 días     | 09/11/2024      | 15/11/2024    |
| **Fase 5: Entrega**          | Documentación final del proyecto y presentación                                              | Equipo completo     | 1 día      | 16/11/2024      | 15/11/2024    |

### Asignación de roles
- **Integrante 2 (Marian1911):** Diseño de diagrama de flujo, implementación de función de configuración del DAC y función de configuración de GPIO.
- **Integrante 3 (LisandroTesta1):** Implementación de función de configuración del módulo ADC y función de configuración de UART.
- **Integrante 4 (Santi-Madrid):** Implementación de función de configuración de timer, Systick y función de interrupción del timer.
- **Integrante 1 (CGR-X):** Implementación de función de configuración de DMA, función de interrupción del Systick y funcion de configuracion de PWM.

# Resultados
La exposicion del trabajo donde se muestra el correcto funcionamiento del sistema completo y una breve descripcion y explicacion, se encuentra en el link a continuacion:
[Video de culminacion del proyecto](https://www.youtube.com/watch?v=5t60uCoHwm4)
Tambien se adjuntan fotos del circuito armado:
![Circuito aramdo](/images/Circuito_Armado.JPEG)
