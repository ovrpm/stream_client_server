#include <sys/socket.h>
#include <sys/types.h>
//#include <arpa/inet.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//#include <errno.h>

#define PORT "3950"
#define BUFFER_SIZE 20

int main(int argc, char *argv[]){
	if (argc != 2){
		printf("Usage: stream_client [server_name]\n");
		return 1;
	}

	struct addrinfo hints, *servinfo, *p;
	int rv, sockfd;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next){
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("socket");
			continue;
		}

		if ((rv = connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
			perror("connect");
			close(sockfd);
			continue;
		}
		break;
	}

	if (p == NULL){
		fprintf(stderr, "failed to connect to a socket\n");
		freeaddrinfo(servinfo);
		return 1;
	}

	freeaddrinfo(servinfo);

	printf("connection successful!\n");

	char buf[BUFFER_SIZE];
	ssize_t msg_len;

	if ((msg_len = recv(sockfd, buf, BUFFER_SIZE - 1, 0)) == -1){
		perror("recv");
		close(sockfd);
		return 1;
	}

	buf[msg_len] = '\0';

	printf("recieved: %s", buf);
	close(sockfd);

	return 0;
}
