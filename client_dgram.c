/*
** needs to be compiled with -lm !
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
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#define SERVERPORT "4444"
#define IP "67.188.126.64"
#define FILEPATH "spongebob.jpg"
#define DGRAM_SIZE 1024
#define HEADER_SIZE 4
#define TIMEOUT 10 // in seconds
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define DELAY 0 // in microseconds
#define CONFIRM 0


int send_dgram(int sockfd, const void *buf, size_t len, 
  const struct sockaddr *to, socklen_t tolen, int check, float* roundtripsec, int* lostpackets) {

  clock_t t;

  t = clock();
  int numbytes = sendto(sockfd, buf, len, 0, to, tolen);
  if (numbytes == -1) {
    perror("talker: sendto");
    exit(1);
  }

  printf("Sent %d bytes to %s\n", numbytes, IP);

  if (check == 1) {
    struct sockaddr from;
    socklen_t fromlen;
    fromlen = sizeof(from);


    uint32_t recvbuff[1];
    printf("Waiting for confirmation...\n");

    numbytes = recvfrom(sockfd, recvbuff, 4, 0, &from, &fromlen);
    //printf("%d\n", numbytes);
    if (numbytes == -1) { // operation timed out
      printf("Confirmation not recieved, sending again...\n");
      //t = clock();
      *lostpackets += 1;
      send_dgram(sockfd, buf, len, to, tolen, check, roundtripsec, lostpackets);
    } else {
      if (0xffffffff == recvbuff[0]) {
        t = clock() - t;
        printf("Message reciept confirmed in %f seconds.\n", ((float) t)/CLOCKS_PER_SEC);
        *roundtripsec +=  ((float) t)/CLOCKS_PER_SEC;
      } else {
        printf("Message not confirmed, expecting 0xffffffff recieved %d\n", recvbuff[0]);
        send_dgram(sockfd, buf, len, to, tolen, check, roundtripsec, lostpackets);
      }
    }  
  }
  return 0;

}




int main(int argc, char *argv[])
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv, i;
  int numbytes;
  clock_t t1, t2;
  struct timeval tv, timestamp;
  float* roundtripsec;
  int* lostpackets;
//  float totaltime;

//  printf("%d\n", sizeof(struct timeval));

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  if ((rv = getaddrinfo(IP, SERVERPORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }
  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("talker: socket");
      continue;
    }
    break;
  }
  if (p == NULL) {
    fprintf(stderr, "talker: failed to bind socket\n");
    return 2;
  }


  // set socket timeout
  tv.tv_sec = TIMEOUT;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&tv, sizeof(struct timeval));

  
  //prepare file to be sent.
  struct stat info;
  rv = stat(FILEPATH, &info);
  if (rv !=0 ) {
    fprintf(stderr, "Error reading file.\n");
    exit(1);
  }
  printf("File Size: %d\n", (int)info.st_size);
  char* content = (char*) malloc(info.st_size * (sizeof (char)));
  FILE *fp = fopen(FILEPATH, "rb");
  fread(content, info.st_size, 1, fp);



  // send the file size
  uint32_t netlong = htonl((uint32_t) info.st_size);
  send_dgram(sockfd, &netlong, sizeof(uint32_t), p->ai_addr, p->ai_addrlen, 1, roundtripsec, lostpackets);



  // send the file
  char* curr_dgram = (char*) malloc((HEADER_SIZE + DGRAM_SIZE) * (sizeof(char)));
  uint32_t packetnum;
  int numofpackets = (int) ceil(((double) info.st_size)/DGRAM_SIZE);
  t1 = clock();
  *roundtripsec = 0;
  *lostpackets = 0;
  for (packetnum = 0; packetnum < numofpackets; packetnum++) {
    // add header
    memcpy(curr_dgram, &packetnum, 4);

    for (i = 0; i < MIN(DGRAM_SIZE, info.st_size - packetnum*DGRAM_SIZE); i++) {
      curr_dgram[i+HEADER_SIZE] = content[packetnum*DGRAM_SIZE+i];
    }
    printf("Sending part %d of %d...\n", packetnum+1, numofpackets);
    send_dgram(sockfd, curr_dgram, HEADER_SIZE+MIN(DGRAM_SIZE, info.st_size - packetnum*DGRAM_SIZE), 
        p->ai_addr, p->ai_addrlen, CONFIRM, roundtripsec, lostpackets);
 //   if (CONFIRM) { // If doing CONFIRM method, keep track of round trip times.
 //     totaltime += *roundtripsec;
 //   }
     
    // add a delay
    usleep(DELAY);
  }
  t2 = clock() - t1;
  printf("Sent %d packets in %f seconds.\n", numofpackets, ((float) t2)/CLOCKS_PER_SEC);
  
  if (CONFIRM) {
    printf("Average packet roundtrip time: %f\n", *roundtripsec / numofpackets);
    printf("Number of lost packets: %d\n", *lostpackets);
  }


  if (!CONFIRM) {


    // send a signal to say that we're done sending
    printf("Signaling end of data transfer.\n");
    uint32_t confirmation = -1;
    send_dgram(sockfd, &confirmation, 4, p->ai_addr, p->ai_addrlen, 0, NULL, NULL);




    // Listen for requests for missing packets and resend.
    *lostpackets = 0;
    while (1) {
      struct sockaddr from;
      socklen_t fromlen;
      fromlen = sizeof(from);

      uint32_t recvbuff[1];
      printf("Waiting for confirmation...\n");

      numbytes = recvfrom(sockfd, recvbuff, 4, 0, &from, &fromlen);
      if (numbytes == -1) {
        printf("Did not recieve confirmation, resending end signal.\n");
        send_dgram(sockfd, &confirmation, 4, p->ai_addr, p->ai_addrlen, 0, NULL, NULL);
      }
      if (recvbuff[0] == 0xffffffff) {
        t2 = clock() - t1;
        printf("Confirmation recieved, all data sent successfully in %f seconds.\n", ((float) t2)/CLOCKS_PER_SEC);
        printf("Total %d packets lost and rerequested.\n", *lostpackets);
        break;
      } else {
        packetnum = ntohl(recvbuff[0]);
        printf("Packet #%d was lost and was requested.\n", packetnum);
        *lostpackets += 1;
        memcpy(curr_dgram, &packetnum, 4);
        for (i = 0; i < MIN(DGRAM_SIZE, info.st_size - packetnum*DGRAM_SIZE); i++) {
          curr_dgram[i+HEADER_SIZE] = content[packetnum*DGRAM_SIZE+i];
        }
        send_dgram(sockfd, curr_dgram, HEADER_SIZE+MIN(DGRAM_SIZE, info.st_size - packetnum*DGRAM_SIZE), p->ai_addr, p->ai_addrlen, 0, NULL, NULL);
      }
    }


  }




  freeaddrinfo(servinfo);
  free(content);
  close(sockfd);
  return 0;
}


