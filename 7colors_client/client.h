#ifndef CLIENT_H
#define CLIENT_H

int init_client();
char* get_initial_board();
char* get_next_move();

void send_next_move(char move);
char* send_game_type_client();
void send_play_request();

int i_am_first();

void spectate();
void play();

#endif
