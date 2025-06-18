#include "procesamiento.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_WORD 128

// Convierte toda la cadena a minúsculas
static void to_lowercase(char *str) {
    for (; *str; ++str) *str = tolower((unsigned char)*str);
}

// Verifica si un carácter es delimitador de palabra
static int es_delimitador(char c) {
    // Considera como delimitadores: espacios, saltos de línea, tab, y signos de puntuación comunes
    return (isspace((unsigned char)c) || c == '.' || c == ',' || c == ';' || c == ':' || c == '!' || c == '?' || c == '"' || c == '\'' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}');
}

void contar_palabras(const char *data, int len, char ***palabras, int **repeticiones, int *n_palabras) {
    size_t capacidad = 4096;
    char **tokens = malloc(capacidad * sizeof(char*));
    int *counts = calloc(capacidad, sizeof(int));
    size_t n_tokens = 0;

    char *copy = strndup(data, len);
    char *saveptr;
    char *token = strtok_r(copy, " \n\r\t.,;:?!\"", &saveptr);
    while (token) {
        char temp[MAX_WORD];
        strncpy(temp, token, MAX_WORD-1);
        temp[MAX_WORD-1] = 0;
        for (int i = 0; temp[i]; i++) temp[i] = tolower((unsigned char)temp[i]);

        // Solo cuenta palabras de 2 letras o más
        if (strlen(temp) < 2) {
            token = strtok_r(NULL, " \n\r\t.,;:?!\"", &saveptr);
            continue;
        }

        int found = 0;
        for (size_t i = 0; i < n_tokens; i++) {
            if (strcmp(tokens[i], temp) == 0) {
                counts[i]++;
                found = 1;
                break;
            }
        }
        if (!found) {
            if (n_tokens == capacidad) {
                capacidad *= 2;
                tokens = realloc(tokens, capacidad * sizeof(char*));
                counts = realloc(counts, capacidad * sizeof(int));
            }
            tokens[n_tokens] = strdup(temp);
            counts[n_tokens] = 1;
            n_tokens++;
        }
        token = strtok_r(NULL, " \n\r\t.,;:?!\"", &saveptr);
    }
    free(copy);

    *palabras = tokens;
    *repeticiones = counts;
    *n_palabras = (int)n_tokens;
}

void encontrar_palabra_mas_repetida(const char *data, int len, char *palabra, int *reps) {
    char **palabras = NULL;
    int *repeticiones = NULL;
    int n_palabras = 0;

    contar_palabras(data, len, &palabras, &repeticiones, &n_palabras);

    int max_reps = 0;
    palabra[0] = '\0';

    for (int i = 0; i < n_palabras; i++) {
        if (repeticiones[i] > max_reps) {
            max_reps = repeticiones[i];
            strncpy(palabra, palabras[i], MAX_WORD - 1);
            palabra[MAX_WORD - 1] = '\0';
        }
        free(palabras[i]);
    }
    free(palabras);
    free(repeticiones);

    *reps = max_reps;
}
