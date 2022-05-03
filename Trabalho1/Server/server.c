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

#define MAXDATASIZE 1000 // max number of bytes we can get at once 

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

//#########################
// Funções que se comunicam com o client
void listAll(int new_fd) {

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[1000] = "";

	fp = fopen("Movies/movies.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		strcat(result, line);
	}

	fclose(fp);
	if (line)
		free(line);

	if (send(new_fd, result, strlen(result), 0) == -1) {
		perror("send");
	}
}

void registerMovie(int new_fd) {

	int numbytes;
	char buf[MAXDATASIZE];

	if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\n';
	buf[numbytes+1] = '\0';

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("Movies/index.txt", "a");
	if (fp == NULL)
		printf("Erro\n");

	fprintf(fp,"%s", buf); //Titulo no index

	fclose(fp);
	if (line)
		free(line);

	if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) { //Filme todo
		perror("recv");
		exit(1);
	}
	
	fp = fopen("Movies/movies.txt", "a");
	if (fp == NULL)
		printf("Erro\n");

	fprintf(fp,"%s", buf); //Registra o filme

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

	char result[100] = "\nCadastrado, id é: ";
	strcat(result,idChar);

	if (send(new_fd, result, strlen(result), 0) == -1) {
		perror("send");
	}

}

void listId(int new_fd) {
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[1000] = "";

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

	if (send(new_fd, result, strlen(result), 0) == -1) {
		perror("send");
	}
}

void movieId(int new_fd) {

	int numbytes;
	char buf[MAXDATASIZE];

	if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

	int idMovie = atoi(buf);

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[1000] = "";

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

	if (send(new_fd, result, strlen(result), 0) == -1) {
		perror("send");
	}
}

void removeId(int new_fd) {

	int numbytes;
	char buf[MAXDATASIZE];

	if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

	int idMovie = atoi(buf);

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[1000] = "";

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


	char result2[1000] = "";

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
	strcat(removed,buf);

	if (send(new_fd, removed, strlen(removed), 0) == -1) {
		perror("send");
	}

}

// Acrescentar um novo gênero em um filme
// Preciso, qual filme(titulo), qual gen 
void acrescentaGen(int new_fd) {
	
	int numbytes;
	char title[MAXDATASIZE];
	char buf[MAXDATASIZE];

	if ((numbytes = recv(new_fd, title, 1000-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	title[numbytes] = '\n';
	title[numbytes + 1] = '\0';

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char result[1000] = "";
	int idMovie;
	int found = 0;
	int id = 0;

	fp = fopen("Movies/index.txt", "r");
	if (fp == NULL)
		printf("Erro\n");

	while ((read = getline(&line, &len, fp)) != -1) {
		id++;

		if (strcmp(title,line) == 0) {
			found = 1;
			break;
		}
	}

	fclose(fp);

	if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

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
				strcat(line, buf);
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

		char removed[100] = "Genero acrescentado no filme ";
		strcat(removed,title);

		if (send(new_fd, removed, strlen(removed), 0) == -1) {
			perror("send");
		}
	}
	else
	{
		char removed[100] = "Filme não encontrado";

		if (send(new_fd, removed, strlen(removed), 0) == -1) {
			perror("send");
		}
	}
	

}

// Listar informações (título, diretor(a) e ano) de todos os filmes de um determinado gênero
void listarGenAll(int new_fd) {
	int numbytes;
	char buf[MAXDATASIZE];

	if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';

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

		if ( (id + 2)%4==0 && strstr(line, buf) != NULL ){
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

	if (send(new_fd, result, strlen(result), 0) == -1) {
		perror("send");
	}
	
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

			// ##############################
			// Escolhe que função realizar
			int numbytes;
			char buf[1000];

			if ((numbytes = recv(new_fd, buf, 1000-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}

			buf[numbytes] = '\0';
			printf("%s\n",buf);

			if (strcmp(buf,"listAll")==0) { // Lista todos os filmes
				listAll(new_fd);
			}
			else if (strcmp(buf,"cadastrar")==0) { // Registra um filme
				registerMovie(new_fd);
			}
			else if (strcmp(buf,"listId")==0) { // Lista todos os títulos e Id
				listId(new_fd);
			}
			else if (strcmp(buf, "movieId") == 0) { // Lista infomações de um filme por Id
				movieId(new_fd);
			}
			else if (strcmp(buf, "removeId") == 0) { // Remove um filme
				removeId(new_fd);
			}

			else if (strcmp(buf, "acrescentaGen") == 0) { // Acrescentar um novo gênero em um filme
				acrescentaGen(new_fd);
			}
			else if (strcmp(buf, "listarGenAll") == 0) { // Listar informações (título, diretor(a) e ano) de todos os filmes de um determinado gênero
				listarGenAll(new_fd);
			}
			else {
				char error[100] = "Não existe essa função";

				if (send(new_fd, error, strlen(error), 0) == -1) {
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
