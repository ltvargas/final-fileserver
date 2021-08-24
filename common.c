#include "common.h"

int open_listenfd(char *port) 
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Accept TCP connections */
    hints.ai_flags = AI_PASSIVE;      /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV; /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections */
    getaddrinfo(NULL, port, &hints, &listp);

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {

        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;  /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void *)&optval , sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        close(listenfd); /* Bind failed, try the next */
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 1024) < 0)
		return -1;
    return listenfd;
}


int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags = AI_NUMERICSERV; 
    hints.ai_flags |= AI_ADDRCONFIG; 
    getaddrinfo(hostname, port, &hints, &listp);
  
    for (p = listp; p; p = p->ai_next) {

        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; 
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
            break; 
        close(clientfd); 
    } 

    freeaddrinfo(listp);
    if (!p) 
        return -1;
    else   
        return clientfd;
}

void connection_error(int connfd)
{
	fprintf(stderr, "Error de conexión: %s\n", strerror(errno));
	close(connfd);
	exit(-1);
}

char *textcpy(char **nombreFi)
{
		char *nombreFile=nombreFi[0];
		size_t j=0;
		char *nombre=(char*)malloc(strlen(nombreFile)*sizeof(char));
		for(size_t i =0; i<strlen(nombreFile);++i){
			nombre[j++]=nombreFile[i];
		}
		nombre[j]=0;
		return nombre;
}

char *textcpyarray(char array[])
{
	size_t j=0;
	char *array_p=(char*)malloc(strlen(array)*sizeof(char));
	for(size_t i =0; i<strlen(array);++i){
		array_p[j++]=array[i];
	}
	array_p[j]=0;
	return array_p;

}

char **buf_tokens(char *linea, char *delim)
{
	
	char *strtak;
	char **tokens=(char **) calloc(sizeof(char),1);
	char *token;
	int i = 0;

	/* obtiene el primer token */
	token = strtok_r(linea, delim,&strtak);

	/* recorre todos los tokens */
	while( token != NULL ) {
		char *p=(char *)calloc(sizeof(char),1);
		p=token;
		tokens[i]=p;
		i++;
		token = strtok_r(NULL, delim,&strtak);
	}
	tokens[++i]=NULL;
	//for(int p=0; p<=i;i++) {printf("%s\n",tokens[p]); p++;}
	return tokens;
}

int cantidad(char ** comand){
    int i=0;
    while(comand[i]!=NULL){
        i++;
    }
    return i;
}
