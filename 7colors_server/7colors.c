#include "7colors.h"


/** Represent the actual current board game
 *
 * NOTE: global variables are usually discouraged (plus encapsulation in
 *  an appropriate data structure would also be preferred), but don't worry.
 *  For this first assignment, no dinosaure will get you if you do that.
 */
// char board[BOARD_SIZE * BOARD_SIZE] = { 0 }; // Filled with zeros

/** Retrieve the color of a given board cell
 * @param board The board.
 * @param x The abscissa of the cell to retrieve.
 * @param y The ordinate of the cell to retrieve.
 */
char get_cell(char* board, int x, int y)
{
  return board[y*BOARD_SIZE + x];
}

/** Change the color of a given board cell.
 * @param board The board.
 * @param x The target cell abscissa.
 * @param y The target cell ordinate.
 */
void set_cell(char* board, int x, int y, char color)
{
  board[y*BOARD_SIZE + x] = color;
}

/** Print the current state of the board on screen
 *
 * Implementation note: It would be nicer to do this with ncurse or even
 * SDL/allegro, but this is not really the purpose of this assignment.
 * 
 * @param board The board to be printed.
 * 
 */
void print_board(char* board)
{
  printf("\033[2H");  // move to the top of the screen
  int i, j;
  for (i=0; i<BOARD_SIZE; i++) {
    for (j=0; j<BOARD_SIZE; j++) {
      switch(get_cell(board, i, j)) {
        case 'A': printf("\033[41;30m");
                  break;
        case 'B': printf("\033[42;30m");
                  break;
        case 'C': printf("\033[43;30m");
                  break;
        case 'D': printf("\033[44;30m");
                  break;
        case 'E': printf("\033[45;30m");
                  break;
        case 'F': printf("\033[46;30m");
                  break;
        case 'G': printf("\033[47;30m");
                  break;
        case '.': printf("\033[40;31;1m");
                  break;
        case '`': printf("\033[40;34;1m");
      }
      printf(" %c ", get_cell(board, i, j));
      printf("\033[0m");
    }
    printf("\n");
  }
}

/** Fill the board with random colors.
 * @param board The board. Note that this function doesn't allocate any memory.
 */
void fill_board(char* board)
{
  int i,j;
  for(i = 0; i < BOARD_SIZE; i++) {
    for(j = 0; j < BOARD_SIZE; j++) {
      set_cell(board, i, j, (char)(rand() % 0x07) + 65);
    }
  }
  set_cell(board, BOARD_SIZE - 1, 0, SYMBOL_0);
  set_cell(board, 0, BOARD_SIZE - 1, SYMBOL_1);
}


/** Fill the board with random colors as a symmetric matrix.
 * @param board The board. Note that this function doesn't allocate any memory.
 */
void symmetric_fill_board(char *board) {
  int i, j;
  for(i = 0; i < BOARD_SIZE; i++) {
    for(j = i; j < BOARD_SIZE; j++) {
      char color = (char)(rand() % 0x07) + 65;
      set_cell(board, i, j, color);
      set_cell(board, j, i, color);
    }
  }
  set_cell(board, BOARD_SIZE - 1, 0, SYMBOL_0);
  set_cell(board, 0, BOARD_SIZE - 1, SYMBOL_1);
}

/** Determine whether a cell has adjacent cells belonging to player.
 * @param board The board.
 * @param i The cell's abscissa.
 * @param j The cell's ordinate.
 * @param player The player's symbol.
 */
bool is_adjacent(char* board, int i, int j, char player) {
  if(i && get_cell(board, i - 1, j) == player) return true;
  if(j && get_cell(board, i, j - 1) == player) return true;
  if(i != BOARD_SIZE - 1 && get_cell(board, i + 1, j) == player) return true;
  if(j != BOARD_SIZE - 1 && get_cell(board, i, j + 1) == player) return true;
  return false;
}

/** This is the game's core function. It updates the board given a player, the 
 * color they have chosen to play and the board.
 * Returns the number of cells the player has gained.
 * @param board The board.
 * @param player The symbol for the current player.
 * @param color The color chosen by player.
 */
int update_board(char* board, char player, char color) {
  bool hasChanged = true;
  int nb_cell_acquired = 0;
  while(hasChanged) {
    hasChanged = false;
    int i, j;
    for(i = 0; i < BOARD_SIZE; i++) {
      for(j = 0; j < BOARD_SIZE; j++) {
        if (get_cell(board, i, j) == color && is_adjacent(board, i, j, player)){
          set_cell(board, i, j, player);
          hasChanged = true;
          nb_cell_acquired++;
        }
      }
    }
  }
  return nb_cell_acquired;
}

