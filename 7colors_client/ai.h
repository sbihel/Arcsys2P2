#ifndef AI_H
#define AI_H

#include "7colors.h"
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

char rand_play();
char rand_valid_play(char* board, char player);
char biggest_move(char* board, char player);
char minimax(char *board, char player);
char minimax_with_depth(char *board, char player, int depth);
int availArea(char* board, char player, int i, int j, int propI, int propJ);
char hegemon(char* board, char player, int startI, int startJ, 
             int propI, int propJ);
char alphabeta(char *board, char player);
char alphabeta_with_depth(char *board, char player, int depth);
char alphabeta_with_expand_perimeter(char *board, char player);
char alphabeta_with_expand_perimeter_depth(char *board, char player, int depth);

#endif
