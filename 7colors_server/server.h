#ifndef SERVER_H
#define SERVER_H

int init_server();
void close_server();
void init_viewers(char *board, int board_size, char current_player);
int update_viewers(char *message, int size_message);

#endif
