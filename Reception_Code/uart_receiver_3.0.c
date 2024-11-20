#include <windows.h>
#include <stdio.h>

#define PACKET_SIZE 4  // Define el tamaño del paquete esperado

int main() {
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};
    DWORD bytesRead;
    BYTE buffer[PACKET_SIZE];
    DWORD totalBytesRead = 0;
    BYTE receivedData[PACKET_SIZE];
    
    // Abrir el puerto COM7
    hSerial = CreateFile("\\\\.\\COM7", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error al abrir el puerto COM7.\n");
        return 1;
    }
    printf("Puerto COM7 abierto correctamente.\n");

    // Obtener el estado actual del puerto
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error al obtener el estado del puerto.\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Configurar los parámetros del puerto
    dcbSerialParams.BaudRate = CBR_9600;        // Velocidad de baudios
    dcbSerialParams.ByteSize = 8;               // Tamaño de byte
    dcbSerialParams.StopBits = ONESTOPBIT;      // 1 bit de parada
    dcbSerialParams.Parity = NOPARITY;          // Sin paridad

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error al configurar el puerto COM.\n");
        CloseHandle(hSerial);
        return 1;
    }
    printf("Configuración del puerto COM6 realizada correctamente.\n");

    // Configurar los tiempos de espera (timeouts)
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        fprintf(stderr, "Error al configurar los tiempos de espera.\n");
        CloseHandle(hSerial);
        return 1;
    }
    printf("Timeouts configurados correctamente.\n");

    printf("Esperando datos en UART...\n");

    while (1) {
        // Leer los datos disponibles en el puerto
        if (ReadFile(hSerial, buffer, PACKET_SIZE - totalBytesRead, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                for (DWORD i = 0; i < bytesRead; i++) {
                    receivedData[totalBytesRead++] = buffer[i];
                    // Verificar si se ha recibido un paquete completo
                    if (totalBytesRead == PACKET_SIZE) {
                        // Imprimir el formato deseado en decimal
                        printf("\nSTATUS\n");
                        printf("Temperatura: %d [C]\n", receivedData[0]);
                        printf("Iluminacion: %d [%%] \n", receivedData[1]);
                        printf("Concentracion: %d [%%]\n", receivedData[2]);
                        switch (receivedData[3]){
                        case 1:
                        printf("Estado de la ventilacion abierto \n");
                        break;
                        case 0: 
                        printf("Estado de la ventilacion cerrado \n");
                        break;
                        default:
                        break;
                        }
                        
                        // Reiniciar el contador para el siguiente paquete
                        totalBytesRead = 0;
                    }
                }
            }
        } else {
            fprintf(stderr, "Error al leer del puerto COM.\n");
            break;
        }

        Sleep(50);  // Reducir la carga de la CPU
    }

    CloseHandle(hSerial);
    printf("Puerto COM cerrado.\n");
    return 0;
}