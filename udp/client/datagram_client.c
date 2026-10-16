#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define PORT "3960"

int main(int argc, char *argv[]){
	if (argc != 3){
		printf("usage: ./datagram_client <address> <message>");
		return 1;
	}

	struct addrinfo hints, *talker_addrinfo, *p;
	int rv, sockfd;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &talker_addrinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = talker_addrinfo; p != NULL; p = p->ai_next){
		if ((sockfd = socket(talker_addrinfo->ai_family, talker_addrinfo->ai_socktype, talker_addrinfo->ai_protocol)) == -1){
			perror("socket");
			continue;
		}

		break;
	}

	freeaddrinfo(talker_addrinfo);

	if (p == NULL){
		fprintf(stderr, "socket failed\n");
		return 1;
	}

	if ((rv = sendto(sockfd, argv[2], strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen)) == -1){
		perror("sendto");
		return 1;
	}

	close(sockfd);

	return 0;
}
