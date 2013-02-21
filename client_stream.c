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
#include <sys/stat.h>
#include <time.h>
#define IP "67.188.126.64"
#define PORT "4444"
#define OUTPATH "output.jpg"

long timevaldiff(struct timeval *starttime, struct timeval *finishtime)
{
  long msec;
  msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
  msec+=(finishtime->tv_usec-starttime->tv_usec)/1000;
  return msec;
}


int recieve_data(int sockfd, char* buf, size_t len) {
  int numbytes;
  numbytes = recv(sockfd, buf, len, MSG_WAITALL);
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
  struct timeval tv1, tv2;

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
  data = (char*) calloc(datasize , sizeof(char));
  


  //t = clock();
  gettimeofday(&tv1, NULL);
  recieve_data(sockfd, data, datasize);
  //t = clock() - t;
  gettimeofday(&tv2, NULL);

  printf("Time to recieve data: %f sec.\n", ((float) timevaldiff(&tv1, &tv2))/1000);


  FILE *fp = fopen(OUTPATH , "wb");
  fwrite(data, datasize,1, fp);
  fclose(fp);

  close(sockfd);
  free(data);
  return 0;
}
