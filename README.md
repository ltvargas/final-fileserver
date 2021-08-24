# PROYECTO DE PROGRAMACION...
Proyecto de Programacion de Sistema de la ESPOL

## Instrucciones
El proyecto consiste en la implementacion de un servidor que envia y recibe archivos mediante la encriptacion de datos mediante uso de contrasena compartida utilizando el algoritmo de Diffie-Hellman, los comandos son los siguientes:
GET <nombreArchivo> garantiza la descarga del archivo al servidor
PUT <nombreArchivo> <longitudArchivo> sube el archivo al servidor enviando conjuntamente el TAMANO DEL ARCHIVO.- Para nuestro ejemplo tambien se le tiene que pasar el tamano del archivo, esto es un requisito importante sin embargo se esta conciente que podria estar a sueltas de mejoras.
READY confirma que esta listo para la descarga del archivo
CHAO ejecuta la descomunicacion entre pares
LIST enlista los archivos existente en el proyecto
Este proyecto se baso bajo la modalidad de multi-hilos y mantiene una libreria de forma estatica.

Toda comunicación entre cliente y servidor debe ser enviada en modo ASCII con salto de línea al final, excepto la transmisión del contenido del archivo, el cual debe ser enviado mediante blowfish pero guardado en el disco duro en ASCII.

El programa servidor *server* tiene el siguiente comportamiento:
```
$ ./server -h
Servidor simple de descarga de archivos.
uso:
 ./server <puerto>
 ./server -h
 ./server -d <puerto>
Opciones:
 -h			Ayuda, muestra este mensaje
 -d 			Activa el modo Daemon
```

El programa cliente *client* tiene el siguiente comportamiento:
```
$ ./client -h
Cliente simple de descarga de archivos.
uso:
 ./client <hostname> <puerto>
 ./client -h
Opciones:
 -h			Ayuda, muestra este mensaje
```



## Compilación

```

Para compilar cliente y servidor:
```
NAME=crypto
DEPS = common.h
CFLAGS = -Wall -c
SLIB = lib$(NAME).a
LDIR= libs
LIBS=libs/archivos/
SSTC= -Wl, -Bstatic

.PHONY: all debug sanitize clean

all: $(LDIR)/$(SLIB) server client

$(LDIR)/$(SLIB): $(LIBS)blowfish.o $(LIBS)uECC.o $(LIBS)sha256.o $(LIBS)common.o
	ar rcs $(LDIR)/$(SLIB) $(LIBS)blowfish.o $(LIBS)uECC.o $(LIBS)sha256.o $(LIBS)common.o

$(LIBS)blowfish.o: blowfish.c
	gcc $(CFLAGS) -c $(LIBS)blowfish.c -o $(LIBS)blowfish.o

$(LIBS)uECC.o: uECC.c
	gcc $(CFLAGS) -c $(LIBS)uECC.c -o $(LIBS)uECC.o

$(LIBS)sha256.o: sha256.c
	gcc $(CFLAGS) -c $(LIBS)sha256.c -o $(LIBS)sha256.o 
 
server: server.c common.o  $(DEPS) 
	gcc  -g -o server server.c  common.o $(SSTC) $(LDIR)/$(SLIB)  -lpthread

client: client.c common.o $(DEPS)
	gcc  -g -o client client.c  common.o $(SSTC) $(LDIR)/$(SLIB)  #-lpthread

common.o: common.c $(DEPS)
	gcc $(CFLAGS) -c common.c

```

# Compila usando la opción -g para facilitar la depuración con gdb.
debug: DFLAGS = -g
debug: clean all

# Compila habilitando la herramienta AddressSanitizer para
# facilitar la depuración en tiempo de ejecución.
sanitize: DFLAGS = -fsanitize=address,undefined
sanitize: clean all

clean:
	rm -rf server client only_server *.o lib/archivos/*.o 
```
Para compilar cliente y servidor facilitando la depuración con gdb:
```
# Compila usando la opción -g para facilitar la depuración con gdb.
.PHONY: all debug sanitize clean
debug: DFLAGS = -g
debug: clean all

$ make debug
```
Para compilar cliente y servidor habilitando la herramienta AddressSanitizer, facilita la depuración en tiempo de ejecución:
```
sanitize: DFLAGS = -fsanitize=address,undefined
sanitize: clean all
$ make sanitize
```
## Integrantes

```
Morales Mendoza Jose
  Actividades realizadas:
	-Agregar Permisos de escritura y lectura al archivo que recibe el cliente.
	-Encriptacion de archivos
	-Claves mediante modelo Diffie-Helman
	-Metodo GET
	-Edicion de Makefile
	-Modo Daemon
```

```
Vargas Aguirre Lilibeth
  Actividades realizadas:
	-Edición del README.
	-Manejo de directorios de archivos clientes y servidor.
	-Metodo PUT, CHAO, READY
	-Automatas
	-Validaciones e implementacion de Hilos
	-Edicion de Makefile
```
