#ifndef HAVE_7COLORS_H
#define HAVE_7COLORS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>


/* We want a 30x30 board game by default */
#define BOARD_SIZE 30
#define SYMBOL_0 '.'
#define SYMBOL_1 '`'


typedef char color;
char get_cell(char* board, int x, int y);
void set_cell(char* board, int x, int y, char color);
void print_board(char* board);
void fill_board(char* board);
void symmetric_fill_board(char* board);
bool is_adjacent(char* board, int i, int j, char player);
int update_board(char* board, char player, char color);
unsigned int board_checksum(char* board);

#endif
