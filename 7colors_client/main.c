#include "7colors.h"
#include <stdio.h>
#include "game.h"
#include <stdlib.h>
#include "client.h"


/** Program entry point */
int main()
{
  // RNG initialization
  srand(time(0));

  // Welcome message
  printf("\n\n  Welcome to the 7 wonders of the world of the 7 colors\n"
      "  *****************************************************\n\n");

  // Spectate
  //spectate();
  play();

  return 0; // Everything went well
}
