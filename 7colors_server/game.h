#ifndef HAVE_GAME_H
#define HAVE_GAME_H

#include <stdbool.h>
#include "7colors.h"
#include "ai.h"
#include <unistd.h>

bool is_game_finished(int*);
char ask(int);
void init_game();
unsigned int ask_tournament();
void ask_game_type(char* game_types, int* depths, char player_id);
char game(char* board, int* depths, char* game_types);
void tournament(char* game_types, int* depths, int nb_games);

#endif
