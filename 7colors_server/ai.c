#include "ai.h"

/** Choose a random color. */
char rand_play() {
  char r = rand() % 7;
  return r + 65;
}

/** Choose a random color among those that can be played.
 * @param board The current board
 * @param player The player who has to choose
 */
char rand_valid_play(char *board, char player) {
  char colors_valid[7] = {0, 0, 0, 0, 0, 0, 0};
  int i, j;
  for(i = 0; i < BOARD_SIZE; i++)
  for(j = 0; j < BOARD_SIZE; j++)
    if (is_adjacent(board, i, j, player) && get_cell(board, i, j) >= 'A' &&
        get_cell(board, i, j) <= 'G')
      colors_valid[get_cell(board, i, j) - 65] = 1;
  char move = (rand() % 7) + 65;
  int availMoves = 0;
  for(i = 0; i < 7; i++)
      availMoves += colors_valid[i];
  if(!availMoves)
      return 'A'; // default : we can't do anything
  
  while(!colors_valid[move - 65])
    move = (rand() % 7) + 65;
  return move;
}

/** Choose the move that expands the most the player's area.
 * @param board The board
 * @param player The player who has to choose
 */
char biggest_move(char* board, char player) {
  int score_best_move = 0;
  char best_move = 'A';
  char* board_copy = malloc(BOARD_SIZE * BOARD_SIZE * sizeof(char));
  int i;
  for(i = 0; i < 8; i++) {
    memcpy(board_copy, board, BOARD_SIZE * BOARD_SIZE);
    int score_move = update_board(board_copy, player, i + 65);
    if(score_move > score_best_move) {
      score_best_move = score_move;
      best_move = i + 65;
    }
  }
  free(board_copy);
  return best_move;
}

/** Recursively compute the area available to the player's opponent.
 * @param board The board.
 * @param player The current player
 * @param i The current position
 * @param j The current position
 * @param propI Propagation direction for the abscissa. Should be -1 or 1.
 * @param propJ Propagation direction for the ordinates. Should be -1 or 1.
 */
int availArea(char* board, char player, int i, int j, int propI, int propJ) {
    if(i > BOARD_SIZE - 1 || i < 0 || j > BOARD_SIZE - 1 || j < 0)
        return 0;
    char content = get_cell(board, i, j);
    if(content == player || content == 0x00)
        return 0;
    set_cell(board, i, j, 0x00);
    return 1 + availArea(board, player, i + propI, j, propI, propJ)
    + availArea(board, player, i, j + propJ, propI, propJ);

}

/** Choose a color according tothe hegemonic strategy.
 * @param board The board
 * @param player The current player
 * @param startI The current player's starting position abscissa.
 * @param startJ The current player's starting position ordinate.
 */
char hegemon(char* board, char player, int startI, int startJ,
             int propI, int propJ) {
    char* board_copy = (char*)malloc(BOARD_SIZE * BOARD_SIZE * sizeof(char));
    memcpy(board_copy, board, BOARD_SIZE * BOARD_SIZE);
    int curArea = availArea(board_copy, player, startI, startJ, propI, propJ);
    int i;
    char argmaxColor = 'A';
    for(i = 0; i < 7; i++)
    {
        memcpy(board_copy, board, BOARD_SIZE * BOARD_SIZE);
        char color = 'A' + (char) i;
        int gain = update_board(board_copy, player, color);
        int area = availArea(board_copy, player, startI, startJ, propI, propJ);
        if(area < curArea || (area == curArea && gain > 0) ) {
            argmaxColor = 'A' + (char) i;
            curArea = area;
        }
    }
    free(board_copy);
    return argmaxColor;

}

/** Compute the number of cells for the player.
 * @param board The game board
 * @param player The current player
 */
int compute_score(char *board, char player) {
  int res = 0;
  int i, j;
  for(i = 0; i < BOARD_SIZE; i++)
  for(j = 0; j < BOARD_SIZE; j++)
    if(get_cell(board, i, j) == player)
      res++;
  return res;
}

/** Resource structure for the minimax algorithm. Contains a move and
 * the associated score.
 */
typedef struct {
  int score;
  char move;
} res_minimax;

