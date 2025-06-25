#include "Mrobot.h"                // Incluye el header de la biblioteca
#include <fcntl.h>                 // Para open()
#include <unistd.h>                // Para close(), write(), usleep()
#include <stdio.h>                 // Para printf(), perror()
#include <errno.h>                 // Para manejo de errores
#include <ctype.h>                 // Para toupper()

#define PASOS_POR_CM 114           // Conversión de centímetros a pasos de motor
#define Q_X_CM 11.3                // Posición X de la tecla Q en cm
#define Q_Y_CM 4.5                 // Posición Y de la tecla Q en cm
#define DISTANCIA_TECLAS 0.9       // Distancia entre teclas en cm

// Estructura para mapear cada letra a su posición física en el teclado
typedef struct {
    char letra;
    float x_cm;
    float y_cm;
} Tecla;

// Arreglo con la posición de cada tecla
Tecla teclado[] = {
    {'q', Q_X_CM + 0 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'w', Q_X_CM + 1 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'e', Q_X_CM + 2 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'r', Q_X_CM + 3 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'t', Q_X_CM + 4 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'y', Q_X_CM + 5 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'u', Q_X_CM + 6 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'i', Q_X_CM + 7 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'o', Q_X_CM + 8 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'p', Q_X_CM + 9 * DISTANCIA_TECLAS, Q_Y_CM + 0 * DISTANCIA_TECLAS},
    {'a', Q_X_CM + 0.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'s', Q_X_CM + 1.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'d', Q_X_CM + 2.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'f', Q_X_CM + 3.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'g', Q_X_CM + 4.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'h', Q_X_CM + 5.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'j', Q_X_CM + 6.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'k', Q_X_CM + 7.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'l', Q_X_CM + 8.5 * DISTANCIA_TECLAS, Q_Y_CM + 1 * DISTANCIA_TECLAS},
    {'z', Q_X_CM + 1 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS},
    {'x', Q_X_CM + 2 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS},
    {'c', Q_X_CM + 3 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS},
    {'v', Q_X_CM + 4 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS},
    {'b', Q_X_CM + 5 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS},
    {'n', Q_X_CM + 6 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS},
    {'m', Q_X_CM + 7 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS},
    {' ', Q_X_CM + 4.5 * DISTANCIA_TECLAS, Q_Y_CM + 3 * DISTANCIA_TECLAS},
    {'\n', Q_X_CM + 9 * DISTANCIA_TECLAS, Q_Y_CM + 2 * DISTANCIA_TECLAS}
};
int num_teclas = sizeof(teclado) / sizeof(teclado[0]); // Número de teclas en el arreglo

static int mrobot_fd = -1; // Descriptor de archivo global para el dispositivo

// Inicializa la conexión con el dispositivo (ej: Arduino)
int mrobot_init(const char *device_path) {
    mrobot_fd = open(device_path, O_RDWR); // Abre el archivo de dispositivo para lectura/escritura
    return (mrobot_fd < 0) ? -1 : 0;       // Devuelve 0 si tuvo éxito, -1 si falló
}

// Envía la posición X/Y al driver (mueve los motores X e Y)
int mrobot_move(int x, int y) {
    if (mrobot_fd < 0) return -1;          // Si no está abierto, error
    int pos[2] = {x, y};                   // Crea un arreglo con X e Y
    return write(mrobot_fd, pos, sizeof(pos)) == sizeof(pos) ? 0 : -1; // Envía los datos al driver
}

// Envía el comando para presionar (activa el servo Z)
int mrobot_press() {
    if (mrobot_fd < 0) return -1;          // Si no está abierto, error
    char cmd = 'P';                        // Comando para presionar
    return write(mrobot_fd, &cmd, 1) == 1 ? 0 : -1; // Envía el comando al driver
}

// Cierra la conexión con el dispositivo
void mrobot_close() {
    if (mrobot_fd >= 0) close(mrobot_fd);  // Si está abierto, ciérralo
    mrobot_fd = -1;                        // Marca como cerrado
}

// Convierte una letra a pasos X/Y usando el mapeo de teclado
int letra_a_pasos(char letra, int *x, int *y) {
    letra = toupper(letra);                // Convierte la letra a mayúscula
    for (int i = 0; i < num_teclas; i++) { // Busca la letra en el arreglo
        if (teclado[i].letra == letra) {
            *x = (int)(teclado[i].x_cm * PASOS_POR_CM); // Calcula pasos X
            *y = (int)(teclado[i].y_cm * PASOS_POR_CM); // Calcula pasos Y
            return 1;                      // Encontrado
        }
    }
    return 0;                              // No encontrado
}

// Programa principal: escribe "hola mundo" letra por letra
// Elimina o comenta este main si usas la biblioteca desde otro programa
/*
int main() {
    if (mrobot_init("/dev/ttyACM0") != 0) {
        perror("No se pudo abrir el dispositivo");
        return 1;
    }

    char *texto = "hola mundo\n";
    for (int i = 0; texto[i]; i++) {
        int x, y;
        if (letra_a_pasos(texto[i], &x, &y)) {
            mrobot_move(x, y);
            usleep(500000);
            mrobot_press();
            usleep(700000);
        } else {
            printf("[ERROR] Letra no reconocida: '%c'\n", texto[i]);
        }
    }

    mrobot_close();
    return 0;
}
*/