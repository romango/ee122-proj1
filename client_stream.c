/* Client Stream */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define IP "67.188.126.64"
#define PORT "4444"
#define OUTPATH = "output.png"
//#define MAXDATASIZE 5000000

int recieve_data(int sockfd, char* buf, size_t len) {
  int numbytes;
  numbytes = recv(sockfd, buf, len, 0);
  if (numbytes == -1) {
    perror("recv");
    exit(1);
  }
  if (numbytes != 0) {
    printf("Recieved %d bytes.\n", numbytes);
  }
  return numbytes;
}





int main(int argc, char* argv[])
{
  struct addrinfo hints, *servinfo, *p;
  char s[INET_ADDRSTRLEN];
  int sockfd, numbytes;
  //char buf[MAXDATASIZE];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int rv = getaddrinfo(IP, PORT, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }


  for(p = servinfo; p != NULL; p = p->ai_next) {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      perror("client: socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }


  inet_ntop(p->ai_family, &(((struct sockaddr_in*)p->ai_addr)->sin_addr), 
    s, sizeof s);
  printf("client: connecting to %s\n", s);
  freeaddrinfo(servinfo); // all done with this structure

  uint32_t datasize;
  char sizechar[4];
  char* data;

  recieve_data(sockfd, sizechar, 4);
  memcpy(&datasize, sizechar, 4);
  datasize = ntohl(datasize);

  printf("Expected Data Size: %d\n", datasize);
  data = (char*) malloc(datasize);
  int recvsofar = 0;
  while (recvsofar < datasize) {
    numbytes = recieve_data(sockfd, data + recvsofar, datasize-recvsofar);
    recvsofar += numbytes;
    if (numbytes != 0) {
      printf("Recieved total %d bytes.\n", recvsofar);
    }
  }
  FILE *fp = fopen( "output.png" , "wb");
  fwrite(data, sizeof(char), datasize, fp);

//  numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
//  if (numbytes == -1) {
//    perror("recv");
//    exit(1);
//  }
//  buf[numbytes] = '\0';
//  printf("client: received '%s'\n",buf);
//  printf("Recieved %d bytes.\n", numbytes);
  close(sockfd);
  free(data);
  return 0;
}
