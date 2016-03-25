#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 1024


int main(int argc, char** argv) {
  
  /* Checking number of arguments */
   if (argc != 2) {
    printf("Invalid argument\n");
    exit(1);
  }
  
  /* Opening socket */
  int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sfd == -1) {
    perror("Socket");
    exit(1);
  }
  
  /* Binding */
  struct sockaddr_in s_sockaddr_in, client_addr;
  s_sockaddr_in.sin_family = AF_INET;
  s_sockaddr_in.sin_addr.s_addr = INADDR_ANY;
  uint16_t port = (uint16_t) atoi(argv[1]);
  s_sockaddr_in.sin_port = htonl(port);
  
  int b = bind(sfd, (const struct sockaddr*) &s_sockaddr_in, sizeof (struct sockaddr_in));
  if (b == -1) {
    perror("Binding");
    exit(1);
  } 
  
  /* Listening */
  int l = listen(sfd, 128);
  if (l == -1) {
    perror("Listen");
    exit(1);
  } 
  printf("server: waiting for connections...\n");
  
  /* Accept loop */
  struct in_addr addr;
  void* buf = (void*) malloc (BUFF_SIZE*sizeof(char));
  ssize_t recv_out;
  
  while (1) {
    
    printf("waiting\n");
    
    int clientfd;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);
    clientfd = accept(sfd, (struct sockaddr *) &client_addr, &client_addr_len);
    
    if (clientfd == -1) {
      perror("Accept");
      exit(1);
    }
    
    addr = client_addr.sin_addr;
    if (inet_aton(argv[1], &addr) == 0) {
      fprintf(stderr, "Invalid address\n");
      exit(EXIT_FAILURE);
    }

    printf("%s\n", inet_ntoa(addr));
    
    recv_out = recv(sfd, buf, BUFF_SIZE, 0);
    recv_out = send(sfd, buf, BUFF_SIZE, 0);
    
    close(clientfd);
  }
  
  free(buf);
  
  return 0;
}
