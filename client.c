
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
  
  /* Checking arguments */
  if (argc != 4) {
    printf("Invalid argument\n");
    exit(1);
  }
  
  /* Opening socket */
  int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sfd == -1) {
    perror("Socket");
    exit(1);
  }
  
  /* Connecting */
  struct sockaddr_in c_sockaddr_in;
  c_sockaddr_in.sin_family = AF_INET;
  struct in_addr s_addr;
  if (inet_aton(argv[1], &s_addr) == 0) {
    fprintf(stderr, "Invalid address\n");
    exit(EXIT_FAILURE);
  }
  
  c_sockaddr_in.sin_addr = s_addr;
  uint16_t port = (uint16_t) atoi(argv[2]); 

  int c = connect(sfd, (const struct sockaddr*) &c_sockaddr_in, sizeof (struct sockaddr_in));
  if (c == -1) {
    perror("Connect");
    exit(1);
  }
  
  void* buf = (void*) malloc (BUFF_SIZE*sizeof(char));
  ssize_t recv_out;
  
  strcpy(buf, argv[3]);
  recv_out = send(sfd, buf, BUFF_SIZE, 0);
  recv_out = recv(sfd, buf, BUFF_SIZE, 0);
  
  printf("%s", (char*) buf);
  
  free(buf);
  
  return 0;
}
