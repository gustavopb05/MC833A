#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#define SERVERPORT "4950"	// the port users will be connecting to
#define MYPORT "3490"

#define MAXBUFLEN 300

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sendMessage(char *message, char *ip) {
	int sockfd_snd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	
	if ((rv = getaddrinfo(ip, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd_snd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "failed to create socket\n");
		return;
	}

	freeaddrinfo(servinfo);

	if ((numbytes = sendto(sockfd_snd, message, strlen(message), 0,
			p->ai_addr, p->ai_addrlen)) == -1) {
		perror("sendto");
		exit(1);
	}

	printf("sent %d bytes to %s\n", numbytes, ip);
	close(sockfd_snd);
	return;
}

void receiveMessage() {
	int sockfd_rcv;
	struct addrinfo hints, *clientinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char buf[MAXBUFLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &clientinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for(p = clientinfo; p != NULL; p = p->ai_next) {
		if ((sockfd_rcv = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("socket");
			continue;
		}

		if (bind(sockfd_rcv, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd_rcv);
			perror("bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "failed to bind socket\n");
		return;
	}

	freeaddrinfo(clientinfo);
	
	printf("waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd_rcv, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("packet contains:\n%s\n", buf);
	close(sockfd_rcv);
	return;

}

void requestInfo(char *message, char *ip) {
	
	sendMessage(message, ip);

	receiveMessage();
}

void requestRegister(char *message, char *ip) {
	sendMessage(message, ip);
	
	char infos[100];
	char movie[200] = "";
	int i = 0;

	printf("Digite o título:\n");

	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	sendMessage(infos, ip);

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

	sendMessage(movie, ip);

	receiveMessage();
}

void requestMovieId(char *buf, char *ip){
	sendMessage(buf, ip);

	char infos[100];
	int i = 0;

	printf("Digite o id do filme:\n");

	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	sendMessage(infos, ip);

	receiveMessage();
}

void requestAcrescentaGen(char *message, char *ip){
	sendMessage(message, ip);

	char infos[100];
	int i = 0;

	printf("Digite o título:\n");

	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	sendMessage(infos, ip);

	printf("Digite o novo genero:\n");

	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	sendMessage(infos, ip);

	receiveMessage();
}

void requestListarGen(char *message, char *ip){
	sendMessage(message, ip);

	char infos[100];
	int i = 0;

	printf("Digite o genero:\n");

	infos[i] = getchar();    /* get the first character */
	while( infos[i] != '\n' ){
		infos[++i] = getchar(); /* gets the next character */
	}
	infos[i] = '\0';
	i = 0;

	sendMessage(infos, ip);

	receiveMessage();
}



int main(int argc, char *argv[])
{

	if (argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}

	while (1)
	{
		char buf[MAXBUFLEN];
		int i = 0;

		printf("Digite uma mensagem pra enviar\n");
		buf[i] = getchar();    /* get the first character */
		while( buf[i] != '\n' ){
			buf[++i] = getchar(); /* gets the next character */
		}
		buf[i] = '\0';
		i = 0;

		if (strcmp(buf,"listAll") == 0 || strcmp(buf,"listId") == 0)
		{
			requestInfo(buf, argv[1]);
		}
		else if (strcmp(buf,"cadastrar") == 0)
		{
			requestRegister(buf, argv[1]);
		}
		else if (strcmp(buf, "movieId") == 0 || strcmp(buf, "removeId") == 0)
		{
			requestMovieId(buf, argv[1]);	
		}
		else if (strcmp(buf,"acrescentaGen") == 0)
		{
			requestAcrescentaGen(buf, argv[1]);
		}
		else if (strcmp(buf,"listarGen") == 0)
		{
			requestListarGen(buf, argv[1]);
		}

		else if (strcmp(buf, "exit") == 0){
			break;
		}
		else
		{
			requestInfo(buf,argv[1]);
		}
		

	}

	return 0;
}