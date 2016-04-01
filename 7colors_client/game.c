#include "game.h"

/** Determine whether game is finished, i.e. a player has more than half of
 * the available cells.
 * @param nb_cells An array containing how many cells each player has. This
 * array has to have 2 elements.
 */
bool is_game_finished(int nb_cells[])
{
  return (nb_cells[0] >= BOARD_SIZE * BOARD_SIZE / 2 || nb_cells[1] >=
      BOARD_SIZE * BOARD_SIZE / 2);
}


/** Ask the user for a game type that will apply to a player.
 * @param game_types An array containing the game types for each player. This
 * function fills one element of this array.
 * @param depths An array containing the depths to which alphabeta/minimax will
 * search, if applicable. One element of this array may be set.
 * @param player_id The identifier for the current player.
 */
void ask_game_type(char* game_types, int* depths, char player_id)
{
    printf("How will player %d act ?\n"
    "[1] Human input | [2] AlphaBeta AI | [3] Minimax AI | [4] Hegemonic AI\n"
    "[5] AlphaBeta with hegemonic heuristic | [6] Greedy | [7] Random \n"
    ">", player_id);
    char ans;
    while((ans = getchar()) == '\n' || ans > '7' || ans < '1')
        printf(">");

    game_types[(int)player_id] = ans;

    getchar(); // get rid of the '\n'

    if(ans == '2' || ans == '3' || ans == '5') {
      int depth;
      printf("How deep will the search go in the tree ?\n>");
      scanf("%d", &depth);
      depths[(int)player_id] = depth;
      getchar();
    }
}

/** Ask the player to pick a color. */
char ask(int curPlayer)
{
  printf("It's player %d's turn. Which color will they choose ?"
    " \033[K\n\033[K>",  curPlayer);
  char nextColor = getchar();
  while(nextColor == '\n') {  // Player has hit enter instead of a color
    printf("\033[FIt's player %d's turn. Which color will they choose ? "
        "\033[K\n\033[K>", curPlayer);
    nextColor = getchar();
  }
  getchar();
  while(nextColor > 'G' || nextColor < 'A') {
    printf("\033[F\033[K");  // clear previous line
    printf("It's player %d's turn. Which color will they choose ?\n\033[K>",
        curPlayer);
    nextColor = getchar();
    getchar();
  }
  return nextColor;
}

/** Ask game type to player_client and return infos of char* type
 * infos[0] is len of infos
 * infos[1] is strategy
 * infos[2] is depth if len >= 3
 * infos must be freed after use
 */
char* ask_game_type_client()
{
  printf("How will you act ?\n"
  "[1] Human input | [2] AlphaBeta AI | [3] Minimax AI | [4] Hegemonic AI\n"
  "[5] AlphaBeta with hegemonic heuristic | [6] Greedy | [7] Random \n"
  ">");
  char ans;
  while((ans = getchar()) == '\n' || ans > '7' || ans < '1')
      printf(">");
  getchar(); // get rid of the '\n'
  char* infos;
  if (ans == '2' || ans == '3' || ans == '5') {
    infos = (char*) malloc (3 * sizeof(char));
    char depth_;
    printf("How deep will the search go in the tree ?\n>");
    scanf("%c", &depth_);
    getchar();
    infos[2] = depth_;
    infos[0] = 3;
  } else {
    infos = (char*) malloc (2 * sizeof(char));
    infos[0] = 2;
  }
  infos[1] = ans;
  return infos;
}



/** The main game function, that runs a single game.
 * @param board An initialized board, as given by the server
 * received spectating request
 */
char game_spectate(char* board)
{

  /* Initialize scores according to the current state of the game */
  bool isFinished = false;
  int* scores = get_current_score(board);
  int nb_cells[2] = {scores[0], scores[1]};
  free(scores);

  printf("\033[2J");  // clear screen
  print_board(board);
  printf("| P0: %.2f%% | P1: %.2f%% |\n\n",
      (double) 100.0 * nb_cells[0] / (BOARD_SIZE * BOARD_SIZE),
      (double) 100.0 * nb_cells[1] / (BOARD_SIZE * BOARD_SIZE));

  while(!isFinished)
  {
    char* c = get_next_move();
    char nextColor = 'A';
    nextColor = c[0];
    nb_cells[(int) c[1]] += update_board(board,
        (c[1])?SYMBOL_1:SYMBOL_0, nextColor);
    print_board(board);

    printf("| P0: %.2f%% | P1: %.2f%% |\n\n",
        (double) 100.0 * nb_cells[0] / (BOARD_SIZE * BOARD_SIZE),
        (double) 100.0 * nb_cells[1] / (BOARD_SIZE * BOARD_SIZE));
    if(is_game_finished(nb_cells)) {
      printf("\033[KPlayer %d won with an occupation rate of %.2f%%\n",
          c[1], (double) 100.0 * nb_cells[(int) c[1]] / (BOARD_SIZE
            * BOARD_SIZE));
      break;

    free(c);
    }

  }
  return (char) 0;
}


