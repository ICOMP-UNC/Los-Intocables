# Guia de instalacion
En esta guia se van a detallar los pasos para realizar el armado a nivel de harware y su posterior instalacion del firmaware, de manera de asegurar que los interesados puedan recrear este proyecto.

## Materiales:
1.  Placa LPC1769 de NXP.
2.  Sensor de temperatura LM35.
3.  Sensor de gases combustibles MQ-2.
4.  Sensor de luz (LDR).
5.  Leds, cant: 5.
6.  Transistores , cant: 5.
7.  Boton pulsador .
8.  Motor paso a paso, NEMA17.
9.  Driver motor paso a paso, A4988.
10. Fuente 5[V], 3,3[V] y 12[V].
11. Resistencias.
12. Cables.

## Armado de hardware:
Para esta parte lo que se va a hacer es armar el circuito, preferiblemente en una protoboard primero para corroborar su correcttor funcionamiento.
A continuacion se detalla el circuito esquematico, con el cual se va a guiar par el armado del hardware:

imagen esquematico.

## Instalacion de firmware:
Suponiendo que ya haya realizado en correcto armado de hardware se pasara a realizar la instalacion del programa.

### Paso 1:
lo primero es descargar el **MCUXpressoIDE** y el repositorio para poder acceder a los archivos de **Src** y **Lib**, en donde se encuentra el archivo main y las librerias correspondientes.

### Paso 2:
Una vez hechas las correspondientes descargas se creara un nuevo proyecto en el **MCUXpresso** donde luego se introduciran los archivos de la carpeta **Src** y **Lib**, de esta manera nuestro proyecto va a contar con el codigo fuente (main) y las correspondientes librerias para ejecutar el codigo.

### Paso 3:
Se procedera a enchufar la placa a la computadora mediante un cable USB-MircroUSB, una vez hecho esto nos vamos a dirigir al MCUXpresso y vamos a buscar abajo a la izquierda donde dice "QuickStart Panel", ahi se encuentra el boton de Debug linkServer.

### Paso 4: 
Al apretar el boton de Debug LinkServer nos va saltar una pantalla que muestra los dispositivos conectados, vamos a seleccionar la placa LPC1769 y darle a Aceptar, luego automaticamente se va a cargar el programa en la placa.

### Paso 5:
Una vez terminado el proceso de carga nos va a aparecer para debugear el programa, si no queresmos debugear el mismo se va a dar play al programa, frenar al debug y darle al boton de reset de la placa. Una vez hecho esto el programa ya va a estar corriendo en la placa sin necesidad de seguir conectada a la computadora.