/** Recursive function for the minimax algorithm. 
 * Returns the best move for the player <code>player</code> according to 
 * minimax.
 * @param board The board on which minimax should be run.
 * @param player The player whose turn it is to choose.
 * @param curPlayer The player minimax is currently considering.
 * @param depth The depth to which the minimax exploration should go.
 */
res_minimax minimax_through(char *board, char player, char curPlayer,
    int depth) {
  res_minimax res;
  res.score = (player == curPlayer)?INT_MIN:INT_MAX;
  // res.move = 'A';
  if(!depth){
    res.score = compute_score(board, player);
    return res;
  }
  if(compute_score(board, curPlayer?SYMBOL_0:SYMBOL_1) >= BOARD_SIZE *
      BOARD_SIZE / 2)  //game is won
    return res;

  char *board_copy = malloc(BOARD_SIZE * BOARD_SIZE);
  int i;
  for(i = 0; i < 8; i++) {
    memcpy(board_copy, board, BOARD_SIZE * BOARD_SIZE);
    int move = update_board(board_copy, curPlayer, i + 65);
    if(move == 0)  // this move doesn't change anyth
      continue;
    res_minimax child = minimax_through(board_copy, player,
        (curPlayer==SYMBOL_1)?SYMBOL_0:SYMBOL_1, depth - 1);
    if((player == curPlayer && child.score >= res.score) ||
        (player != curPlayer && child.score <= res.score)) {
      // Need the score equal to have a somewhat valid move
      res.score = child.score;
      res.move = i + 65;
    }
  }
  free(board_copy);
  return res;
}

/** Wrapper function for the minimax algorithm. Has default depth of 5.
 * @param board The current board
 * @param player The player who has to choose
 */
char minimax(char *board, char player) {
  return minimax_through(board, player, player, 5).move;
}

/** Wrapper function for the minimax algorithm.
 * @param board The current board
 * @param player The player who has to choose
 * @param depth The maximal depth minimax should go to
 */
char minimax_with_depth(char *board, char player, int depth) {
  return minimax_through(board, player, player, depth).move;
}

/** Recursive function for the alphabeta algorithm. 
 * Returns the best move for the player <code>player</code> according to 
 * alphabeta. Alphabeta's heuristic here is the number of cells for each player
 * (i.e. their score).
 * @param board The board on which alphabeta should be run.
 * @param player The player whose turn it is to choose.
 * @param curPlayer The player alphabeta is currently considering.
 * @param depth The depth to which the alphabeta exploration should go.
 * @param lower_bound The minimal score one can except from the caller's 
 * situation.
 * @param upper_bound The maximal score one can except from the caller's 
 * situation.
 */
res_minimax alphabeta_through(char *board, char player, char curPlayer,
    int depth, int lower_bound, int upper_bound) {
  res_minimax res;
  res.score = (player == curPlayer)?INT_MIN:INT_MAX;
  res.move = 0x00;
  if(!depth){
    res.score = compute_score(board, player);
    return res;
  }
  if(compute_score(board, curPlayer?SYMBOL_0:SYMBOL_1) >= BOARD_SIZE *
      BOARD_SIZE / 2)  //game is won
    return res;

  char *board_copy = malloc(BOARD_SIZE * BOARD_SIZE);
  int i;
  for(i = 0; i < 8; i++) {
    memcpy(board_copy, board, BOARD_SIZE * BOARD_SIZE);
    int move = update_board(board_copy, curPlayer, i + 65);
    if(move == 0)  // this move doesn't change anyth
      continue;
    res_minimax child = alphabeta_through(board_copy, player,
        (curPlayer==SYMBOL_1)?SYMBOL_0:SYMBOL_1, depth - 1, lower_bound,
        upper_bound);
    if((player == curPlayer && child.score >= res.score) ||
        (player != curPlayer && child.score <= res.score)) {
      // Need the score equal to have a somewhat valid move
      res.score = child.score;
      res.move = i + 65;
    }
    // Pruning
    if(player == curPlayer) {
      if(lower_bound < res.score) {
        lower_bound = res.score;
        if(upper_bound <= lower_bound)
          break; // The branch we're exploring will lead to loss
      }
    } else {
      if(upper_bound > res.score) {
        upper_bound = res.score;
        if(upper_bound <= lower_bound)
          break;
      }
    }
  }
  free(board_copy);
  return res;
}

