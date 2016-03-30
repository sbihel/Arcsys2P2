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
#define PORT_NB "7777"
#define NB_VIEWERS 999
int viewers[NB_VIEWERS];


int init_server() {
  for(int i = 0; i < NB_VIEWERS; i++)
    viewers[i] = 0;

  /* Opening socket */
  int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sfd == -1) {
    perror("Socket");
    exit(1);
  }

  /* Binding */
  struct sockaddr_in s_sockaddr_in, client_addr;
  memset(&s_sockaddr_in, '0', sizeof (struct sockaddr_in));
  memset(&client_addr, '0', sizeof (struct sockaddr_in));
  s_sockaddr_in.sin_family = AF_INET;
  s_sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);
  uint16_t port = (uint16_t) atoi(PORT_NB);
  s_sockaddr_in.sin_port = htons(port);

  int b = bind(sfd, (const struct sockaddr*) &s_sockaddr_in, sizeof (struct
        sockaddr_in));
  if (b == -1) {
    perror("Binding");
    exit(1);
  }

  /* Listening */
  int l = listen(sfd, 10);
  if (l == -1) {
    perror("Listen");
    exit(1);
  }
  printf("server: waiting for connections...\n");

  /* Accept loop */
  int clientfd;
  socklen_t client_addr_len = sizeof(struct sockaddr_in);

  int i_viewer = 0;
  while(!viewers[NB_VIEWERS-1]) {
    struct timeval tv;
    fd_set readfds;
    tv.tv_sec = 2;
    tv.tv_usec = 500000;  // wait 2.5 seconds before timeout

    FD_ZERO(&readfds);
    FD_SET(sfd, &readfds);

    int res_select = select(sfd+1, &readfds, NULL, NULL, &tv);

    if(res_select) {
      clientfd = accept(sfd, (struct sockaddr *) &client_addr,
          &client_addr_len);

      if (clientfd == -1) {
        perror("Accept");
        exit(1);
      }

      char client_addr_str[INET_ADDRSTRLEN];
      if (!inet_ntop(AF_INET, &client_addr, client_addr_str, INET_ADDRSTRLEN)) {
        perror("Address");
        exit(1);
      }

      printf("Accepted %s\n", client_addr_str);
      viewers[i_viewer] = clientfd;

      i_viewer++;
    } else {
      printf("Initial wait is over, let's start.");
      break;
    }
  }

  return 0;
}

void close_server() {
  for(int i = 0; i < NB_VIEWERS; i++)
    close(viewers[i]);
  // TODO graceful shutdown
}


/**
 * Send a message with the board's size, the board and the symbols of both
 * players
 */
void init_viewers(char *board, int board_size, char current_player){
  // TODO remove char current_player argument, not used anymore
  char size_string[40];
  sprintf(size_string, "%d", board_size);
  char *message = malloc((board_size * board_size + 50) * sizeof(char));

  int j;
  j = 0;
  while(size_string[j] != '\0') {
    message[j] = size_string[j];
    j++;
  }
  message[j] = ' ';
  j++;
  for(int k = 0; k < board_size * board_size; k++) {
    message[j] = board[k];
    j++;
  }
  // TODO current_player is needed when the game has already started
  message[++j] = '\0';

  for(int i = 0; i < NB_VIEWERS; i++) {
    send(viewers[i], message, (board_size * board_size + 50) * sizeof(char), 0);
  }
  free(message);
}


void update_viewers(char *message, int size_message) {
  for(int i = 0; i < NB_VIEWERS; i++) {
    send(viewers[i], message, size_message, 0);
  }
}
