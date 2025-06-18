#ifndef CLIENTE_H
#define CLIENTE_H

#define PORT 12345
#define SERVER_IP "127.0.0.1"
#define KEY 0xAA

int conectarServidor();
void enviarArchivoCifrado(int socket, const char *rutaArchivo);
void xor_encrypt(char *data, int len);

#endif
