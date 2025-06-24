#include "Servidor.h"
#include "procesamiento.h"
#include "../Biblioteca/Mrobot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h> // Para sockets

#define N_NODOS 3
#define MAX_PALABRAS 10000
#define PUERTO_NODO1 5001
#define PUERTO_NODO2 5002
#define PUERTO_NODO3 5003

typedef struct {
    char *data;
    int len;
    char **palabras;
    int *repeticiones;
    int n_palabras;
} NodoArgs;

typedef struct {
    char palabra[MAX_WORD];
    int repeticiones;
} PalabraGlobal;

// funciones para dividir el conteo por lìneas
int contar_lineas(const char *data, int len) {
    int lineas = 0;
    for (int i = 0; i < len; i++) {
        if (data[i] == '\n') lineas++;
    }
    // Considera la última línea si no termina en '\n'
    if (len > 0 && data[len-1] != '\n') lineas++;
    return lineas;
}

void obtener_indices_lineas(const char *data, int len, int *indices, int n_lineas) {
    int l = 0;
    indices[l++] = 0;
    for (int i = 0; i < len; i++) {
        if (data[i] == '\n' && l < n_lineas) {
            indices[l++] = i + 1;
        }
    }
    indices[n_lineas] = len; // marca el final
}

// Nueva función para enviar parte a un nodo y recibir resultado
void enviar_a_nodo(const char *data, int len, int puerto, char ***palabras, int **repeticiones, int *n_palabras) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in nodo_addr;
    nodo_addr.sin_family = AF_INET;
    nodo_addr.sin_port = htons(puerto);
    nodo_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(sock, (struct sockaddr *)&nodo_addr, sizeof(nodo_addr));

    // Enviar tamaño y datos
    write(sock, &len, sizeof(int));
    write(sock, data, len);

    // Recibir n_palabras
    read(sock, n_palabras, sizeof(int));
    *palabras = malloc(*n_palabras * sizeof(char*));
    *repeticiones = malloc(*n_palabras * sizeof(int));
    for (int i = 0; i < *n_palabras; i++) {
        int plen;
        read(sock, &plen, sizeof(int));
        (*palabras)[i] = malloc(plen+1);
        read(sock, (*palabras)[i], plen);
        (*palabras)[i][plen] = 0;
        read(sock, &((*repeticiones)[i]), sizeof(int));
    }
    close(sock);
}

int main() {
    int server_fd = iniciarServidor();
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);

    printf("Esperando conexión del cliente...\n");
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

    char *archivo = NULL;
    int total = 0;
    recibirArchivo(client_fd, &archivo, &total);
    close(client_fd);

    // Guardar el archivo descifrado en disco
    FILE *fp_descifrado = fopen("archivo_descifrado.txt", "wb");
    if (fp_descifrado) {
        fwrite(archivo, 1, total, fp_descifrado);
        fclose(fp_descifrado);
        printf("Archivo descifrado guardado como 'archivo_descifrado.txt'\n");
    } else {
        perror("No se pudo guardar el archivo descifrado");
    }

    //  dividir por líneas 
    int n_lineas = contar_lineas(archivo, total);
    int *indices = malloc((n_lineas + 1) * sizeof(int));
    obtener_indices_lineas(archivo, total, indices, n_lineas);

    NodoArgs nodos[N_NODOS];
    int lineas_por_hilo = n_lineas / N_NODOS;
    int resto = n_lineas % N_NODOS;
    int inicio = 0;
    for (int i = 0; i < N_NODOS; i++) {
        int l_ini = inicio;
        int l_fin = inicio + lineas_por_hilo + (i < resto ? 1 : 0);
        nodos[i].data = archivo + indices[l_ini];
        nodos[i].len = indices[l_fin] - indices[l_ini];
        nodos[i].palabras = NULL;
        nodos[i].repeticiones = NULL;
        nodos[i].n_palabras = 0;
        inicio = l_fin;
    }
    free(indices);

    for (int i = 0; i < N_NODOS; i++) {
        enviar_a_nodo(nodos[i].data, nodos[i].len, PUERTO_NODO1 + i,
                      &nodos[i].palabras, &nodos[i].repeticiones, &nodos[i].n_palabras);
    }

    // Junta todos los resultados
    PalabraGlobal *globales = malloc(MAX_PALABRAS * sizeof(PalabraGlobal));
    int n_globales = 0;

    for (int i = 0; i < N_NODOS; i++) {
        for (int j = 0; j < nodos[i].n_palabras; j++) {
            char temp_palabra[MAX_WORD];
            strncpy(temp_palabra, nodos[i].palabras[j], MAX_WORD-1);
            temp_palabra[MAX_WORD-1] = 0;
            // Asegura que esté en minúsculas
            for (char *p = temp_palabra; *p; ++p) *p = tolower(*p);

            int found = 0;
            for (int k = 0; k < n_globales; k++) {
                if (strcmp(globales[k].palabra, temp_palabra) == 0) {
                    globales[k].repeticiones += nodos[i].repeticiones[j];
                    found = 1;
                    break;
                }
            }
            if (!found && n_globales < MAX_PALABRAS) {
                strncpy(globales[n_globales].palabra, temp_palabra, MAX_WORD-1);
                globales[n_globales].palabra[MAX_WORD-1] = 0;
                globales[n_globales].repeticiones = nodos[i].repeticiones[j];
                n_globales++;
            }
        }
    }

    // Busca la palabra más repetida globalmente
    char palabra_final[MAX_WORD] = "";
    int repeticiones_final = 0;
    for (int i = 0; i < n_globales; i++) {
        if (globales[i].repeticiones > repeticiones_final) {
            strcpy(palabra_final, globales[i].palabra);
            repeticiones_final = globales[i].repeticiones;
        }
    }

    printf("Palabra más repetida: '%s' (%d veces)\n", palabra_final, repeticiones_final);

    // Usar Mrobot para escribir la palabra 
    if (mrobot_init("/dev/Mrobot") == 0) {
        for (int i = 0; palabra_final[i]; i++) {
            int x = (palabra_final[i] - 'a') % 10; // Ejemplo de mapeo a X
            int y = (palabra_final[i] - 'a') / 10; // Ejemplo de mapeo a Y
            mrobot_move(x, y);
            mrobot_press();
        }
        mrobot_close();
    } else {
        printf("No se pudo inicializar Mrobot\n");
    }

    // Libera memoria
    for (int i = 0; i < N_NODOS; i++) {
        for (int j = 0; j < nodos[i].n_palabras; j++) free(nodos[i].palabras[j]);
        free(nodos[i].palabras);
        free(nodos[i].repeticiones);
    }
    free(globales);
    close(server_fd);
    free(archivo);
    return 0;
}