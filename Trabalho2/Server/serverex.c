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

#define MYPORT "4950"	// the port users will be connecting to
#define CLIENTPORT "3490"

#define MAXBUFLEN 1000

struct IpMessage {
	const char *ip;
	const char *message;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sendMessage(const char *message, const char *ip) {
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

	
	if ((rv = getaddrinfo(ip, CLIENTPORT, &hints, &servinfo)) != 0) {
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

struct IpMessage receiveMessage() {
	int sockfd_rcv;
	struct addrinfo hints, *clientinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char buf[MAXBUFLEN];
	struct IpMessage ip_message;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &clientinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
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
		exit(1);
	}

	freeaddrinfo(clientinfo);
	
	printf("waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd_rcv, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	char *ip = inet_ntop(their_addr.ss_family,
						get_in_addr((struct sockaddr *)&their_addr),
						s, 
						sizeof s);

	printf("got packet from %s\n", ip);
	printf("packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("packet contains \"%s\"\n", buf);
	close(sockfd_rcv);

	ip_message.ip = ip;
	ip_message.message = buf;

	return ip_message;

}

void listAll(const char *ip) {

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int numbytes;
	char result[300] = "";
	struct timeval tv2;

	fp = fopen("Movies/movies.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		strcat(result, line);
	}

	fclose(fp);
	if (line)
		free(line);


	sendMessage(result, ip);
}

void listId(const char *ip){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[100] = "";
	struct timeval tv2;

	int id = 0;
	char idChar[3];

	fp = fopen("Movies/index.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		sprintf(idChar, "%d", id);
		strcat(result, idChar);
		strcat(result, " ");
		strcat(result, line);

	}

	fclose(fp);
	if (line)
		free(line);

	sendMessage(result, ip);
}

int main(void)
{
	struct IpMessage ip_message;
	char buf[100];
	while (1)
	{
		ip_message = receiveMessage();


		if (strcmp(ip_message.message,"listAll")==0) { // Lista todos os filmes
			listAll(ip_message.ip);
		}
		else if (strcmp(ip_message.message,"listId")==0) { // Lista todos os títulos e Id
			listId(ip_message.ip);
		}
		else
		{
			sendMessage("Não tem", ip_message.ip);
		}
		
	}
	return 0;
}