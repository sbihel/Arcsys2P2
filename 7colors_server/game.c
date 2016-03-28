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

/** Ask the user for game settings, then play the game according to the settings
 * the user gave.
 */
void init_game()
{
  char game_types[2]; // we don't have to malloc' it as it will keep alive
  int depths[2];
  // ask the user for the number of games to play
  unsigned int nb_games = ask_tournament();
  
  // ask the user for players strategies
  ask_game_type(&(game_types[0]), &(depths[0]), (char)0x00);
  ask_game_type(&(game_types[0]), &(depths[0]), (char)0x01);
  
  if(nb_games > 1)
    tournament(game_types, depths, nb_games); 
  else {
    // ensure equality between the contestants
    char* board = malloc(BOARD_SIZE * BOARD_SIZE);
    symmetric_fill_board(board);
    game(board, depths, game_types);
  }
}

/** Ask the user for the number of games to play. */
unsigned int ask_tournament() 
{
    printf("How many turns will there be ?\nIf there is more than one turn,"
           " then this is a tournament and stats will be displayed."
           "\n>");
    unsigned int ans;
    scanf("%d", &ans);
    getchar(); // get rid of the '\n'
    return ans;
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

/** The main game function, that runs a single game.
 * @param board An initialized board.
 * @param depths The depths to which alphabetas and minimaxes will go to.
 * This array should have been filled by calling ask_game_type().
 * @param game_types The game types that will apply to each player.
 * This array should have been filled by calling ask_game_type().
 */
char game(char* board, int* depths, char* game_types)
{
  char curPlayer = 0;
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
    switch(game_types[(int)curPlayer])
    {
      case '1': // human v. human
        nextColor = ask(curPlayer);
        break;
      case '2': // alphabeta
        nextColor = alphabeta_with_depth(board, (curPlayer)?SYMBOL_1:SYMBOL_0, 
                                         depths[(int)curPlayer]);
        printf("\033[H\033[KAI %d (alphabeta) played %c\n", curPlayer, 
                nextColor);
        break;
      case '3': // minimax
        nextColor = minimax_with_depth(board, (curPlayer)?SYMBOL_1:SYMBOL_0, 
                                       depths[(int)curPlayer]);
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
                                                depths[(int)curPlayer]);
        printf("\033[H\033[KAI %d (Alphabeta hegemonic) played %c\n", curPlayer, 
                nextColor);
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

/** Run a tournament then show statistics. Running a tournament is 
 * reinitializing the board after each game, and counting the score.
 * @param depths The depths to which alphabetas and minimaxes will go to.
 * This array should have been filled by calling ask_game_type().
 * @param game_types The game types that will apply to each player.
 * This array should have been filled by calling ask_game_type().
 * @param nb_games How many games should be played.
 */
void tournament(char* game_types, int* depths, int nb_games) 
{
  int res[2] = {0, 0};
  char *board = malloc(BOARD_SIZE * BOARD_SIZE);
  int i;
  for(i = 0; i < nb_games; i++) {
    symmetric_fill_board(board); // ensure equality between the contestants
      char winner = game(board, depths, game_types);
      res[(int)winner]++;
  }
  printf("AI n.0 won %.2f%% of the time\n", (double) 100 * res[0] / nb_games);
  printf("AI n.1 won %.2f%% of the time\n", (double) 100 * res[1] / nb_games);
  
}
