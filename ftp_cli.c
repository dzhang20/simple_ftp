#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

//$ ./ttweetcli <ServerIP> <ServerPort> <Path-to-file>
int main(int argc, char** argv){
  //checking inputs
  if(argc<4){
    fprintf(stderr,"Usage: $ ./ttweetcli <ServerIP> <ServerPort> <Path-to-file>");
    exit(1);
  }
  char* serverIP = argv[1];
  int port;
  sscanf(argv[2], "%d", &port);
  char* path = argv[3];

  int sock;
  struct sockaddr_in serv_addr;
  //create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
  if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0){
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }
  //set max sending buffer
  int cwin = 4086;
  printf("set maximum congestion window to %d\n",cwin);
  if((setsockopt(sock, SOL_SOCKET, SO_SNDBUF,&cwin, sizeof(cwin)))==-1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  //try connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    printf("\nConnection Failed \n");
    return -1;
  }


  //client start sending requests
  char buffer[1024];
  memset(buffer,0,1024);
  FILE* fp = fopen(path, "r");
  if(fp == NULL){
    fprintf(stderr, "filepath no valid");
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  size_t file_size = ftell(fp);
  rewind(fp);
  printf("sending file of size: %zu\n",file_size);
  size_t bytes_read=0;
  //total bytes read
  size_t total_read=0;
  struct timeval start, end;
  gettimeofday(&start,NULL);
  char ack [100];
  size_t d;
  //reading file content and send it to server
  while((bytes_read = fread(buffer,1024,1,fp))>0){
    send(sock, buffer, bytes_read, 0);
    total_read += bytes_read;
    printf("%zu packets sent\n", total_read);
    read(sock,ack,100);
    gettimeofday(&end,NULL);
    sscanf(ack, "%ld", &d);

    printf("received ack %ld at %ld usec\n",d, ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));

  }
  printf("Client finish sending file\n");
  close(sock);
  gettimeofday(&end,NULL);
  printf("server received all packets in %ld usec\n",((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
  return 0;
}
