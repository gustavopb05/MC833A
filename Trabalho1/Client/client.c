/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 1000 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


//#########################################
// Funções que trocam mensagem com o servidor

void requestInfo(int sockfd,char *send_message) {

	int numbytes;
	char buf[MAXDATASIZE];

	if (send(sockfd, send_message, strlen(send_message), 0) == -1)
		perror("send");
	
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

	printf("%s\n",buf);
}

void requestRegister(int sockfd, char *send_message) {

	int numbytes, i = 0;
	char buf[MAXDATASIZE];
	char movie[1000] = "";

	if (send(sockfd, send_message, strlen(send_message), 0) == -1)
		perror("send");

	char infos[100];

	printf("Digite o título:\n");

	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	if (send(sockfd, infos, strlen(infos), 0) == -1)
		perror("send");

	strcat(movie, infos);
	strcat(movie, "\n");
	
	printf("Digite o genero:\n");
	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	strcat(movie, infos);
	strcat(movie, "\n");
	
	printf("Digite o diretor:\n");
	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	strcat(movie, infos);
	strcat(movie, "\n");

	printf("Digite o ano:\n");
	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	strcat(movie, infos);
	strcat(movie, "\n\0");

	if (send(sockfd, movie, strlen(movie), 0) == -1)
		perror("send");
	
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';
	printf("%s\n",buf);
}

void requestMovieId(int sockfd, char *send_message) {
	int numbytes, i = 0;
	char buf[MAXDATASIZE];
	char idChar[3] = "";

	if (send(sockfd, send_message, strlen(send_message), 0) == -1)
		perror("send");

	printf("Digite o id:\n");

	idChar[i] = getchar();    /* get the first character */
	while( idChar[i] != '\n' ){
		idChar[++i] = getchar(); /* gets the next character */
	}
	idChar[i] = '\0';
	i = 0;

	if (send(sockfd, idChar, strlen(idChar), 0) == -1)
		perror("send");

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';
	printf("%s\n",buf);
}

void requestAcrescentaGen(int sockfd, char *send_message){
	int numbytes, i = 0;
	char buf[MAXDATASIZE];
	char sChar[100] = "";

	if (send(sockfd, send_message, strlen(send_message), 0) == -1)
		perror("send");

	printf("Digite o Titulo:\n");

	sChar[i] = getchar();    /* get the first character */
	while( sChar[i] != '\n' ){
		sChar[++i] = getchar(); /* gets the next character */
	}
	sChar[i] = '\0';
	i = 0;

	if (send(sockfd, sChar, strlen(sChar), 0) == -1)
		perror("send");

	printf("Digite o genero:\n");

	sChar[i] = getchar();    /* get the first character */
	while( sChar[i] != '\n' ){
		sChar[++i] = getchar(); /* gets the next character */
	}
	sChar[i] = '\0';
	i = 0;

	if (send(sockfd, sChar, strlen(sChar), 0) == -1)
		perror("send");

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';
	printf("%s\n",buf);
}

void requestListarGenAll(int sockfd, char *send_message) {
	int numbytes, i = 0;
	char buf[MAXDATASIZE];
	char gChar[3] = "";

	if (send(sockfd, send_message, strlen(send_message), 0) == -1)
		perror("send");

	printf("Digite o genero:\n");

	gChar[i] = getchar();    /* get the first character */
	while( gChar[i] != '\n' ){
		gChar[++i] = getchar(); /* gets the next character */
	}
	gChar[i] = '\0';
	i = 0;

	if (send(sockfd, gChar, strlen(gChar), 0) == -1)
		perror("send");

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';
	printf("%s\n",buf);
}

//##########################################
// Função que abre a conexao e escolhe qual outra função pra enviar a mensagem

int sendServer(int argc, char *argv[], char *send_message) {
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	//###################################
	// Escolhe a função baseado na entrada

	if (strcmp(send_message,"listAll") == 0 || strcmp(send_message,"listId") == 0)
	{
		requestInfo(sockfd, send_message);
	}
	else if (strcmp(send_message, "movieId") == 0 || strcmp(send_message, "removeId") == 0)
	{
		requestMovieId(sockfd, send_message);	
	}
	else if(strcmp(send_message, "AcrescentaGen") == 0)
	{
		requestAcrescentaGen(sockfd, send_message);
	}
	else if(strcmp(send_message, "AcrescentaGen") == 0)
	{
		requestAcrescentaGen(sockfd, send_message);
	}
	else if (strcmp(send_message,"cadastrar") == 0)
	{
		requestRegister(sockfd, send_message);
	}
	else
	{
		requestInfo(sockfd, send_message);
	}
	

	close(sockfd);
	
	return 0;
}


int main(int argc, char *argv[])
{

	char message[MAXDATASIZE];
	int ret, i =0;

	while (1)
	{
		printf("\nAções disponíveis:\ncadastrar - Cadastra um novo filme\nlistAll - Lista todas informações de todos os filmes\nlistId - Lista tods os títulos e seu id\nmovieId - Infos de um filme dado seu id\nremoveId - Remove um filme pelo id\n\nDigite uma ação:\n\n");

		message[i] = getchar();    /* get the first character */
		while( message[i] != '\n' ){
			message[++i] = getchar(); /* gets the next character */
		}
		message[i] = '\0';
		i = 0;

		printf("\n");
	
		if(strcmp(message,"exit") == 0) {
			break;
		}
		else {
			ret = sendServer(argc, argv, message);
		}
	}
	return ret;
}
