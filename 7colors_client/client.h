#ifndef CLIENT_H
#define CLIENT_H

int init_client();
char* get_initial_board();
char get_next_move();
char* init_game_server(char* game_infos);
void spectate();

#endif
