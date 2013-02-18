/*
** listener.c -- a datagram sockets "server" demo
*/
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
#define MYPORT "4444"
// the port users will be connecting to
#define MAXBUFLEN 100
#define DGRAM_SIZE 1024
#define HEADER_SIZE 4

void send_dgram(int sockfd, const void *buf, size_t len, 
				const struct sockaddr *to, socklen_t tolen, int check) {

					int numbytes = sendto(sockfd, buf, len, 0, to, tolen);
					if (numbytes == -1) {
						perror("talker: sendto");
						exit(1);
					}

					printf("talker: sent %d bytes to \n", numbytes);

}

int main(void)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	int size;

	socklen_t addr_len;
	char s[INET_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(servinfo);
	printf("listener: waiting to recvfrom...\n");
	addr_len = sizeof their_addr;

	uint32_t intbuf[1];  

	if ((numbytes = recvfrom(sockfd, /*buf*/intbuf, /*MAXBUFLEN-1*/4 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
	}

	//get_in_addr((struct sockaddr *)&their_addr)
	printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
		&(((struct sockaddr_in*)(struct sockaddr *)&their_addr)->sin_addr), s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);

	intbuf[0] = ntohl(intbuf[0]);

	printf("listener: packet contains %d \n", intbuf[0]);

	int expectedSize = intbuf[0];

	//size = 4 * (sizeof (char));

	uint32_t intbuf2[1];

	intbuf2[0] = -1;

	send_dgram(sockfd, &intbuf2, sizeof(uint32_t), (struct sockaddr *)&their_addr, addr_len, 1);

	char* content = (char*) malloc(expectedSize * (sizeof (char)));
	char* curr_dgram = (char*) malloc((HEADER_SIZE + DGRAM_SIZE) * (sizeof(char)));
	char headerbuf[4];
	uint32_t packetnum, currentpacket;
	FILE *fp = fopen("/output_dgram.jpg", "wb");



	printf("listener: waiting to recvfrom...\n");

	packetnum = 0;

	while (packetnum < expectedSize/DGRAM_SIZE + 1){

		if ((numbytes = recvfrom(sockfd, curr_dgram,  HEADER_SIZE + DGRAM_SIZE, 0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
		}

		int i;
		for (i = 0; i < HEADER_SIZE; i++) {
			headerbuf[i] = curr_dgram[i];
		}

		currentpacket = (uint32_t)headerbuf[0] << 24 |
						(uint32_t)headerbuf[1] << 16 |
						(uint32_t)headerbuf[2] << 8  |
						(uint32_t)headerbuf[3];
		printf("The current packet is: %d \n", ntohl(currentpacket));
		if (ntohl(currentpacket) == packetnum){
			for (i = 0; i < DGRAM_SIZE; i++){
				content[packetnum*DGRAM_SIZE + i] = curr_dgram[i+HEADER_SIZE];
			}
			send_dgram(sockfd, &intbuf2, sizeof(uint32_t), (struct sockaddr *)&their_addr, addr_len, 1);
			packetnum++;
		}
		if (((expectedSize/DGRAM_SIZE + 1) - packetnum) == 1){
			fwrite(content, sizeof(char), expectedSize, fp);
		}
	}



	close(sockfd);
	return 0;
}