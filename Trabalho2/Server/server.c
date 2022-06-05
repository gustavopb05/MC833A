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

#define MAXBUFLEN 300

struct IpMessage {
	char ip[20];
	char message[MAXBUFLEN];
};

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
	printf("packet contains: \n%s\n", buf);
	close(sockfd_rcv);


	strcpy(ip_message.ip, ip);
	strcpy(ip_message.message, buf);

	return ip_message;

}

void listAll(char *ip) {

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int numbytes;
	char result[300] = "";

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

void listId(char *ip){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[100] = "";

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

void registerMovie(char *ip) {
	int numbytes;
	struct IpMessage ip_message;

	ip_message = receiveMessage();

	strcat(ip_message.message,"\n");
	strcat(ip_message.message,"\0");

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("Movies/index.txt", "a");
	if (fp == NULL)
		printf("Erro\n");

	fprintf(fp,"%s", ip_message.message); //Titulo no index

	fclose(fp);
	if (line)
		free(line);

	ip_message = receiveMessage(); // Filme todo
	
	fp = fopen("Movies/movies.txt", "a");
	if (fp == NULL)
		printf("Erro\n");

	fprintf(fp,"%s", ip_message.message); //Registra o filme

	fclose(fp);
	if (line)
		free(line);

	
	fp = fopen("Movies/index.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	int idCadastro = 0;
	while ((read = getline(&line, &len, fp)) != -1) { // Pega o número do cadastro
		idCadastro++;
	}

	fclose(fp);
	if (line)
		free(line);

	char idChar[3];
	sprintf(idChar, "%d", idCadastro); //Passa int pra string

	char result[40] = "\nCadastrado, id é: ";
	strcat(result,idChar);

	sendMessage(result, ip);
}

void movieId(char *ip) {

	struct IpMessage info = receiveMessage();

	int idMovie = atoi(info.message);

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[MAXBUFLEN] = "";

	int id = 0;
	char idChar[3];

	fp = fopen("Movies/movies.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		if (id >= (4*idMovie - 3) && id <= (4*idMovie)) {
			strcat(result, line);
		}
	}

	fclose(fp);
	if (line)
		free(line);

	sendMessage(result, ip);
}

void removeId(char *ip) {

	struct IpMessage info = receiveMessage();

	int idMovie = atoi(info.message);

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[MAXBUFLEN] = "";

	int id = 0;

	fp = fopen("Movies/movies.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		if (!(id >= (4*idMovie - 3) && id <= (4*idMovie))) {
			strcat(result, line);
		}
	}

	id = 0;

	fclose(fp);

	fp = fopen("Movies/movies.txt", "w");
	if (fp == NULL)
		printf("Erro\n");
	
	fprintf(fp,"%s", result);

	fclose(fp);


	char result2[MAXBUFLEN] = "";

	fp = fopen("Movies/index.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		if (id != idMovie ) {
			strcat(result2, line);
		}
	}

	fclose(fp);

	fp = fopen("Movies/index.txt", "w");
	if (fp == NULL)
		printf("Erro\n");
	
	fprintf(fp,"%s", result2);

	fclose(fp);
	if (line)
		free(line);

	char removed[100] = "Removido o filme ";
	strcat(removed,info.message);

	sendMessage(removed, ip);
}

void acrescentaGen(char *ip) {
	
	char title[MAXBUFLEN];

	struct IpMessage info = receiveMessage();
	strcpy(title, info.message);
	int ind = strlen(title);
	title[ind] = '\n';
	title[ind+1] = '\0';
	
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[MAXBUFLEN] = "";
	int idMovie;
	int found = 0;
	int id = 0;

	fp = fopen("Movies/index.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		if (strcmp(line,title) == 0) {
			found = 1;
			break;
		}
	}

	fclose(fp);

	info = receiveMessage();

	if (found == 1) {
		fp = fopen("Movies/movies.txt", "r");
		if (fp == NULL)
			printf("Erro\n");

		idMovie = id;
		id = 0 ;
		while ((read = getline(&line, &len, fp)) != -1) {
			id++;

			if ( id == (4*idMovie - 2) ) {
				line[strlen(line)-1] = '\0';

				strcat(line, ", ");
				strcat(line, info.message);
				strcat(line, "\n");
				strcat(result, line);
			}
			else
			{
				strcat(result, line);
			}
		}

		fclose(fp);

		fp = fopen("Movies/movies.txt", "w");
		if (fp == NULL)
			printf("Erro\n");
		
		fprintf(fp,"%s", result);

		fclose(fp);
		if (line)
			free(line);

		char returned[100] = "Genero acrescentado no filme ";
		strcat(returned,title);

		sendMessage(returned, ip);
	}
	else
	{
		char returned[100] = "Filme não encontrado";

		sendMessage(returned, ip);
	}
	
}

void listarGenAll(char *ip) {
	
	struct IpMessage info = receiveMessage();

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[1000] = "";
	int listaid[1000];
	int idcont = 0;
	int idincre = 0;
	int readOneMovie = 0;

	int id = 0;

	fp = fopen("Movies/movies.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		if ( (id + 2)%4==0 && strstr(line, info.message) != NULL ){
			listaid[idcont] = (id + 2)/4;
			idcont++;
		}
	}
	fclose(fp);

	fp = fopen("Movies/movies.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	id = 0;

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		if (idincre < idcont && id >= (4*listaid[idincre] - 3) && id <= (4*listaid[idincre])) {
			strcat(result, line);
			readOneMovie++;
			if (readOneMovie == 4)
			{
				readOneMovie = 0;
				idincre++;
			}
		}
	}
	fclose(fp);

	sendMessage(result, ip);
	
}

int main(void)
{
	struct IpMessage ip_message;
	char buf[100];
	while (1)
	{
		ip_message = receiveMessage();


		if (strcmp(ip_message.message,"listAll") == 0) { // Lista todos os filmes
			listAll(ip_message.ip);
		}
		else if (strcmp(ip_message.message,"listId") == 0) { // Lista todos os títulos e Id
			listId(ip_message.ip);
		}
		else if (strcmp(ip_message.message, "cadastrar") == 0) {
			registerMovie(ip_message.ip);
		}
		else if (strcmp(ip_message.message, "movieId") == 0) { // Lista infomações de um filme por Id
			movieId(ip_message.ip);
		}
		else if (strcmp(ip_message.message, "removeId") == 0) { // Remove um filme
			removeId(ip_message.ip);
		}
		else if (strcmp(ip_message.message, "acrescentaGen") == 0) { // Acrescentar um novo gênero em um filme
			acrescentaGen(ip_message.ip);
		}
		else if (strcmp(ip_message.message, "listarGenAll") == 0) { // Listar informações (título, diretor(a) e ano) de todos os filmes de um determinado gênero
			listarGenAll(ip_message.ip);
		}

		else
		{
			sendMessage("Não tem", ip_message.ip);
		}
		
	}
	return 0;
}