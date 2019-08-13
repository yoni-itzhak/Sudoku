

#ifndef HW4_GAME_H
#define HW4_GAME_H


#include "solver.h"
#include "doubly_linked_list.h"



/**
 * game summary:
 *
 * A container that supports the "move" functions:
 *
 * isFixed          - checks if the cell is fixed.
 * findHeadBlock    - finds the top-left cell in the block of cell[X][Y].
 * isZValid         - checks if the value Z can be set in cell[X][Y].
 * isSolved         - checks if the board is solved.
 * isRowValid       - checks if the value Z appears in the required row.
 * isColumnValid    - checks if the value Z appears in the required column.
 * isBlockValid     - checks if the value Z appears in the required block.
 * resetCell        - resets the cell[X][Y].
 * setCell          - sets the cell[X][Y] with value Z.
 * hint             - gives a hint to cell[X][Y].
 * validate         - checks if the current board is valid.
 */

typedef enum{
    UNSOLVED,
    SOLVED
}SET_STATUS;

int isContainsValue(Sudoku* sudoku, int x, int y);
int isThereXEmptyCells(Sudoku* sudoku, int x);
int isFilled(Sudoku* sudoku);
int isErroneous(Sudoku* sudoku);
SET_STATUS lastCellToBeFilled(Sudoku* sudoku);
void solve(); /*path*/
void edit(); /*path*/
void mark_errors(Sudoku* sudoku,int x);
void print_board(Sudoku* sudoku);
void addOneMoveToList(Sudoku *sudoku, int x, int y, int value, int z);
SET_STATUS set(Sudoku* sudoku, int x, int y, int z);
void validate(Sudoku* sudoku);
void guess(Sudoku* sudoku, float x);
void generate(Sudoku* sudoku, int x, int y);
int hasMoveToUndo(Sudoku* sudoku);
void setPointerToPreviousMove(Sudoku* sudoku);
void updateTheBoard(Sudoku* sudoku, Move* move, int command);
void undoMove(Sudoku* sudoku);
void undo(Sudoku* sudoku);
void setPointerToNextMove(Sudoku* sudoku);
int hasMoveToRedo(Sudoku* sudoku);
void redoMove(Sudoku* sudoku);
void redo(Sudoku* sudoku);
void save();/*path*/
void hint(Sudoku* sudoku, int x, int y);
void guess_hint(Sudoku* sudoku, int x, int y);
void num_solutions(Sudoku* sudoku);
void markSingleLegalValue(Sudoku* sudoku);
int hasSingleLegalValue(Sudoku* sudoku, int i, int j);
void updateObviousCell(Sudoku* sudoku, int i, int j,Move** arrMove, int arrSize);
void fillObviousValues(Sudoku* sudoku);
void autofill(Sudoku* sudoku);
void undoAllMoves(Sudoku* sudoku);
void reset(Sudoku* sudoku);
void exitProgram(Sudoku* sudoku);


/****************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************/

int isFixed(Sudoku* sudoku, int x, int y);
void findHeadBlock(Cell* head,int row,int column, int x, int y);
int isZValid(SudokuCell*** board, int row, int column, int x, int y, int z);
int isSolved(Sudoku* sudoku);
int isRowValid(SudokuCell*** board ,int x, int y, int z);
int isColumnValid(SudokuCell*** board, int x, int y, int z);
int isBlockValid(SudokuCell*** board,int row, int column, int x, int y, int z);
void resetCell(Sudoku* sudoku, int x, int y);
void setCell(Sudoku* sudoku, int x, int y, int z);
SET_STATUS set1(Sudoku* sudoku, int x, int y, int z);
void hint1(Sudoku* sudoku, int x, int y);
void validate1(Sudoku* sudoku);


#endif