/** Wrapper function for the alphabeta algorithm. Has default depth of 6.
 * @param board The current board
 * @param player The player who has to choose
 */
char alphabeta(char *board, char player) {
  return alphabeta_through(board, player, player, 6, INT_MIN, INT_MAX).move;
}

/** Wrapper function for the alphabeta algorithm.
 * @param board The current board
 * @param player The player who has to choose
 * @param depth How deep the alphabeta exploration should go
 */
char alphabeta_with_depth(char *board, char player, int depth) {
  return alphabeta_through(board, player, player, depth, INT_MIN, INT_MAX).move;
}

/** Recursive function for the alphabeta algorithm with a hegemonic heuristic. 
 * Returns the best move for the player <code>player</code> according to 
 * alphabeta. Alphabeta's heuristic here is the opponent's available area, 
 * computed using the hegemonic strategy.
 * @param board The board on which alphabeta should be run.
 * @param player The player whose turn it is to choose.
 * @param curPlayer The player alphabeta is currently considering.
 * @param depth The depth to which the alphabeta exploration should go.
 * @param lower_bound The minimal score one can except from the caller's 
 * situation.
 * @param upper_bound The maximal score one can except from the caller's 
 * situation.
 */
res_minimax alphabeta_perimeter_through(char *board, char player, char
    curPlayer, int depth, int lower_bound, int upper_bound) {
  res_minimax res;
  res.score = (player == curPlayer)?INT_MIN:INT_MAX;
  res.move = 0x00;
  if(!depth){
    int start_i, start_j, prop_i, prop_j;
    if(player == SYMBOL_1) {
      start_i = BOARD_SIZE-1;
      start_j = 0;
      prop_i = -1;
      prop_j = 1;
    }
    else {
      start_i = 0;
      start_j = BOARD_SIZE - 1;
      prop_i = 1;
      prop_j = -1;
    }
    res.score = -availArea(board, player, start_i, start_j, prop_i, prop_j);
    return res;
  }
  if(compute_score(board, curPlayer?SYMBOL_0:SYMBOL_1) >= BOARD_SIZE *
      BOARD_SIZE / 2)  //game is won
    return res;

  char *board_copy = malloc(BOARD_SIZE * BOARD_SIZE);
  int i;
  for(i = 0; i < 8; i++) {
    memcpy(board_copy, board, BOARD_SIZE * BOARD_SIZE);
    int move = update_board(board_copy, curPlayer, i + 65);
    if(move == 0)  // this move doesn't change anyth
      continue;
    res_minimax child = alphabeta_perimeter_through(board_copy, player,
        (curPlayer==SYMBOL_1)?SYMBOL_0:SYMBOL_1, depth - 1, lower_bound,
        upper_bound);
    if((player == curPlayer && child.score >= res.score) ||
        (player != curPlayer && child.score <= res.score)) {
      // Need the score equal to have a somewhat valid move
      res.score = child.score;
      res.move = i + 65;
    }
    // Pruning
    if(player == curPlayer) {
      if(lower_bound < res.score) {
        lower_bound = res.score;
        if(upper_bound <= lower_bound)
          break;
      }
    } else {
      if(upper_bound > res.score) {
        upper_bound = res.score;
        if(upper_bound <= lower_bound)
          break;
      }
    }
  }
  free(board_copy);
  return res;
}

/** Wrapper function for the alphabeta-hegemonic algorithm. Has default depth 
 * of 6.
 * @param board The current board
 * @param player The player who has to choose
 */
char alphabeta_with_expand_perimeter(char *board, char player) {
  return alphabeta_perimeter_through(board, player, player, 6, INT_MIN,
      INT_MAX).move;
}

/** Wrapper function for the alphabeta-hegemonic algorithm.
 * @param board The current board
 * @param player The player who has to choose
 * @param depth How deep the alphabeta exploration should go
 */
char alphabeta_with_expand_perimeter_depth(char *board, char player, int depth)
{
  return alphabeta_perimeter_through(board, player, player, depth, INT_MIN,
      INT_MAX).move;
}
