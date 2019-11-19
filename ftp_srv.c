#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
//#include <sys/epoll.h>
#include <sys/time.h>

//user struct to keep necessary client info
typedef struct User{
  int sock_id;
  struct timeval start;
  /*could add more attributes below*/

} user_t;

void* process_request(void* input);

//modify this function to measure performance
void* process_request(void* input){
  user_t *user  = (user_t*) input;
  //start time of client request
  struct timeval start = user->start;
  //Server side : Receiver window set to 1024
  char buffer [1350];
  memset(buffer,0,1350);
  size_t bytes_read = 0;
  size_t total_read = 0;
  //current time
  struct timeval cur;
  while((bytes_read = read(user->sock_id,buffer, 1024))>0){
    total_read +=bytes_read;
    gettimeofday(&cur,NULL);
    //print instant throughput
    printf("%zu packets received at %ld usec\n", total_read, ((cur.tv_sec * 1000000 + cur.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
    memset(buffer,0,1024);
    //send ack
    memcpy(buffer,&total_read,sizeof(total_read));
    //send(user->sock_id,buffer,sizeof(total_read),0);
    memset(buffer,0,1350);
    printf("sending ACK %zu\n",total_read);
  }
  gettimeofday(&cur,NULL);
  printf("all received at %ld usec\n",((cur.tv_sec * 1000000 + cur.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
  close(user->sock_id);
  return NULL;
}

// $./ttweetsrv <PORT>
int main(int argc, char** argv){
  //checking inputs
  if(argc<2){
    fprintf(stderr, "Usage: $./ttweetsrv <Port>");
    exit(1);
  }
  char* port = argv[1];
  int server_fd;
  int opt = 1;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(struct addrinfo));
  //set address info
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

    //get addr info for current machine
  int s;
  if((s=getaddrinfo(NULL, port, &hints, &res))<0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(1);
  }
    // Creating socket file descriptor
  if ((server_fd =  socket(AF_INET, SOCK_STREAM, 0)) == 0){
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  //set socket option
  //set max receiver buffer
  int rwin = 1350;
  printf("set receiving window to %d\n",rwin);
  if((setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF,&rwin, sizeof(rwin)))==-1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  //Enable quickack mode
//  if((setsockopt(server_fd, IPPROTO_TCP, TCP_QUICKACK,&opt, sizeof(opt)))==-1){
//    perror("setsockopt");
//    exit(EXIT_FAILURE);
//  }

  if((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,&opt, sizeof(opt)))==-1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  //set congestion control algorithm
//  char* congestion_control = "reno";
//  if((setsockopt(server_fd, IPPROTO_TCP, TCP_CONGESTION ,&congestion_control, sizeof(congestion_control)))==-1){
//    perror("setsockopt");
//    exit(EXIT_FAILURE);
//  }
  //bind socket to address
  if (bind(server_fd, res->ai_addr, res->ai_addrlen)<0){
    perror("bind failed");
    exit(1);
  }
  //listen for connection
  if (listen(server_fd, 3) < 0){
    perror("listen");
    exit(EXIT_FAILURE);
  }
  //start of the server
  while(1){
    //waiting for user connection
    int client_fd = accept(server_fd, NULL, NULL);
    printf("accepting %d\n", client_fd);
    //if have connection
    if(client_fd>0){
      //receiving header info
      struct timeval t1;
      user_t *user = malloc(sizeof(user_t));
      user-> sock_id = client_fd;
      gettimeofday(&t1,NULL);
      user->start = t1;
      pthread_t tid;
      //create thread for each client connection
      pthread_create(&tid,NULL, process_request,(void*)user);
    }
  }
	return 0;
}
