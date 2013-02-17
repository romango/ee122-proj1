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


  numbytes = sendto(sockfd, content, info.st_size, 0,p->ai_addr, p->ai_addrlen);





//  if ((numbytes = sendto(sockfd, argv[1], strlen(argv[1]), 0,
//    p->ai_addr, p->ai_addrlen)) == -1) {
  if (numbytes == -1) {
    perror("talker: sendto");
    exit(1);
  }
  freeaddrinfo(servinfo);
  printf("talker: sent %d bytes to %s\n", numbytes, IP);
  close(sockfd);
  return 0;
}

