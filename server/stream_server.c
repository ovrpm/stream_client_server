#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <netdb.h>              // getaddrinfo & struct addrinfo
#include <netinet/in.h>         // get_in_addr
#include <stdio.h>              // fprintf, perror
#include <sys/socket.h>         // socket
#include <unistd.h>             // close

#define PORT "3950"
#define BACKLOG 10

void sigchld_handler(int s){
	(void)s;

	int saved_err = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_err;
}

void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void){
	struct addrinfo hints, *servinfo, *p;
	int err, sockfd;
	int yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((err = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "in getaddrinfo:\n%s\n", gai_strerror(err));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next){
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("socket:");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
					&yes, sizeof(yes)) == -1){
			perror("setsockopt:");
			return 1;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("bind:");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL){
		fprintf(stderr, "failed to bind\n");
		return 1;
	}

	if (listen(sockfd, BACKLOG)){
		perror("listen:");
		return 1;
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&(sa.sa_mask));

	if (sigaction(SIGCHLD, &sa, NULL)){
		perror("sigaction");
		return 1;
	}
	
	printf("server: waiting for connections...\n");

	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	int new_fd;
	char s[INET6_ADDRSTRLEN];

	while (1){
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1){
			perror("accept:");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof(s));
		printf("connection from %s\n", s);

		if (!fork()){
			close(sockfd);
			const char *msg = "Hello, world!\n";
			if (send(new_fd, msg, strlen(msg), 0) == -1){
				perror("send");
			}
			return 0;
		}

		close(new_fd);
	}
	
	return 0;
}
