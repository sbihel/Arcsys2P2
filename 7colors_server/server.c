#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#define BUFF_SIZE 1024
#define PORT_NB "7777"
#define NB_VIEWERS 999

/* Contains all client and then send them to viewers or
 * potential_player according to message received
 */
int clients[NB_VIEWERS];
int current_nb_clients = 0;

int viewers[NB_VIEWERS];
int sfd;
int current_nb_viewers = 0;
int player_socket;
int potential_player;

#define MOVE_REQUEST "ceciestunerequetedemove"
#define PLAYER_REQUEST "ceciestunerequetedestrategiepourlejoueur"
#define PLAY_REQUEST "iwannaplaydude"
#define SPECTATE_REQUEST "iwannaspectateman"
#define SERVER_YES "forsure"
#define SERVER_NO "nosorrybro"


/**
 * Return true if there is a potential player.
 */
bool is_there_potential_player() {
  return potential_player != 0;
}


/**
 * Check messages to see if there is an incoming viewer or player.
 *
 * @param message: Pointer to buffer to use for recv. Legacy system.
 * @param message_size: Size of the buffer.
 */
int check_messages(char *message, int message_size) {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sfd, &readfds);
  int max_sfd = sfd;
  for(int i = 0; i < current_nb_clients; i++) {
    FD_SET(clients[i], &readfds);
    max_sfd = (max_sfd<clients[i])?clients[i]:max_sfd;
  }
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  printf("Waiting for messages...\n");
  if (select(max_sfd+1, &readfds, NULL, NULL, &tv) < 0) {
    perror("select");
    exit(1);
  }
  for(int i = 0; i < current_nb_clients; i++) {
    if(FD_ISSET(clients[i], &readfds)) {
      if(recv(clients[i], message, message_size, 0) == -1) {
        perror("recv");
        exit(2);
      }
      if(strncmp(message, PLAY_REQUEST, sizeof(PLAY_REQUEST)) == 0) {
        potential_player = clients[i];
      }
      if(strncmp(message, SPECTATE_REQUEST, sizeof(SPECTATE_REQUEST)) == 0) {
        viewers[current_nb_viewers] = clients[i];
        current_nb_viewers++;
      }
    }
  }
  return 0;
}


/**
 * Initialize the server.
 */
int init_server() {
  for(int i = 0; i < NB_VIEWERS; i++) {
    clients[i] = 0;
    viewers[i] = 0;
  }
  potential_player = 0;

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
    tv.tv_sec = 4;
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
      clients[current_nb_clients] = clientfd;
      current_nb_clients++;

      char *message = (char*) malloc(1024);
      check_messages(message, 1024);
      free(message);
    } else {
      printf("Initial wait is over, let's start.\n");
      break;
    }
  }

  return 0;
}


/**
 * Shutdown and close sockets.
 */
void close_server() {
  shutdown(sfd, 0);  // Further receives are disallowed
  for(int i = 0; i < NB_VIEWERS; i++)
    close(viewers[i]);
  close(sfd);
  // TODO graceful shutdown
}


/**
 * Give all the initial information to the distant player.
 *
 * Send a string composed of the size of the board, a space, and the board.
 *
 * @param board: Array of char representing a 2D square board.
 * @param board_size: Length of a side of the board.
 */
void init_player(char *board, int board_size) {
  printf("INIT_PLAYER\n");
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
  printf("sent: %s\n", message);
}


/**
 * Give all the initial information to a certain viewer.
 *
 * Send a string composed of the size of the board, a space, and the board.
 *
 * @param board: Array of char representing a 2D square board.
 * @param board_size: Length of a side of the board.
 * @param index_viewer: Index of the viewer in the array of viewers.
 */
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
 * Give all the initial information to all viewers.
 *
 * Send a string composed of the size of the board, a space, and the board.
 *
 * @param board: Array of char representing a 2D square board.
 * @param board_size: Length of a side of the board.
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

  for(int i = 0; i < current_nb_viewers; i++) {
    send(viewers[i], message, (board_size * board_size + 50) * sizeof(char), 0);
  }
  free(message);
}


