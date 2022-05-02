/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct Movie{
	char title[70];
	char id[3];
	char gen[2][70];
	char dir[70];
	char year[70];
};


// Database
struct Movie db[100];
int registered_ids[100];

//Lista com booleanos 
char * cadastrar(){

	int id;
	char *charId;

	for (int i = 0; i < 100; i++)
	{
		if (registered_ids[i] == 0) {
			id = i;
			registered_ids[i] = 1;
			break;
		}
	}

	sprintf(charId, "%d", id); 
	
	strcpy(db[id].dir, "Wachowski");
	strcpy(db[id].title, "Matrix");
	strcpy(db[id].year, "1999");
	strcpy(db[id].gen[0], "Sci-fi");
	strcpy(db[id].gen[1], "Ação");
	strcpy(db[id].id, charId);

	return charId;
}

char * addgen(struct Movie db[100]){

}

char * listtitles(struct Movie db[100]){

}

char * listgen(struct Movie db[100]){

}

char * listall(struct Movie db[100]){

}

char * listid(struct Movie db[100]){

}

char * removeid(struct Movie db[100]){

}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;


	//Init database
	for (int i = 0; i < 100; i++)
	{
		registered_ids[i] = 0;
	}

	strcpy(db[0].dir, "Peter Jackson");
	strcpy(db[0].title, "Lord of the Rings: The Fellowship of the ring");
	strcpy(db[0].year, "2001");
	strcpy(db[0].gen[0], "Aventura");
	strcpy(db[0].gen[1], "Ação");
	strcpy(db[0].id, "0");

	registered_ids[0] = 1;



	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			// Testing custom send recv
			int numbytes;
			char buf[1000];

			if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}

			buf[numbytes] = '\0';
			printf("%s\n",buf);

			if (strcmp(buf,"teste")==0) {
				char result[] = "Título: ";
				strcat(result,db[0].title);
				strcat(result, "\nGênero: ");
				strcat(result,db[0].gen[0]);
				strcat(result, ", ");
				strcat(result,db[0].gen[1]);
				strcat(result,"\nDiretor(a): ");
				strcat(result,db[0].dir);
				strcat(result,"\nAno de Lançamento: ");
				strcat(result, db[0].year);
				strcat(result,"\nid: ");
				strcat(result, db[0].id);
				strcat(result,"\n");

				if (send(new_fd, result, strlen(result), 0) == -1) {
					perror("send");
				}

			}
			else if (strcmp(buf,"cadastrar")==0) {

				char *charId = cadastrar();

				char result[] = "Cadastrado, id do filme é: ";
				strcat(result,charId);

				if (send(new_fd, result, strlen(result), 0) == -1) {
					perror("send");
				}
			}
			else {
				buf[numbytes] = '+';

				if (send(new_fd, buf, strlen(buf), 0) == -1) {
					perror("send");
				}
			}

			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
