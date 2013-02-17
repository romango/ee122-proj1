/*
** talker.c -- a datagram "client" demo
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
#define SERVERPORT "4444"
#define IP "67.188.126.64"
#define FILEPATH "pic.jpg"
#define DGRAM_SIZE 1024

void send_dgram(int sockfd, const void *buf, size_t len, 
  const struct sockaddr *to, socklen_t tolen, int check) {

  int numbytes = sendto(sockfd, buf, len, 0, to, tolen);
  if (numbytes == -1) {
    perror("talker: sendto");
    exit(1);
  }

  printf("talker: sent %d bytes to %s\n", numbytes, IP);


//  if (check == 1) {
//    char recvbuff[];
//    numbytes = recvfrom(sockfd, recvbuff, 4, 0, to, tolen);
//    if (0xffffffff & recvbuff) {
//        printf("Message reciept confirmed.\n");
//    } else {
//      printf("Message not confirmed

}




// the port users will be connecting to
int main(int argc, char *argv[])
{
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;
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


  uint32_t netlong = htonl((uint32_t) info.st_size);
  printf("Network long: %d\n", netlong);
  printf("address: %d\n", ntohl((uint32_t) &netlong));
  send_dgram(sockfd, &netlong, sizeof(uint32_t), p->ai_addr, p->ai_addrlen, 1);


  char* curr_dgram = (char*) malloc((1 + DGRAM_SIZE) * (sizeof(char)));

 // int packetnum;
 // for (packetnum = 
  
 // curr_dgram[0] = (char) header;
 // int i;
 // for (i = 1

  


//numbytes = sendto(sockfd, content, info.st_size, 0,p->ai_addr, p->ai_addrlen);


  //send_dgram(sockfd, content, 4, p->ai_addr, p->ai_addrlen, 0);



  freeaddrinfo(servinfo);
  free(content);
  close(sockfd);
  return 0;
}


