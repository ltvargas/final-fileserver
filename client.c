#include <getopt.h>
#include <limits.h>


#include "common.h"

void print_help(char *command)
{
	printf("Cliente simple de descarga de archivos.\n");
	printf("uso:\n %s <hostname> <puerto>\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
}

char *hex(const char * data, size_t size)
{
    char *enviar=(char *)calloc(sizeof(char),size);
    int i;
    for(i = 0; i < size; ++i)
        sprintf(enviar,"%02x",(unsigned char) data[i]);
    return enviar;
}

enum Estado{
	EXIT=0,
	INICIO,
	OTROS,
	GENERARCLAVE,
	SUBIR,
	INVALIDO,
	LISTAR,
	LISTO
} estado;

int main(int argc, char **argv)
{
	int opt;

	//Socket
	int clientfd;
	//Direcciones y puertos
	char *hostname, *port;

	//Lectura desde consola
	char *linea_consola;
	char read_buffer[MAXLINE] = {0};
	size_t max = MAXLINE;
	ssize_t n, l = 0;

	//File
	//FILE *file;
	int fd2,fd1;
	char *input_file="";
	char *path="fileClient/";
	char *paths="fileServer/";
	char dir[MAXLINE]={0};

	//Automata
	enum estado;
	//Blowfish
	BLOWFISH_KEY key;

	//Claves
	uint8_t private2[DH_PRIVATE_KEY_SIZE]={0};
	uint8_t public2[DH_PUBLIC_KEY_SIZE]={0};
	uint8_t secret2[DH_SECRET_SIZE]={0};
	uint8_t public1 [DH_PUBLIC_KEY_SIZE]={0};//clave publica del servidor
	BYTE clave256[SHA256_BLOCK_SIZE]={0};
	while ((opt = getopt (argc, argv, "h")) != -1){
		switch(opt)
		{
			case 'h':
				print_help(argv[0]);
				return 0;
			default:
				fprintf(stderr, "uso: %s <hostname> <puerto>\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				return -1;
		}
	}

	if(argc != 3){
		fprintf(stderr, "uso: %s <hostname> <puerto>\n", argv[0]);
		fprintf(stderr, "     %s -h\n", argv[0]);
		return -1;
	}else{
		hostname = argv[1];
		port = argv[2];
	}

	//Valida el puerto
	int port_n = atoi(port);
	if(port_n <= 0 || port_n > USHRT_MAX){
		fprintf(stderr, "Puerto: %s invalido. Ingrese un número entre 1 y %d.\n", port, USHRT_MAX);
		return -1;
	}

	//Se conecta al servidor retornando un socket conectado
	clientfd = open_clientfd(hostname, port);

	if(clientfd < 0)
		connection_error(clientfd);

	printf("Conectado exitosamente a %s en el puerto %s.\n", hostname, port);

	linea_consola = (char *) calloc(1, MAXLINE);
	//printf("Ingrese texto para enviar al servidor, escriba CHAO para terminar...\n");
	printf("ESCRIBA EL COMANDO --START-- PARA INICIAR LA COMUNICACION..\n");
	//printf("Ingrese el nombre del archivo a descargar: \n");
	printf("> ");
	l = getline(&linea_consola, &max, stdin); //lee desde consola
	long int numByte=0;
	int num_curves;
	const struct uECC_Curve_t * curves[5];
	//char **clave;
	int flags=0;
	long int len=0;
	bool continuar;
	while(l > 0){
		n = write(clientfd, linea_consola, l); //Envia al servidor
		if(n<=0)break;
		char **comand;
		linea_consola[n-1] = '\0';
		comand= buf_tokens(linea_consola," "); //aqui
		
		
		if((strcmp(comand[0],"START")==0) && (flags==0)){
			input_file=NULL;
			estado=INICIO;
			flags=1;
		}else if(flags==1){
			if(strcmp(comand[0],"READY")!=0 ){
			memset(read_buffer,0,MAXLINE);	
			n = read(clientfd, read_buffer, MAXLINE); //Lee respuesta del servidor
			if(n<=0)break;
			
		    }	
			if(strcmp(comand[0],"GET")==0){
				input_file=NULL;
				numByte=0;
				memset(dir,0,MAXLINE);
				if(cantidad(comand)==2){
				input_file=comand[1];
				strcat(dir,path);
				strcat(dir,input_file);
				estado=OTROS;}
				else estado=INVALIDO;
						
				
			}else if(strcmp(comand[0],"PUT")==0){			
				input_file=NULL;
				numByte=0;
				memset(dir,0,MAXLINE);
				if(cantidad(comand)==4){
				input_file=comand[1];
				strcat(dir,path);
				strcat(dir,input_file);
				len=atoi(comand[2]);
				if(strcmp(read_buffer,"READY\n")==0){
					estado=SUBIR;
				}		
				}else{estado=INVALIDO;}
		}
			else if(strcmp(comand[0],"LIST")==0){			
				input_file=NULL;
				numByte=0;
				memset(dir,0,MAXLINE);
				if(cantidad(comand)==1){
				estado=LISTAR;
				}else estado=INVALIDO;
			}
			else if(strcmp(comand[0],"READY")==0){
				if(numByte<=0) estado=INVALIDO;
			}else if(strcmp(read_buffer,"BYE\n")==0){
				printf("Recibido: %s", read_buffer);
				printf("Desconectando...\n");
				free(comand);
				free(linea_consola);
				l=0;
				estado=0;
			}
			else{
				estado=INVALIDO;
			}

		}else if(strcmp(comand[0],"CHAO")==0){
				estado=EXIT;
		}else{
			n = read(clientfd, read_buffer, MAXLINE); //Lee respuesta del servidor
			if(n<=0)break;
			estado=INVALIDO;
		}

	switch (estado){

		case INICIO:
					memset(private2,0,DH_PRIVATE_KEY_SIZE);
					memset(public2,0,DH_PUBLIC_KEY_SIZE);
					memset(public1,0,DH_PUBLIC_KEY_SIZE);
					memset(secret2,0,DH_SECRET_SIZE);
					memset(clave256,0,SHA256_BLOCK_SIZE);
					printf("GENERAR CLAVES\n");
					num_curves = 0;
					curves[num_curves++] = uECC_secp160r1();
					if (!uECC_make_key(public2, private2, curves[0])){
						printf("Error al generar claves..");
					} else {
						memset(linea_consola,0,MAXLINE);
						//memset(read_buffer,0,MAXLINE);
						estado=GENERARCLAVE; 
						
						}
					//break;

		case GENERARCLAVE:
						n = read(clientfd, read_buffer, MAXLINE); //Lee respuesta del servidor
						if(n<=0)break;
						memset(public1,0,DH_PUBLIC_KEY_SIZE);
						//char *cclave=" clave";
						//strcat((char *)public2,cclave);
						write(clientfd,public2,DH_PUBLIC_KEY_SIZE);
						//char *p=clave[0];
						for(int publica=0;publica<DH_PUBLIC_KEY_SIZE;publica++) public1[publica]=read_buffer[publica];
						//printf("%s\n",private2);
						if (!uECC_shared_secret(public1, private2, secret2, curves[0])) {
								printf("shared_secret() failed (1)\n");
								estado=INVALIDO;
							}
						
					SHA256_CTX ctx;
					sha256_init(&ctx);
					sha256_update(&ctx, secret2, DH_SECRET_SIZE);
					sha256_final(&ctx, clave256);
					printf("%s",secret2);
					memset(read_buffer,0,MAXLINE);
					memset(linea_consola,0,MAXLINE);
					printf("\n> ");
					l = getline(&linea_consola, &max, stdin);
					break;
		case LISTO:
					blowfish_key_setup(clave256, &key, BLOWFISH_KEY_SIZE);
					
					
					BYTE enc_buf[BLOWFISH_BLOCK_SIZE]={0};
					BYTE temp[BLOWFISH_BLOCK_SIZE]={0};
					fd2= open(dir, O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0200|0400);
					for(int w=0;w<numByte;w+=BLOWFISH_BLOCK_SIZE){
						for(int j=0; j<BLOWFISH_BLOCK_SIZE; j++) temp[j]=0;
						n = read(clientfd, read_buffer, BLOWFISH_BLOCK_SIZE);
						if(n<=0) break;
						for(int d=0; d<n; d++) temp[d]=read_buffer[d];
						blowfish_decrypt(temp,enc_buf, &key);
						//char *mandar=(char *)enc_buf;
						write(fd2,enc_buf,BLOWFISH_BLOCK_SIZE);
						memset(read_buffer,0,BLOWFISH_BLOCK_SIZE);
						memset(enc_buf,0,BLOWFISH_BLOCK_SIZE);
						//len1+=8;
					}
					memset(read_buffer,0,BLOWFISH_BLOCK_SIZE);
					memset(enc_buf,0,BLOWFISH_BLOCK_SIZE);
					close(fd2);
					printf("Descargando y guardando en el disco duro..\n Se guardaron %ld bytes en el archivo: %s \n", numByte, dir );		
					printf("Ingrese el nombre del archivo a descargar: ");
					memset(read_buffer,0,MAXLINE);
					numByte=0;
					printf("\n> ");
					l = getline(&linea_consola, &max, stdin);	
				
				break;
		case SUBIR:
				printf("ENVIANDO ARCHIVO SOLICITADO...\n");
			//	printf("\n%s\n",clave256);
				blowfish_key_setup(clave256, &key, BLOWFISH_KEY_SIZE);
				memset(read_buffer,0,MAXLINE);
				
				//int w=0;
				fd1=open(dir,O_RDONLY);
				BYTE buffer[BLOWFISH_BLOCK_SIZE]={0};

				for(int w=0;w<len;w+=BLOWFISH_BLOCK_SIZE){
					memset(buffer,0,BLOWFISH_BLOCK_SIZE);
					memset(enc_buf,0,BLOWFISH_BLOCK_SIZE);
					for(int j=0; j<BLOWFISH_BLOCK_SIZE; j++) temp[j]=0;
					int cantidad=read(fd1,&buffer,BLOWFISH_BLOCK_SIZE);
					for(int d=0; d<cantidad; d++) temp[d]=buffer[d];
					blowfish_encrypt(temp, enc_buf, &key);
					write(clientfd,&enc_buf,BLOWFISH_BLOCK_SIZE);
					//w+=BLOWFISH_BLOCK_SIZE;
					}
				close(fd1);
				input_file=NULL;
				len=0;
				memset(read_buffer,0,MAXLINE);
				memset(dir, 0, MAXLINE);
				printf("Ingrese el nombre del archivo a descargar: ");
				printf("\n> ");
					l = getline(&linea_consola, &max, stdin);
				break; 
		
		case OTROS:
					strtok(read_buffer,"\n");
			        //strtok(linea_consola,"\n");
					printf("Recibido: %s", read_buffer);
					numByte= atoi(read_buffer);
					numByte=((numByte%8==0) ? numByte: ((numByte/8)*8+8));
					memset(linea_consola,0,MAXLINE);
					estado=LISTO;
					//free(comand);
			      	printf("\n> ");
				    l = getline(&linea_consola, &max, stdin);
					
				break;
		case LISTAR:

			continuar = false;
			do{
				n = recv(clientfd, read_buffer, MAXLINE, MSG_DONTWAIT);
				if(n < 0){
					if(errno == EAGAIN) //Vuelve a intentar
						continuar = true;
					else
						continuar = false;
				}else if(n == MAXLINE) //Socket lleno, volver a leer
					continuar = true;
				else if(n == 0)
					continuar = false;
				else{ 
					char c = read_buffer[n - 1]; //Busca '\0' para detectar fin
					if(c == '\0')
						continuar = false;
					else
						continuar = true;
				}
				printf("%s", read_buffer);
				memset(read_buffer,0,MAXLINE); 
				printf("\n> ");
				l = getline(&linea_consola, &max, stdin);//Encerar el buffer
			}while(continuar);
			dup2(clientfd,2);
			dup2(clientfd,1);
			
		

		break;
		case INVALIDO:
					printf("Recibido: %s", read_buffer);
					printf("\n> ");
					l = getline(&linea_consola, &max, stdin);
		break;

		case EXIT:
				l=0;
				break;
		
		free(comand);
		}
	}close(clientfd);
}

