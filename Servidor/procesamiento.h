#ifndef PROCESAMIENTO_H
#define PROCESAMIENTO_H

#define MAX_WORD 128

void contar_palabras(const char *data, int len, char ***palabras, int **repeticiones, int *n_palabras);
void encontrar_palabra_mas_repetida(const char *data, int len, char *palabra, int *reps);

#endif
