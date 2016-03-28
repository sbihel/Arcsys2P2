#include "7colors.h"
#include <stdio.h>
#include "game.h"
#include <stdlib.h>
#include "server.h"


/** Program entry point */
int main()
{
  // RNG initialization
  srand(time(0));

  // Welcome message
  printf("\n\n  Welcome to the 7 wonders of the world of the 7 colors\n"
      "  *****************************************************\n\n");

  // Play the game !
  init_server();
  init_game();

  return 0; // Everything went well
}
