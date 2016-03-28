#ifndef SERVER_H
#define SERVER_H

int init_server();
void close_server();
void init_viewers(char *board, int board_size, char symbol_0, char symbol_1);
int update_viewers(char *message, int size_message);

#endif