/**
 * Remove a viewer of the viewers array.
 *
 * @param index_viewer: Index of the viewer in the viewers array.
 */
void remove_viewer(int index_viewer) {
  for(int i = index_viewer; i < current_nb_viewers; i++)
    viewers[i] = viewers[i + 1];
  current_nb_viewers--;
}


/**
 * Check if there is a new viewer that connected during a game and initialize
 * him.
 *
 * @param board: Array of char representing a square board game.
 * @param board_size: Length of a side of the board.
 */
void check_new_viewers(char *board, int board_size) {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sfd, &readfds);
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
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
    current_nb_viewers++;
  }
}


/**
 * Update viewers with the move that has just been played and check for new
 * viewers.
 *
 * @param message: String that should be sent to viewers as a notice of the
 * move.
 * @param message_size: Size of the message.
 * @param board: Array of char representing a square board game.
 * @param board_size: Length of a side of the board.
 */
void update_viewers(char *message, int size_message, char *board,
                    int board_size) {
  check_new_viewers(board, board_size);
  for(int i = 0; i < NB_VIEWERS; i++) {
    send(viewers[i], message, size_message, 0);
    // TODO remove the viewer if error
  }
}

/**
 * Update the distant player with the move that has just been played.
 *
 * @param message: String that should be sent to viewers as a notice of the
 * move.
 * @param message_size: Size of the message.
 */
void update_player(char *message, int size_message) {
  if(player_socket != 0) {
    send(player_socket, message, size_message, 0);
  }
}

/**
 * Accept the potential player as the distant player.
 */
void accept_player() {
  send(potential_player, SERVER_YES, sizeof(SERVER_YES), 0);
  player_socket = potential_player;
  printf("Accepted player %d\n", player_socket);
  /*viewers[current_nb_viewers] = player_socket;*/
  /*current_nb_viewers++;*/
}


/**
 * Don't accept the potential_player.
 */
void reject_player() {
  send(potential_player, SERVER_NO, sizeof(SERVER_NO), 0);
  potential_player = 0;
}


/**
 * Ask which strategy the distant player will use.
 */
char* ask_player_game_type() {
  printf("lel1");
  fflush(stdout);
  usleep(100);
  send(player_socket, &PLAYER_REQUEST, sizeof(PLAYER_REQUEST), 0);
  printf("lel2");
  fflush(stdout);
  char *response = (char *) malloc (3*sizeof(char));
  if(recv(player_socket, response, 3, 0) == -1) {
    perror("recv");
    exit(2);
  }
  printf("%c | %c | %c\n", response[0], response[1], response[2]);
  return response;
}


/*
 * Announce the distant player whether he plays first or second. '0' for first.
 *
 * @param first_player: Char that should be sent.
 */
void announce_first_player(char first_player) {
  usleep(100);
  char buff = first_player;
  send(player_socket, &buff, 1, 0);
  printf("sent: %c\n", buff);
}


/**
 * Get the move from the distant player, wait for 5 seconds.
 */
char ask_player_move() {
  printf("HELLLOOW\n");
  usleep(100);
  char response = '\0';
  int rc = 0;
  while (rc != 1) {
    /*if (send(player_socket, &MOVE_REQUEST, sizeof(MOVE_REQUEST), 0) == -1) {*/
    /*  perror("send");                                                       */
    /*  exit(2);                                                              */
    /*}                                                                       */
    printf("checkpoint1\n");
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(player_socket, &readfds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 10000;  // Wait a bit more to take account of sending time
    if (select(player_socket+1, &readfds, NULL, NULL, &tv) < 0) {
      perror("select");
      exit(1);
    }
    if(FD_ISSET(player_socket, &readfds)) {
      rc = recv(player_socket, &response, 1, 0);
      printf("checkpoint2 - %d\n", rc);
      if (rc == -1) {
        perror("recv");
        exit(2);
      }
    } else {
      // The player did not play in time
      break;
    }
  }
  printf("Da fuck is %c ??", response);
  fflush(stdout);
  return response;
}
