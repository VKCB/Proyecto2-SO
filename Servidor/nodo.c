// nodo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "procesamiento.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        return 1;
    }
    int puerto = atoi(argv[1]);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(puerto);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);

    printf("Nodo en puerto %d esperando conexión...\n", puerto);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        int len;
        read(client_fd, &len, sizeof(int));
        char *data = malloc(len);
        read(client_fd, data, len);

        printf("Nodo en puerto %d recibió %d bytes\n", puerto, len);

        char **palabras;
        int *repeticiones;
        int n_palabras;
        contar_palabras(data, len, &palabras, &repeticiones, &n_palabras);

        printf("Nodo en puerto %d procesó %d palabras distintas\n", puerto, n_palabras);

        write(client_fd, &n_palabras, sizeof(int));
        for (int i = 0; i < n_palabras; i++) {
            int plen = strlen(palabras[i]);
            write(client_fd, &plen, sizeof(int));
            write(client_fd, palabras[i], plen);
            write(client_fd, &repeticiones[i], sizeof(int)); // <-- Agregado
        }
        free(palabras);
        free(repeticiones);
        free(data);
        close(client_fd);
    }
    close(server_fd);
    return 0;
}