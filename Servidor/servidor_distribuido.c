
#include "Servidor.h"                  
#include "procesamiento.h"             
#include "../Biblioteca/Mrobot.h"      
#include <stdio.h>                     
#include <stdlib.h>                    
#include <string.h>                    
#include <netinet/in.h>                
#include <unistd.h>                    
#include <ctype.h>                     
#include <arpa/inet.h>                 

// Definiciones de constantes del sistema
#define N_NODOS 3                      // Número de nodos de procesamiento distribuido
#define MAX_PALABRAS 10000             // Máximo número de palabras únicas a procesar
#define PUERTO_NODO1 5001              
#define PUERTO_NODO2 5002              
#define PUERTO_NODO3 5003              
#define MAX_TAM_ARCHIVO 1048576        // Tamaño máximo del archivo: 1 MB

// Pasar argumentos a cada nodo d
typedef struct {
    char *data;                        // Puntero al fragmento de texto asignado al nodo
    int len;                           // Longitud del fragmento de texto
    char **palabras;                   // Array de palabras encontradas por el nodo
    int *repeticiones;                 // Array con el conteo de cada palabra
    int n_palabras;                    // Número total de palabras únicas encontradas
} NodoArgs;

// Almacenar palabras con su conteo global
typedef struct {
    char palabra[MAX_WORD];            // La palabra encontrada
    int repeticiones;                  
} PalabraGlobal;


// funciones para dividir el conteo por lìneas
int contar_lineas(const char *data, int len) {
    int lineas = 0;
    // Recorre todo el texto contando los saltos de línea
    for (int i = 0; i < len; i++) {
        if (data[i] == '\n') lineas++;
    }
    // Considera la última línea si no termina en '\n'
    if (len > 0 && data[len-1] != '\n') lineas++;
    return lineas;
}

// Para validar donde inicia cada línea en el texto
void obtener_indices_lineas(const char *data, int len, int *indices, int n_lineas) {
    int l = 0;
    indices[l++] = 0;                  // La primera línea inicia en la posición 0
    // Busca cada salto de línea y guarda la posición siguiente
    for (int i = 0; i < len; i++) {
        if (data[i] == '\n' && l < n_lineas) {
            indices[l++] = i + 1;      // Guarda el inicio de la siguiente línea
        }
    }
    indices[n_lineas] = len;           // Marca el final del texto
}


// Establece conexión TCP con el nodo, envía datos y recibe palabras procesadas
void enviar_a_nodo(const char *data, int len, int puerto, char ***palabras, int **repeticiones, int *n_palabras) {
    // Crear socket TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in nodo_addr;
    nodo_addr.sin_family = AF_INET;
    nodo_addr.sin_port = htons(puerto);
    nodo_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(sock, (struct sockaddr *)&nodo_addr, sizeof(nodo_addr));

    // Enviar tamaño del fragmento y los datos al nodo
    write(sock, &len, sizeof(int));
    write(sock, data, len);

    // Recibir número de palabras procesadas por el nodo
    read(sock, n_palabras, sizeof(int));
    *palabras = malloc(*n_palabras * sizeof(char*));
    *repeticiones = malloc(*n_palabras * sizeof(int));
    
    // Recibir cada palabra y su conteo
    for (int i = 0; i < *n_palabras; i++) {
        int plen;
        read(sock, &plen, sizeof(int));             // Longitud de la palabra
        (*palabras)[i] = malloc(plen+1);
        read(sock, (*palabras)[i], plen);           // La palabra
        (*palabras)[i][plen] = 0;                   // Terminador nulo
        read(sock, &((*repeticiones)[i]), sizeof(int)); // Su conteo
    }
    close(sock);                                    // Cerrar conexión
}

