#include "Servidor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int iniciarServidor() {
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("No se pudo crear socket");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

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

void xor_encrypt(char *data, int len) {
    for (int i = 0; i < len; i++)
        data[i] ^= KEY;
}

void recibirArchivo(int socketDestino, char **buffer, int *len) {
    FILE *fp = fopen(OUTPUT_FILE, "wb");
    FILE *fp_cifrado = fopen("archivo_cifrado.enc", "wb");
    char tmp[1024];
    int total = 0, n, cap = 1024 * 1024;
    char *buf = (char *)malloc(cap);
    if (!fp || !fp_cifrado || !buf) {
        perror("No se puede abrir archivo o asignar memoria");
        exit(1);
    }
    while ((n = recv(socketDestino, tmp, sizeof(tmp), 0)) > 0) {
        if (fwrite(tmp, 1, n, fp_cifrado) != n) {
            perror("Error escribiendo archivo cifrado");
            exit(1);
        }
        xor_encrypt(tmp, n);
        fwrite(tmp, 1, n, fp);
        if (total + n > cap) {
            cap *= 2;
            buf = (char *)realloc(buf, cap);
            if (!buf) {
                perror("Fallo de realloc");
                exit(1);
            }
        }
        memcpy(buf + total, tmp, n);
        total += n;
    }
    fclose(fp);
    fclose(fp_cifrado);
    *buffer = buf;
    *len = total;
}