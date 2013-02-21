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
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MYPORT "4444"
#define MAXBUFLEN 100
#define DGRAM_SIZE 1024
#define HEADER_SIZE 4
#define CONFIRM 0

void send_dgram(int sockfd, const void *buf, size_t len, 
				const struct sockaddr *to, socklen_t tolen, int check) {

					int numbytes = sendto(sockfd, buf, len, 0, to, tolen);
					if (numbytes == -1) {
						perror("talker: sendto");
						exit(1);
					}

					printf("talker: sent %d bytes to \n", numbytes);

}

float timevaldiff(struct timeval *starttime, struct timeval *finishtime)
{
  float msec;
  msec= (float) (finishtime->tv_sec-starttime->tv_sec)*1000;
  msec+=((float) (finishtime->tv_usec-starttime->tv_usec))/1000;
  return msec;
}

int main(void)
{
	int bucket0 = 0;
	int bucket1 = 0;
	int bucket2 = 0;
	int bucket3 = 0;

	struct timeval begin_timer, end_timer, totalbegin_timer, totalend_timer;

	// TIME DIFFERENCE
	float time_difference;

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

	//int expectedSize = 663836;

	//printf("expected # of packets is: %d", expectedSize/DGRAM_SIZE + 1);

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

	//GET THE CURRENT TIME
	gettimeofday(&begin_timer, NULL);

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
	packetnum = 0;
	bool firstPacket = 1;
	int droppedPackets = 0;


	printf("listener: waiting to recvfrom...\n");

	if(CONFIRM == 0) {
		bool* receivedPackets = (bool*) malloc((expectedSize/DGRAM_SIZE + 1) * (sizeof (bool)));
		memset(receivedPackets, 0, (expectedSize/DGRAM_SIZE + 1) * (sizeof (bool)));
		int totalPackets = 0;
		int lastPacket = 0;
		
		int j;
		

		while (1){

			if ((numbytes = recvfrom(sockfd, curr_dgram,  HEADER_SIZE + DGRAM_SIZE, 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
			}
			//GET END TIME, COMPARE TO START TIME, SET NEXT START TIME
			gettimeofday(&end_timer, NULL);
			time_difference = timevaldiff(&begin_timer, &end_timer);

			//PRINT THE DELAY
			printf("delay: %f\n", time_difference);

			if((int)time_difference == 99){
				bucket0++;
			}
			if((int)time_difference == 100){
				bucket1++;
			}
			if((int)time_difference == 101){
				bucket2++;
			}
			if((int)time_difference == 102){
				bucket3++;
			}

			gettimeofday(&begin_timer, NULL);

			uint32_t i;
			for (i = 0; i < HEADER_SIZE; i++) {
				headerbuf[i] = curr_dgram[i];
			}

			currentpacket = (uint32_t)headerbuf[0] << 24 |
				(uint32_t)headerbuf[1] << 16 |
				(uint32_t)headerbuf[2] << 8  |
				(uint32_t)headerbuf[3];

			if (ntohl(currentpacket) != 0xffffffff){

				//printf("The current packet is: %d \n", ntohl(currentpacket));

				for (i = 0; i < DGRAM_SIZE; i++){
					content[ntohl(currentpacket)*DGRAM_SIZE + i] = curr_dgram[i+HEADER_SIZE];
				}
				receivedPackets[ntohl(currentpacket)] = 1;
				totalPackets++;
			}
			else{
				for(i = 0; i < expectedSize/DGRAM_SIZE + 1; i++){
					if (receivedPackets[i] == 0){

						while(1) {
							printf("i = %d\n", i);
							uint32_t curri;
							curri = htonl(i);
							printf("htonl(i) = %u\n", htonl(i));
							printf("curri = %u", curri);

							printf("requesting packet %u\n", curri);

							send_dgram(sockfd, &curri, sizeof(uint32_t), (struct sockaddr *)&their_addr, addr_len, 1);



							if ((numbytes = recvfrom(sockfd, curr_dgram,  HEADER_SIZE + DGRAM_SIZE, 0,
								(struct sockaddr *)&their_addr, &addr_len)) == -1) {
									perror("recvfrom");
									exit(1);
							}

							for (j = 0; j < HEADER_SIZE; j++) {
								headerbuf[j] = curr_dgram[j];
							}

							currentpacket = (uint32_t)headerbuf[0] << 24 |
								(uint32_t)headerbuf[1] << 16 |
								(uint32_t)headerbuf[2] << 8  |
								(uint32_t)headerbuf[3];

							if (ntohl(currentpacket) == i){
								//printf("The current packet is: %d \n", ntohl(currentpacket));
								int k;
								for (k = 0; k < DGRAM_SIZE; k++){
									content[ntohl(currentpacket)*DGRAM_SIZE + k] = curr_dgram[k+HEADER_SIZE];
								}
							    receivedPackets[ntohl(currentpacket)] = 1;
								totalPackets++;
								break;
							}
						}

					}
				}
			}
			if (totalPackets == expectedSize/DGRAM_SIZE + 1){
				gettimeofday(&totalend_timer, NULL);
				time_difference = timevaldiff(&totalbegin_timer, &totalend_timer);
				fwrite(content, sizeof(char), expectedSize, fp);
				printf("All packets received\n");
				printf("bucket0: %d\n", bucket0);
				printf("bucket1: %d\n", bucket1);
				printf("bucket2: %d\n", bucket2);
				printf("bucket3: %d\n", bucket3);
				send_dgram(sockfd, &intbuf2, sizeof(uint32_t), (struct sockaddr *)&their_addr, addr_len, 1);
				break;
			}
		}
	}

	else if (CONFIRM == 1) {
		while (packetnum < expectedSize/DGRAM_SIZE + 1){

			if (firstPacket == 0){
				printf("listener: waiting to recvfrom...\n");
			}

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


			if (ntohl(currentpacket) <= packetnum){
				send_dgram(sockfd, &intbuf2, sizeof(uint32_t), (struct sockaddr *)&their_addr, addr_len, 1);
				for (i = 0; i < DGRAM_SIZE; i++){
					content[ntohl(currentpacket)*DGRAM_SIZE + i] = curr_dgram[i+HEADER_SIZE];
				}
				if (ntohl(currentpacket) == packetnum) {
					packetnum++;
				}
				else{
					droppedPackets++;
				}
			}

			if ((expectedSize/DGRAM_SIZE + 1) == packetnum) {
				fwrite(content, sizeof(char), expectedSize, fp);
			}

			if (firstPacket == 1){
				firstPacket = 0;
			}

			printf("packetnum = %d \n", packetnum);
		}
	}
	close(sockfd);
	return 0;
}
