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
int sfd;
int current_nb_viewers = 0;
int player_socket;
int potential_player;

#define MOVE_REQUEST "ceciestunerequetedemove"
#define PLAYER_REQUEST "ceciestunerequetedestrategiepourlejoueur"
#define PLAY_REQUEST "iwannaplaydude"
#define SERVER_YES "forsure"
#define SERVER_NO "nosorrybro"


int init_server() {
  for(int i = 0; i < NB_VIEWERS; i++)
    viewers[i] = 0;

  /* Opening socket */
  sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

  while(!viewers[NB_VIEWERS-1]) {
    struct timeval tv;
    fd_set readfds;
    tv.tv_sec = 2;
    tv.tv_usec = 500000;  // wait 2.5 seconds before timeout

    FD_ZERO(&readfds);
    FD_SET(sfd, &readfds);

    // Using select just to have a timeout
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
      viewers[current_nb_viewers] = clientfd;

      current_nb_viewers++;
    } else {
      printf("Initial wait is over, let's start.\n");
      break;
    }
  }

  return 0;
}

void close_server() {
  shutdown(sfd, 0);  // Further receives are disallowed
  for(int i = 0; i < NB_VIEWERS; i++)
    close(viewers[i]);
  close(sfd);
  // TODO graceful shutdown
}


void init_player(char *board, int board_size) {
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

  message[++j] = '\0';

  send(player_socket, message, (board_size * board_size + 50) *
      sizeof(char), 0);
  free(message);
}

void init_viewer(char *board, int board_size, int index_viewer) {
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

  message[++j] = '\0';

  send(viewers[index_viewer], message, (board_size * board_size + 50) *
      sizeof(char), 0);
  free(message);
}


/**
 * Send a message with the board's size and the board
 */
void init_viewers(char *board, int board_size) {
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

  message[++j] = '\0';

  for(int i = 0; i < NB_VIEWERS; i++) {
    send(viewers[i], message, (board_size * board_size + 50) * sizeof(char), 0);
  }
  free(message);
}

void remove_viewer(int index_viewer) {
  for(int i = index_viewer; i < current_nb_viewers; i++)
    viewers[i] = viewers[i + 1];
  current_nb_viewers--;
}

void check_new_viewers(char *board, int board_size) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sfd, &readfds);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;  // wait 2.5 seconds before timeout
    if (select(sfd+1, &readfds, NULL, NULL, &tv) < 0) {
      perror("select");
      exit(1);
    }
    if(FD_ISSET(sfd, &readfds)) {
      struct sockaddr_in client_addr;
      memset(&client_addr, '0', sizeof (struct sockaddr_in));
      int clientfd;
      socklen_t client_addr_len = sizeof(struct sockaddr_in);
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

      viewers[current_nb_viewers] = clientfd;
      init_viewer(board, board_size, current_nb_viewers);
      // TODO init viewers
      current_nb_viewers++;
    }
}

void update_viewers(char *message, int size_message, char *board,
    int board_size) {
  check_new_viewers(board, board_size);
  for(int i = 0; i < NB_VIEWERS; i++) {
    send(viewers[i], message, size_message, 0);
    // TODO remove the viewer if error
  }
}

void update_viewers_but_not_player(char *message, int size_message, char *board,
    int board_size) {
  check_new_viewers(board, board_size);
  for(int i = 0; i < NB_VIEWERS; i++) {
    if(viewers[i] != player_socket) {
    send(viewers[i], message, size_message, 0);
    // TODO remove the viewer if error
    }
  }
}

int check_messages(char *message, int message_size) {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sfd, &readfds);
  struct timeval tv;
  tv.tv_sec = 4;
  tv.tv_usec = 0;
  if (select(sfd+1, &readfds, NULL, NULL, &tv) < 0) {
    perror("select");
    exit(1);
  }
  for(int i = 0; i < NB_VIEWERS; i++) {
    if(FD_ISSET(viewers[i], &readfds)) {
      recv(viewers[i], message, message_size, 0);
      if(strcmp(message, PLAY_REQUEST)) {
        potential_player = viewers[i];
        return 1;
      }
    }
  }
  return 0;
}

void accept_player() {
  send(potential_player, SERVER_YES, sizeof(SERVER_YES), 0);
  player_socket = potential_player;
  viewers[current_nb_viewers] = player_socket;
  current_nb_viewers++;
}

void reject_player() {
  send(potential_player, SERVER_NO, sizeof(SERVER_NO), 0);
  potential_player = 0;
}

char* ask_player_game_type() {
  send(player_socket, MOVE_REQUEST, sizeof(MOVE_REQUEST), 0);
  char *response = "010";
  recv(potential_player, response, 3, 0);
  return response;
}

// first_player = '1' if distant player plays firt
void announce_first_player(char firt_player) {
  send(player_socket, &firt_player, 1, 0);
}

char ask_player_move() {
  send(player_socket, MOVE_REQUEST, sizeof(MOVE_REQUEST), 0);
  char response;
  recv(player_socket, &response, 1, 0);
  return response;
}
