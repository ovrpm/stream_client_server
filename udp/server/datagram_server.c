#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define PORT "3960"
#define BUFFER_LENGTH 20

int main(){
	struct addrinfo hints, *servinfo, *p;
	int rv, sockfd;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next){
		if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
			perror("socket");
			continue;
		}

		if ((rv = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1){
			perror("bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL){
		fprintf(stderr, "failed to init a socket\n");
		return 2;
	}

	struct sockaddr_storage their_addr;
	socklen_t addrlen = sizeof(their_addr);
	char buf[BUFFER_LENGTH];
	ssize_t msg_size;

	if ((msg_size = recvfrom(sockfd, buf, BUFFER_LENGTH - 1, 0, (struct sockaddr *)&their_addr, &addrlen)) == -1){
		perror("recvfrom");
		return 3;
	}

	buf[msg_size] = '\0';
	printf("received: %s\n", buf);

	close(sockfd);

	return 0;
}
