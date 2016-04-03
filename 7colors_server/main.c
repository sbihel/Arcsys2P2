#include "7colors.h"
#include <stdio.h>
#include "game.h"
#include <stdlib.h>
#include "server.h"
#include <signal.h>

#define UNUSED(x) (void)(x)

void int_handler(int sig) {
  UNUSED(sig);  // Suppress warning about sig not being used.
  close_server();
  exit(0);
}

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
  signal(SIGINT, int_handler);
  init_game();
  close_server();

  return 0; // Everything went well
}
