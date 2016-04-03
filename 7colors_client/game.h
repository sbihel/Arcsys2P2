#ifndef HAVE_GAME_H
#define HAVE_GAME_H

#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include "7colors.h"
#include "ai.h"
#include <unistd.h>
#include "client.h"
#include <sys/mman.h>

bool is_game_finished(int*);

char ask(int);
void ask_game_type(char* game_types, int* depths, char player_id);
char* ask_game_type_client();

char game_spectate(char* board);
char game_play(char* board, int order, char* infos);

#endif
