
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
#define SERVER_IP "127.0.0.1"

int sfd;

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
 * buff needs to be freed when not used anymore
 */
char* get_initial_board() {
  char* game_infos = (char*) malloc (BUFF_SIZE*sizeof(char));
  recv(sfd, game_infos, BUFF_SIZE, 0);
  return game_infos;
}

/** Receive and return the next move (char) to play in the same
 */
char get_next_move() {
  char c;
  recv(sfd, &c, 1, 0);
  return c;
}

/** Init game by parsing the game_infos received from server
 */
void init_game_server(char* game_infos) {
  
  /* Parsing game_infos */
  
  char size_string[40];
  int j = 0;
  while (game_infos[j] != ' ') {
    size_string[j] = game_infos[j];
    j++;
  }
  j++
  int board_size = atoi(size_string);
  
  char* board = (char*) malloc (board_size * board_size * sizeof(char));
  int i = j;
  for ( ; j < i + board_size * board_size; j++) {
    board[j-i] = game_infos[j];
  }
  
  char symbol_0 = game_infos[++j];
  j++;
  char symbol_1 = game_infos[++j];
  
  if (game_infos[+jj] != '\0') {
    printf("Error parsing infos from server\n");
    exit(1);
  }
  
  free(game_infos);
  
  return board;
}

void spectate() {
  init_client();
  char* game_infos = get_initial_board();
  char* board = init_game_server(game_infos);
  game_spectate(board);
  free(board);
}


