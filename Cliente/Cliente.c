#include "Cliente.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int conectarServidor() {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        exit(1);
    }

    return sock;
}

void xor_encrypt(char *data, int len) {
    for (int i = 0; i < len; i++)
        data[i] ^= KEY;
}

void enviarArchivoCifrado(int socket, const char *rutaArchivo) {
    FILE *fp = fopen(rutaArchivo, "rb");
    char buffer[1024];
    int n;

    if (!fp) {
        perror("No se puede abrir archivo");
        exit(1);
    }

    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        xor_encrypt(buffer, n);
        send(socket, buffer, n, 0);
    }

    fclose(fp);
    close(socket);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s archivo_a_enviar.txt\n", argv[0]);
        return 1;
    }

    int sock = conectarServidor();
    enviarArchivoCifrado(sock, argv[1]);
    printf("Archivo enviado correctamente.\n");
    return 0;

}
