#ifndef SERVIDOR_H
#define SERVIDOR_H

#define PORT 12345
#define OUTPUT_FILE "archivo_cifrado.enc"
#define KEY 0xAA

int iniciarServidor();
void recibirArchivo(int socketDestino, char **buffer, int *len);
void xor_encrypt(char *data, int len);

#endif