int main() {

    // Inicializar servidor para recibir conexiones del cliente
    int server_fd = iniciarServidor();
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);

    printf("Esperando conexión del cliente...\n");
 
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

    // Recibir el archivo cifrado del cliente
    char *archivo = NULL;
    int total = 0;
    recibirArchivo(client_fd, &archivo, &total);
    close(client_fd);                           // Cerrar conexión con el cliente

    // Validar tamaño máximo del archivo
    if (total > MAX_TAM_ARCHIVO) {
        printf("El archivo es demasiado grande (máximo permitido: %d bytes)\n", MAX_TAM_ARCHIVO);
        free(archivo);
        close(server_fd);
        return 1;
    }

    // Guardar el archivo descifrado en disco
    FILE *fp_descifrado = fopen("archivo_descifrado.txt", "wb");
    if (fp_descifrado) {
        fwrite(archivo, 1, total, fp_descifrado);
        fclose(fp_descifrado);
        printf("Archivo descifrado guardado como 'archivo_descifrado.txt'\n");
    } else {
        perror("No se pudo guardar el archivo descifrado");
    }


    
    // Dividir el texto por líneas completas para distribuir el trabajo
    int n_lineas = contar_lineas(archivo, total);
    int *indices = malloc((n_lineas + 1) * sizeof(int));
    obtener_indices_lineas(archivo, total, indices, n_lineas);

    // Preparar argumentos para cada nodo de procesamiento
    NodoArgs nodos[N_NODOS];
    int lineas_por_hilo = n_lineas / N_NODOS;
    int resto = n_lineas % N_NODOS;
    int inicio = 0;
    
    // Dividir el trabajo entre los nodos
    for (int i = 0; i < N_NODOS; i++) {
        int l_ini = inicio;
        int l_fin = inicio + lineas_por_hilo + (i < resto ? 1 : 0);
        nodos[i].data = archivo + indices[l_ini];        // Puntero al fragmento
        nodos[i].len = indices[l_fin] - indices[l_ini];  // Tamaño del fragmento
        nodos[i].palabras = NULL;                        // Inicializar punteros
        nodos[i].repeticiones = NULL;
        nodos[i].n_palabras = 0;
        inicio = l_fin;
    }
    free(indices);                                       // Liberar memoria de índices

    
    // Enviar fragmentos a cada nodo y recibir resultados procesados
    for (int i = 0; i < N_NODOS; i++) {
        enviar_a_nodo(nodos[i].data, nodos[i].len, PUERTO_NODO1 + i,
                      &nodos[i].palabras, &nodos[i].repeticiones, &nodos[i].n_palabras);
    }


    
    // Crear array para almacenar todas las palabras con su conteo global
    PalabraGlobal *globales = malloc(MAX_PALABRAS * sizeof(PalabraGlobal));
    int n_globales = 0;

    // Recorrer resultados de todos los nodos para consolidar
    for (int i = 0; i < N_NODOS; i++) {
        for (int j = 0; j < nodos[i].n_palabras; j++) {
            // Copiar y normalizar la palabra y convertir a minúsculas
            char temp_palabra[MAX_WORD];
            strncpy(temp_palabra, nodos[i].palabras[j], MAX_WORD-1);
            temp_palabra[MAX_WORD-1] = 0;
            // Asegura que esté en minúsculas
            for (char *p = temp_palabra; *p; ++p) *p = tolower(*p);

            // Buscar si la palabra ya existe en el conteo global
            int found = 0;
            for (int k = 0; k < n_globales; k++) {
                if (strcmp(globales[k].palabra, temp_palabra) == 0) {
                    // Si existe, sumar las repeticiones
                    globales[k].repeticiones += nodos[i].repeticiones[j];
                    found = 1;
                    break;
                }
            }
            // Si no existe, agregarla al array global
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

    
    // Usar Mrobot para escribir la palabra más repetida
    if (mrobot_init("/dev/ttyACM0") == 0) { // Puerto Arduino
        for (int i = 0; palabra_final[i]; i++) {
            int x, y;
            if (letra_a_pasos(palabra_final[i], &x, &y)) {
                mrobot_move(x, y);
                usleep(500000);     // Espera a que llegue
                mrobot_press();
                usleep(700000);     // Espera a que termine de presionar
            } else {
                printf("[ERROR] Letra no reconocida: '%c'\n", palabra_final[i]);
            }
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