#ifndef CLIENT_H
#define CLIENT_H

int init_client();
char* get_initial_board();
char* get_next_move();
void spectate();
void send_next_move(char move);
void send_game_type_client();
void send_play_request();

#endif