/** The main game function, that runs a single game.
 * @param board An initialized board, as given by the server
 * @param order 0 if first to play, 1 otherwise
 * @parem infos about this client player, same format as in send_game_type_client
 */
char game_play(char* board, int order, char* infos)
{
  char myId = (char) order;
  char curPlayer = 0;

  /* Initialize scores according to the current state of the game */
  bool isFinished = false;
  int nb_cells[2] = {1, 1};

  printf("\033[2J");  // clear screen
  print_board(board);
  printf("| P0: %.2f%% | P1: %.2f%% |\n\n",
      (double) 100.0 * nb_cells[0] / (BOARD_SIZE * BOARD_SIZE),
      (double) 100.0 * nb_cells[1] / (BOARD_SIZE * BOARD_SIZE));

  while(!isFinished)
  {
    char nextColor = 'A';

    if (curPlayer != myId) { // other player turn
      nextColor = get_next_move()[0];
    } else { // our turn

      switch(infos[1])
      {
        case '1': // human v. human
          nextColor = ask(curPlayer);
          break;
        case '2': // alphabeta
          nextColor = alphabeta_with_depth(board, (curPlayer)?SYMBOL_1:SYMBOL_0,
                                           infos[2]);
          printf("\033[H\033[KAI %d (alphabeta) played %c\n", curPlayer,
                  nextColor);
          break;
        case '3': // minimax
          nextColor = minimax_with_depth(board, (curPlayer)?SYMBOL_1:SYMBOL_0,
                                         infos[2]);
          printf("\033[H\033[KAI %d (minimax) played %c\n", curPlayer,
                  nextColor);
          break;
        case '4': // hegemonic
          nextColor = hegemon(board, (curPlayer)?SYMBOL_1:SYMBOL_0,
                      (curPlayer)?BOARD_SIZE-1:0,
                      (curPlayer)?0:BOARD_SIZE-1,
                      (curPlayer)?-1:1, (curPlayer)?1:-1);
          printf("\033[H\033[KAI %d (hegemonic) played %c\n", curPlayer,
                  nextColor);
          break;
        case '5':
          nextColor = alphabeta_with_expand_perimeter_depth(board,
                                                  (curPlayer)?SYMBOL_1:SYMBOL_0,
                                                  infos[1]);
          printf("\033[H\033[KAI %d (Alphabeta hegemonic) played %c\n",
              curPlayer, nextColor);
          break;
        case '6':
          nextColor = biggest_move(board, (curPlayer)?SYMBOL_1:SYMBOL_0);
          printf("\033[H\033[KAI %d (Greedy) played %c\n", curPlayer,
                  nextColor);
          break;
        case '7':
          nextColor = rand_valid_play(board, (curPlayer)?SYMBOL_1:SYMBOL_0);
          printf("\033[H\033[KAI %d (Greedy) played %c\n", curPlayer,
                  nextColor);
          break;

        default:
          break;
    }
    send_next_move(nextColor);
  }

    if(nextColor < 'A' || nextColor > 'G') {
      // this is typically a 0x00 returned by alphabeta/minimaxi
      // let's determine the first available move; if none, then move 'A'.
      nextColor = rand_valid_play(board, (curPlayer)?SYMBOL_1:SYMBOL_0);
    }
    nb_cells[(int) curPlayer] += update_board(board,
        (curPlayer)?SYMBOL_1:SYMBOL_0, nextColor);
    print_board(board);

    printf("| P0: %.2f%% | P1: %.2f%% |\n\n",
        (double) 100.0 * nb_cells[0] / (BOARD_SIZE * BOARD_SIZE),
        (double) 100.0 * nb_cells[1] / (BOARD_SIZE * BOARD_SIZE));
    if(is_game_finished(nb_cells)) {
      printf("\033[KPlayer %d won with an occupation rate of %.2f%%\n",
          curPlayer, (double) 100.0 * nb_cells[(int) curPlayer] / (BOARD_SIZE
            * BOARD_SIZE));
      break;
    }
    curPlayer = (curPlayer + 1) % 2;

  }
  return curPlayer;
}
