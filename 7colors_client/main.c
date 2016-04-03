#include "7colors.h"
#include <stdio.h>
#include "game.h"
#include <stdlib.h>
#include "client.h"


/** Program entry point */
int main(int argc, char *argv[])
{
  // RNG initialization
  srand(time(0));

  // Welcome message
  printf("\n\n  Welcome to the 7 wonders of the world of the 7 colors\n"
      "  *****************************************************\n\n");

  int f;
  while((f = getopt(argc, argv, "sp")) != -1) {
    switch(f) {
      case 's':
        spectate();
        break;
      case 'p':
        play();
        break;
      default:
        printf("Usage: -p to play and -s to watch.\n");
        exit(2);
        break;
    }
  }

  if(argc == 1) {
    printf("Usage: -p to play and -s to watch.\n");
    exit(2);
  }

  return 0; // Everything went well
}
