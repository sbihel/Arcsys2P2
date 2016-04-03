#ifndef SERVER_H
#define SERVER_H

int init_server();
void close_server();
void init_player(char *board, int board_size);
void init_viewers(char *board, int board_size);
void update_viewers(char *message, int size_message, char *board,
        int board_size);
void update_player(char *message, int size_message);
int check_messages(char *message, int message_size);
void accept_player();
void reject_player();
char* ask_player_game_type();
void announce_first_player(char firt_player);
char ask_player_move();
bool is_there_potential_player();

#endif
