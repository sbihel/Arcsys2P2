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

/* BUFF_SIZE is size of char* used to get message from server
 * Must be longer than size of *_REQUEST and SERVER_*
 */
#define BUFF_SIZE 1024
#define PORT_NB "7777"
#define SERVER_IP "127.0.0.1"

/* Exiting if server is not responding
 * after 2^(MAX_SERVER_MISS) - 1 seconds */
#define MAX_SERVER_MISS 3

#define MOVE_REQUEST "ceciestunerequetedemove"
#define PLAYER_REQUEST "ceciestunerequetedestrategiepourlejoueur"
#define PLAY_REQUEST "iwannaplaydude"
#define SPECTATE_REQUEST "iwannaspectateman"
#define SERVER_YES "forsure"
#define SERVER_NO "nosorrybro"

int sfd;

/** Use send function, and try several times if not working
 * if not working after MAX_SERVER_MISS tries, error is returned,
 * meaning server is not responding
 */
void client_to_server(int sfd, void* buff, size_t buff_len, int flags) {
  int i = MAX_SERVER_MISS;
  int wait_time = 1;
  while (i > 0) {
    if (send(sfd, buff, buff_len, flags) > 0) {
      /*printf("Sent: %s\n", buff);*/
      break;
    } else {
      i--;
      sleep(wait_time);
      wait_time *= 2;
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
  int wait_time = 1;
  int rc;
  while (i > 0) {
    if ((rc = recv(sfd, buff, buff_len, flags)) > 0) {
      /*printf("Received: %s - %d\n", buff, rc);*/
      break;
    } else {
      i--;
      sleep(wait_time);
      wait_time *= 2;
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
  char* buffer = (char*) malloc (BUFF_SIZE*sizeof(char));
  /*do {                                                               */
  /*  server_to_client(sfd, buffer, BUFF_SIZE, 0);                     */
  /*} while (strncmp(buffer, MOVE_REQUEST, sizeof(MOVE_REQUEST)) != 0);*/
  client_to_server(sfd, &move, 1, 0);
  free(buffer);
}


/** Receive messages from server, and when a game_type request is detected,
 * send the infos, same format as in ask_game_type_client
 */
char* send_game_type_client() {
  char* buffer = (char*) malloc (BUFF_SIZE*sizeof(char));
  do {
    server_to_client(sfd, buffer, BUFF_SIZE, 0);
  } while (strncmp(buffer, PLAYER_REQUEST, sizeof(PLAYER_REQUEST)) != 0);
  char* infos = ask_game_type_client();
  client_to_server(sfd, infos, 3, 0);
  free(buffer);
  return infos;
}


/** Send a play request to server
 * If accepted, server will ask for game_type and move then
 * If refused (because already two players for example), then exit
 */
void send_play_request() {
  char* buffer = (char*) malloc (BUFF_SIZE*sizeof(char));
  client_to_server(sfd, &PLAY_REQUEST, sizeof(PLAY_REQUEST), 0);
  do {
    server_to_client(sfd, buffer, BUFF_SIZE, 0);
    if (strncmp(buffer, SERVER_YES, sizeof(SERVER_YES)) == 0) {
      printf("Server accepted your play request\n");
      break;
    } else if (strncmp(buffer, SERVER_NO, sizeof(SERVER_NO)) == 0) {
      printf("Server refused your play request\n");
      printf("Redirecting you to spectating\n");
      spectate();
    }
  } while (1);
  free(buffer);
}


/** Send a spectate request to server
 */
void send_spectate_request() {
  char* buffer = (char*) malloc (BUFF_SIZE*sizeof(char));
  sprintf(buffer, SPECTATE_REQUEST);
  client_to_server(sfd, buffer, BUFF_SIZE, 0);
  free(buffer);
}


/** Receive a message from server and return 1
 * if the player is first to play, 0 otherwise
 */
int am_i_first() {
  char buff = 'a';
  server_to_client(sfd, &buff, 1, 0);
  return atoi(&buff);
}


/** Spectating a game
 */
void spectate() {

  /* Getting infos from server */
  init_client();
  send_spectate_request();

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
  init_client();
  sleep(1);
  send_play_request();
  char* infos = send_game_type_client();
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

  int first = am_i_first();
  if (!first) { // first to play
    game_play(board, 0, infos);
  } else { // second to play
    game_play(board, 1, infos);
  }

  free(board);
}
