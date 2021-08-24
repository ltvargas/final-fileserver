NAME=crypto
DEPS = common.h
CFLAGS = -Wall -c
SLIB = lib$(NAME).a
LDIR= lib
FSERVER = /fileServer
LIBS=libs/archivos/
SSTC=-Wl,-Bstatic
LIBD=-Wl,-Bdynamic -lpthread

.PHONY: all debug sanitize clean

all: $(LDIR)$(SLIB) server

$(LDIR)$(SLIB): $(LIBS)blowfish.o $(LIBS)uECC.o $(LIBS)sha256.o 
	ar rcs $(LDIR)$(SLIB) $(LIBS)blowfish.o $(LIBS)uECC.o $(LIBS)sha256.o 

$(LIBS)blowfish.o: $(LIBS)blowfish.c
	gcc $(CFLAGS) -c $(LIBS)blowfish.c -o $(LIBS)blowfish.o

$(LIBS)uECC.o: $(LIBS)uECC.c
	gcc $(CFLAGS) -c $(LIBS)uECC.c -o $(LIBS)uECC.o

$(LIBS)sha256.o: $(LIBS)sha256.c
	gcc $(CFLAGS) -c $(LIBS)sha256.c -o $(LIBS)sha256.o 
 
server: server.c common.o  $(LDIR)$(SLIB) $(DEPS) 
	gcc  -g -o s_server server.c  common.o $(SSTC) $(LDIR)$(SLIB) $(LIBD) 

common.o: common.c $(DEPS)
	gcc $(CFLAGS) -c common.c

$(shell mkdir -p $(FSERVER))

# Compila usando la opción -g para facilitar la depuración con gdb.
debug: DFLAGS = -g
debug: clean all

# Compila habilitando la herramienta AddressSanitizer para
# facilitar la depuración en tiempo de ejecución.
sanitize: DFLAGS = -fsanitize=address,undefined
sanitize: clean all

clean:
	rm -rf  $(LIBS)*.o common.o liblibcrypto.a s_server only_server *~
