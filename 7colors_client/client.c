
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "game.h"

#define BUFF_SIZE 1024
#define PORT_NB "7777"
#define SERVER_IP "127.0.0.1"

#define MAX_SERVER_MISS 5

#define MOVE_REQUEST "ceciestunerequetedemove"
#define PLAYER_REQUEST "ceciestunerequetedestrategiepourlejoueur"
#define PLAY_REQUEST "iwannaplaydude"
#define SERVER_YES "forsure"
#define SERVER_NO "nosorrybro"

int sfd;

/** Use send function, and try several times if not working
 * if not working after MAX_SERVER_MISS tries, error is returned,
 * meaning server is not responding
 */
void client_to_server(int sfd, void* buff, size_t buff_len, int flags) {
  int i = MAX_SERVER_MISS;
  while (i > 0) {
    if (send(sfd, buff, buff_len, flags) > 0) {
      break;
    } else {
      i--;
      sleep(0.1);
    }
  }
  if (i == 0) {
    printf("Sending to server not working\n");
    exit(1);
  }
}

/** Use recv function, and try several times if not working
 * if not working after MAX_SERVER_MISS tries, error is returned,
 * meaning server is not responding
 */
void server_to_client(int sfd, void* buff, size_t buff_len, int flags) {
  int i = MAX_SERVER_MISS;
  while (i > 0) {
    if (recv(sfd, buff, buff_len, flags) > 0) {
      break;
    } else {
      i--;
      sleep(0.1);
    }
  }
  if (i == 0) {
    printf("Receiving from server not working\n");
    exit(1);
  }
}

/** Open a client socket
 */
int init_client() {

  /* Opening socket */
  sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sfd == -1) {
    perror("Socket");
    exit(1);
  }

  /* Connecting */
  struct sockaddr_in serv_addr;
  memset(&serv_addr, '0', sizeof (struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
    /* Port */
  uint16_t port = (uint16_t) atoi(PORT_NB);
  serv_addr.sin_port = htons(port);
    /* Server address */
  if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) == -1) {
    perror("Inet_pton");
    exit(1);
  }

  int c = connect(sfd, (const struct sockaddr*) &serv_addr, sizeof (struct
        sockaddr_in));
  if (c == -1) {
    perror("Connect");
    exit(1);
  }

  return 0;
}

/** Receive the board description as given by init_viewers and return it
 * game_infos needs to be freed when not used anymore
 */
char* get_initial_board() {
  char* game_infos = (char*) malloc (BUFF_SIZE*sizeof(char));
  server_to_client(sfd, game_infos, BUFF_SIZE, 0);
  return game_infos;
}

/** Receive from server and return a char[2]
 * c[0] in the next move to play
 * c[1] is the player that plays it
 * c needs to be freed after use
 */
char* get_next_move() {
  char* c = (char*) malloc (2*sizeof(char));
  server_to_client(sfd, c, 2, 0);
  return c;
}


/** Receive messages from server, and when a move request is detected,
 * send the move
 * @param move : next move to play
 */
void send_next_move(char move) {
  char move_request[BUFF_SIZE];
  sprintf(move_request, MOVE_REQUEST);
  char* buffer = (char*) malloc (BUFF_SIZE*sizeof(char));
  sprintf(buffer, " ");
  do {
    server_to_client(sfd, buffer, BUFF_SIZE, 0);
  } while (!strcmp(buffer, move_request));
  buffer[0] = move;
  client_to_server(sfd, buffer, BUFF_SIZE, 0);
  free(buffer);
  }
  

/** Receive messages from server, and when a game_type request is detected,
 * send the infos, same format as in ask_game_type_client
 */
char* send_game_type_client() {
  char player_request[BUFF_SIZE];
  sprintf(player_request, PLAYER_REQUEST);
  char* buffer = (char*) malloc (BUFF_SIZE*sizeof(char));
  sprintf(buffer, " ");
  do {
    server_to_client(sfd, buffer, BUFF_SIZE, 0);
  } while (!strcmp(buffer, player_request));
  char* infos = ask_game_type_client();
  sprintf(buffer, infos);
  client_to_server(sfd, buffer, BUFF_SIZE, 0);
  free(buffer);
  return infos;
}


/** Send a play request to server
 * If accepted, server will ask for game_type and move then
 * If refused (because already two players for example), then exit
 */
void send_play_request() {
  char server_yes[BUFF_SIZE];
  char server_no[BUFF_SIZE];
  sprintf(server_yes, SERVER_YES);
  sprintf(server_no, SERVER_NO);
  char* buffer = (char*) malloc (BUFF_SIZE*sizeof(char));
  sprintf(buffer, PLAY_REQUEST);
  client_to_server(sfd, buffer, BUFF_SIZE, 0);
  do {
    server_to_client(sfd, buffer, BUFF_SIZE, 0);
    if (strcmp(buffer, server_yes)) {
      printf("Server accepted your play request\n");
      break;
    } else if (strcmp(buffer, server_no)) {
      printf("Server refused your play request\n");
      printf("Redirecting you to spectating\n");
      spectate();
    }
  } while (1);
  free(buffer);
}


/** Receive a message from server and return 1
 * if the player is first to play, 0 otherwise
 */
int i_am_first() {
  char c;
  server_to_client(sfd, &c, 1, 0);
  return c;
}


/** Spectating a game
 */
void spectate() {

  /* Getting infos from server */
  init_client();
  char* game_infos = get_initial_board();

  /* Parsing game_infos */
  char size_string[40];
  int j = 0;
  while (game_infos[j] != ' ') {
    size_string[j] = game_infos[j];
    j++;
  }
  j++;
  int board_size = atoi(size_string);

  char* board = (char*) malloc (board_size * board_size * sizeof(char));
  int i = j;
  for ( ; j < i + board_size * board_size; j++) {
    board[j-i] = game_infos[j];
  }

  if (game_infos[++j] != '\0') {
    printf("Error parsing infos from server\n");
    exit(1);
  }

  free(game_infos);

  /* Play the game from current state */
  game_spectate(board);

  free(board);
}


/** Sending a play request, and playing if accepted
 */
void play() {
  send_play_request();
  char* infos = send_game_type_client();
  char* board = get_initial_board();
  
  if (i_am_first()) { // first to play
    game_play(board, 0, infos);
  } else { // second to play
    game_play(board, 1, infos);
  }
  
}


