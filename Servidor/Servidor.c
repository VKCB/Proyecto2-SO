
#include "Servidor.h"              
#include <stdio.h>                 
#include <stdlib.h>                
#include <string.h>                
#include <unistd.h>                
#include <arpa/inet.h>             


int iniciarServidor() {
    int server_fd;
    struct sockaddr_in server_addr;

    // Crear socket 
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("No se pudo crear socket");
        exit(1);
    }
    
    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;           
    server_addr.sin_port = htons(PORT);         
    server_addr.sin_addr.s_addr = INADDR_ANY;   

    // Asocia el socket con la dirección y puerto especificados
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        exit(1);
    }

    if (listen(server_fd, 1) < 0) {
        perror("Error en listen");
        exit(1);
    }
    
    return server_fd;                           
}

// Función para cifrar/descifrar datos usando XOR
// Aplica la operación XOR con una clave fija a cada byte de los datos
void xor_encrypt(char *data, int len) {
    for (int i = 0; i < len; i++)
        data[i] ^= KEY;                         // XOR cada byte con la clave 
}

// Función para recibir archivo cifrado del cliente y descifrarlo
// Parámetros:
//   socketDestino: descriptor del socket conectado al cliente
//   buffer: puntero donde se almacenará el contenido descifrado
//   len: puntero donde se almacenará el tamaño total del archivo
void recibirArchivo(int socketDestino, char **buffer, int *len) {
    // Abrir archivos para guardar tanto la versión cifrada como la descifrada
    FILE *fp = fopen(OUTPUT_FILE, "wb");            // Archivo descifrado
    FILE *fp_cifrado = fopen("archivo_cifrado.enc", "wb"); // Archivo cifrado original
    
    char tmp[1024];                                 // Buffer temporal para recibir datos
    int total = 0, n, cap = 1024 * 1024;           // Contadores y capacidad inicial (1MB)
    char *buf = (char *)malloc(cap);                // Buffer dinámico para almacenar todo el contenido
    
    // Verificar que se pudieron abrir los archivos y asignar memoria
    if (!fp || !fp_cifrado || !buf) {
        perror("No se puede abrir archivo o asignar memoria");
        exit(1);
    }
    
    // Recibir datos del cliente en bloques de hasta 1024 bytes
    while ((n = recv(socketDestino, tmp, sizeof(tmp), 0)) > 0) {
        // Guardar datos cifrados tal como llegan del cliente
        if (fwrite(tmp, 1, n, fp_cifrado) != n) {
            perror("Error escribiendo archivo cifrado");
            exit(1);
        }
        
        // Descifrar los datos usando XOR
        xor_encrypt(tmp, n);
        
        // Guardar datos descifrados al archivo
        fwrite(tmp, 1, n, fp);
        
        // Verificar si necesitamos expandir el buffer dinámico
        if (total + n > cap) {
            cap *= 2;                               // Duplicar la capacidad
            buf = (char *)realloc(buf, cap);        // Redimensionar el buffer
            if (!buf) {
                perror("Fallo de realloc");
                exit(1);
            }
        }
        
        // Copiar datos descifrados al buffer en memoria
        memcpy(buf + total, tmp, n);
        total += n;                                 // Actualizar contador total
    }
    
    // Cerrar archivos
    fclose(fp);
    fclose(fp_cifrado);
    
    // Retornar el buffer con el contenido completo y su tamaño
    *buffer = buf;
    *len = total;
}