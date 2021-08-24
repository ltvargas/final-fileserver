#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <fcntl.h>
#include "libs/archivos/blowfish.h"
#include "libs/archivos/uECC.h"
#include "libs/archivos/types.h"
#include "libs/archivos/sha256.h"

#define MAXLINE 1024

/* Tamaño clave pública Diffie-Hellman: 64 bytes */
#define DH_PUBLIC_KEY_SIZE 64

/* Tamaño clave privada Diffie-Hellman: 32 bytes */          
#define DH_PRIVATE_KEY_SIZE 32

/* Tamaño secreto compartido Diffie-Hellman: 32 bytes */ 
#define DH_SECRET_SIZE 32

/* Tamaño clave Blowfish: 32 bytes (256 bits)*/ 
#define BLOWFISH_KEY_SIZE 32

///////////////////////////////////////
/* Funciones de gestión de conexión */

/**
 * Open and return a listening socket on port. This function is reentrant and protocol-independent.
 *
 * @param port Character array with TCP port, decimal.
 *
 * @return A listening socker. On error, returns -1 and sets errno.
 */
int open_listenfd(char *port);


/**
 * Open connection to server at <hostname, port> and 
 * return a socket descriptor ready for reading and writing. This
 * function is reentrant and protocol-independent.
 *
 * @param hostname Character array with IP address or hostname.
 * @param port Character array with TCP port, decimal.
 *
 * @return Socket file descriptor. On error, returns -1 and sets errno.
 */
int open_clientfd(char *hostname, char *port);

/**
 * Closes the socket, prints error on STDERR and exits.
 *
 * @param connfd Socket file descriptor.
 */
void connection_error(int connfd);

char *textcpy(char **nombreFile);
char *textcpyarray(char array[]);
char **buf_tokens(char *linea, char *delim);
int cantidad(char ** comand);
#endif /* COMMON_H */
