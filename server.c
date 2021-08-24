#include <getopt.h>
#include <limits.h>
#include <sys/wait.h>
#include "common.h"
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

void *atender_cliente(void *vargp);

void print_help(char *command)
{
	printf("Servidor simple de ejecución remota de comandos.\n");
	printf("uso:\n %s <puerto>\n", command);
	printf(" %s -h\n", command);
	printf(" %s -d  <puerto>\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
	printf(" -d\t\t\tSe activa el formato Daemon--\n");
}

enum Estado{
	EXIT=0,
	INICIO,
	EXISTIR,
	LISTAR,
	Recibiendo_archivo,
	Enviando_archivo,
	COMANDOS,
	GENERARCLAVE,
	} estado;


int main(int argc, char **argv)
{
	int opt;

	//Sockets
	int listenfd, *connfd;
	unsigned int clientlen;
	//Direcciones y puertos
	struct sockaddr_in clientaddr;
	struct hostent;
	char *port;
	int flag=0;
	while ((opt = getopt (argc, argv, ":dh")) != -1){
		switch(opt)
		{
			case 'h':
				print_help(argv[0]);
				return 0;
			case 'd':
				flag=1;
				break;

			default:
				fprintf(stderr, "uso: %s <puerto>\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				return -1;
		}
	}

	if(argc==3) port=argv[2];
	else if(argc==2) port=argv[1];
	else{
		fprintf(stderr, "uso: %s <puerto>\n", argv[0]);
		fprintf(stderr, "     %s -h\n", argv[0]);
		return -1;

	}

	if(port!=NULL){
	//Valida el puerto
	int port_n = atoi(port);
	if(port_n <= 0 || port_n > USHRT_MAX){
		fprintf(stderr, "Puerto: %s invalido. Ingrese un número entre 1 y %d.\n", port, USHRT_MAX);
		return -1;
		}
	}else return -1;

	//Abre un socket de escucha en port
	listenfd = open_listenfd(port);

	if(listenfd < 0)
		connection_error(listenfd);

	printf("server escuchando en puerto %s...\n", port);
	
	if(flag==1){
		int fd0,fd1,fd2;
		pid_t pid;
		printf("Entrando modo Daemon\n");
		if((pid=fork())<0){ 
			fprintf(stderr,"NO es posible conectarse al modo Daemon..\n ");
			exit(1);
		}else if(pid!=0) exit(0);
		setsid();
		close(0);
		close(1);
		close(2);
		fd0=open("/dev/null",O_RDWR);
		fd1=dup(fd0);
		fd2=dup(fd0);

		if(fd0!= 0 || fd1 != 1 || fd2!=2){
			//syslog(LOG_ERR,"NO esperado!!..\n");
			exit(1);
		}
		

	}

	while (1) {

		pthread_t tid;
		clientlen = sizeof(clientaddr);
		
		connfd=malloc(sizeof(int));
		*connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		pthread_create(&tid,NULL,atender_cliente,connfd);
		//pthread_join(tid,NULL);
		//printf("Cliente Desconectado..");
		//pthread_detach(tid);
	}
}

void *atender_cliente(void *vargp)
{
	int connfd=*((int *) vargp);
	int n;
	char buf[MAXLINE] = {0};
	struct stat mi_stat;
	char *input_file=NULL;
	long int len=0;

	//FILE *file;
	int fd1,fd2;
	char *path="fileServer/";
	char *pathc="fileClient/";
	char dir[MAXLINE]={0};
	BLOWFISH_KEY key;


	//Claves
	//int i, c;
	uint8_t private1[DH_PRIVATE_KEY_SIZE]={0};
	uint8_t public1[DH_PUBLIC_KEY_SIZE]={0};
	uint8_t secret1[DH_SECRET_SIZE]={0};
	uint8_t public2[DH_PUBLIC_KEY_SIZE]={0}; //clave publica del cliente
	BYTE clave256[SHA256_BLOCK_SIZE]={0};
	const struct uECC_Curve_t * curves[5];
	enum estado;
	//char *p;
	int int_stat;
	char **comand;
	int *stats=0;
	pid_t pid;
	char **enlistar;
	char *comandoClave;
	BYTE enc_buf[BLOWFISH_BLOCK_SIZE]={0};
	BYTE temp[BLOWFISH_BLOCK_SIZE]={0};
	//int flag=0;
	while(1){

		
		n = read(connfd, buf, MAXLINE);
			if(n <= 0)
				estado=0;	
		buf[n-1] = '\0';
		comand= buf_tokens(buf," ");	 //aqui	
		//printf("%s\n",comand[0]);
		if(strcmp(comand[0],"START")==0){
						memset(private1,0,DH_PRIVATE_KEY_SIZE);
						memset(public1,0,DH_PUBLIC_KEY_SIZE);
						memset(public2,0,DH_PUBLIC_KEY_SIZE);
						memset(secret1,0,DH_SECRET_SIZE);
						memset(clave256,0,SHA256_BLOCK_SIZE);
						input_file=NULL;
						estado=INICIO;
		}
		else if(strcmp(comand[0],"GET")==0){
						input_file=NULL;
						memset(dir,0,MAXLINE);
						if(cantidad(comand)==2){
						input_file=comand[1];
						strcat(dir,path);
						strcat(dir,input_file);
						estado=EXISTIR;}
				        else estado=COMANDOS;
						
			} 
		else if (strcmp(comand[0],"PUT")==0){
				printf("RECIBO DE ARCHIVO..\n");
				input_file=NULL;
				memset(dir,0,MAXLINE);
				if(cantidad(comand)==4){
				input_file=comand[1];
				strcat(dir,path);
				strcat(dir,input_file);
				len=atoi(comand[2]);
				write(connfd,"READY\n",6);
				estado=Recibiendo_archivo;}
				else estado=COMANDOS;		

				}
		else if(strcmp(comand[0],"LIST")==0){
				if(secret1!=NULL){
					comandoClave=malloc(sizeof(char)*100);
					strcpy(comandoClave,"ls fileServer");
					enlistar = buf_tokens(comandoClave, " ");
					estado=LISTAR;
				}else estado=COMANDOS;
				
		}
		else if (strcmp(comand[0],"READY")==0){
						if (input_file==NULL){
						//write(connfd,"COMMAND NOT ACTION\n", 20);
						estado=COMANDOS;
						}
		}
		
 		else if (strcmp(comand[0],"CHAO")==0){
			write(connfd, "BYE\n", 4);	
			free(vargp);
			free(comand);
			estado=EXIT;
		} 
		else estado=COMANDOS;

		
		switch (estado){

			case INICIO:
					printf("--INTERCAMBIOS DE CLAVES--\n");
					int num_curves = 0;
					curves[num_curves++] = uECC_secp160r1();
					if (!uECC_make_key(public1, private1, curves[0])){
						printf("Error al generar claves..");
					}else {
						write(connfd,public1,DH_PUBLIC_KEY_SIZE); //envio de clave
						//estado=GENERARCLAVE;	
					}
				//break;

			case GENERARCLAVE:
					n = read(connfd, buf, MAXLINE);
					if(n <= 0)
					estado=0;	
					//p=comand[0];
					for(int pub=0; pub<DH_PUBLIC_KEY_SIZE;pub++) public2[pub]=buf[pub];
					if (!uECC_shared_secret(public2, private1, secret1, curves[0])) {
               				printf("shared_secret() failed (1)\n");
               				estado=COMANDOS;
					}
						SHA256_CTX ctx;
						sha256_init(&ctx);
						sha256_update(&ctx, secret1, DH_SECRET_SIZE);
						//memset(clave256,0,SHA256_BLOCK_SIZE);
						sha256_final(&ctx, clave256);
						printf("%s\n",secret1);
						memset(buf,0,MAXLINE);
						//exit(0);
						//flag=0;
					break;

			
			case Recibiendo_archivo:
					

				//	printf("\n%s\n",clave256);
					blowfish_key_setup(clave256, &key, BLOWFISH_KEY_SIZE);
					
					
					fd2= open(dir, O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0200|0400);
					for(int w=0;w<len;w+=BLOWFISH_BLOCK_SIZE){
						for(int j=0; j<BLOWFISH_BLOCK_SIZE; j++) temp[j]=0;
						n = read(connfd, buf, BLOWFISH_BLOCK_SIZE);
						if(n<=0) break;
						for(int d=0; d<n; d++) temp[d]=buf[d];
						blowfish_decrypt(temp,enc_buf, &key);
						//char *mandar=(char *)enc_buf;
						write(fd2,enc_buf,BLOWFISH_BLOCK_SIZE);
						memset(buf,0,BLOWFISH_BLOCK_SIZE);
						memset(enc_buf,0,BLOWFISH_BLOCK_SIZE);
						//len1+=8;
					}
					memset(buf,0,BLOWFISH_BLOCK_SIZE);
					memset(enc_buf,0,BLOWFISH_BLOCK_SIZE);
					close(fd2);
					printf("Subiendo y guardando en el disco duro..\n Se guardaron %ld bytes en el archivo: %s \n",len, dir );		
					
					memset(buf,0,MAXLINE);
					len=0;
					
					break;

			case EXISTIR:
					int_stat=stat(dir, &mi_stat);
					if((int_stat < 0)){
						//printf("NO EXISTE EL ARCHIVO\n");
						write(connfd,"0\n",2); //envia al cliente que no existe con 0 bytes.
						input_file=NULL;
						memset(dir, 0, MAXLINE);
					}else{
						memset(buf, 0, MAXLINE);
						len=(((int)mi_stat.st_size%8==0) ? mi_stat.st_size : (((int)mi_stat.st_size/8)*8+8));
						sprintf(buf, "%d", (int)mi_stat.st_size);
						char *len1=buf;
						printf("(%s bytes)...\n", len1);
						strcat(len1,"\n");
						 write(connfd,len1,strlen(len1)); // envio el tamano del archivo...
						memset(buf,0,MAXLINE);
						estado=Enviando_archivo;
					}	
					break;

			case Enviando_archivo:
				printf("ENVIANDO ARCHIVO SOLICITADO...\n");
				blowfish_key_setup(clave256, &key, BLOWFISH_KEY_SIZE);
				memset(buf,0,MAXLINE);
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
					write(connfd,&enc_buf,BLOWFISH_BLOCK_SIZE);
					//w+=BLOWFISH_BLOCK_SIZE;
					}
				close(fd1);
				input_file=NULL;
				len=0;
				memset(dir, 0, MAXLINE);		
			break;
			case LISTAR:
				if((pid = fork()) == 0){
					dup2(connfd, 1); //Redirecciona STDOUT al socket
					dup2(connfd, 2); //Redirecciona STDERR al socket
					if(execvp(enlistar[0], enlistar) < 0){
						fprintf(stderr, "Comando desconocido...\n");
						return NULL;
				}
			}
			wait(stats);
				free(enlistar);
				free(comandoClave);
			memset(buf,0,MAXLINE);
			break;
			case COMANDOS:
						input_file=NULL;
						len=0;
						write(connfd,"COMMAND NO ACTION-NO VALUES..\n", 30);
						//memset(buf,0,MAXLINE);
					break;
			case EXIT:
				pthread_cancel(pthread_self());	
				break;
			
		}
		
		//free(comand);
		memset(buf, 0, MAXLINE); //Encera el buffer
		//exit(0);
	}close(connfd);
}
