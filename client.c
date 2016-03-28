
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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
  struct sockaddr_in serv_addr;
  memset(&serv_addr, '0', sizeof (struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
    /* Port */
  uint16_t port = (uint16_t) atoi(argv[2]);
  serv_addr.sin_port = htons(port);
    /* Server address */
  if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) == -1) {
    perror("Inet_pton");
    exit(1);
  }

  int c = connect(sfd, (const struct sockaddr*) &serv_addr, sizeof (struct
        sockaddr_in));
  if (c == -1) {
    perror("Connect");
    exit(1);
  }

  char* buff = (char*) malloc (BUFF_SIZE*sizeof(char));

  strcpy(buff, argv[3]);
  printf("%s\n", buff);
  send(sfd, buff, BUFF_SIZE, 0);
  recv(sfd, buff, BUFF_SIZE, 0);

  printf("%s", buff);

  free(buff);

  return 0;
}
