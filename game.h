

#ifndef HW4_GAME_H
#define HW4_GAME_H


#include "structs.h"
#include "parser.h"
#include "solver.h"
#include "doubly_linked_list.h"
#include "stack.h"


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
    NEIGHBORS_FROM_DIG,
    NEIGHBORS_TO_DIG,
    NEIGHBORS_LOAD_FILE,
    NEIGHBORS_POSSIBLE_ARRAY
}NeighborsType;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void LP_Guesses();
void fillCellsWithScoreX();
int LP_Validation();
void printAllLegalValues();


void addArrMoveToList(Sudoku*, Move**, int);
int fillXCells(SudokuCell***, int, Cell**, int, int, int);

void createEmptyCellsArr(Sudoku*, Cell**);
int isContainsValue(Sudoku* sudoku, int x, int y);
int isFilled(Sudoku* sudoku);
int isErroneous(Sudoku* sudoku);
void lastCellToBeFilled(Sudoku* sudoku);
void solve(Sudoku*, char*);
void edit(Sudoku*, char*);
void mark_errors(Sudoku* sudoku,int x);
void print_board(Sudoku* sudoku);
/*void addOneMoveToList(Sudoku *sudoku, int x, int y, int value, int z);*/
void set(Sudoku* sudoku, int x, int y, int z);
void validate(Sudoku* sudoku);
void guess(Sudoku* sudoku, float x);
void generate(Sudoku* sudoku, int x, int y);
int hasMoveToUndo(Sudoku* sudoku);
void setPointerToPreviousMove(Sudoku* sudoku);
void updateTheBoard(Sudoku* sudoku, Move* move, Command command);
void undoMove(Sudoku* sudoku, Command command);
void undo(Sudoku* sudoku);
void setPointerToNextMove(Sudoku* sudoku);
int hasMoveToRedo(Sudoku* sudoku);
void redoMove(Sudoku* sudoku);
void redo(Sudoku* sudoku);
void save(Sudoku*, char*);
void hint(Sudoku* sudoku, int x, int y);
void guess_hint(Sudoku* sudoku, int x, int y);
void printAllLegalValues();
void num_solutions(Sudoku* sudoku);
void markSingleLegalValue(Sudoku* sudoku);
int hasSingleLegalValue(Sudoku* sudoku, int i, int j);
/*void updateObviousCell(Sudoku* sudoku, int i, int j,Move** arrMove, int arrSize);*/
void fillObviousValues(Sudoku* sudoku);
void autofill(Sudoku* sudoku);
void undoAllMoves(Sudoku* sudoku);
void reset(Sudoku* sudoku);
void exitProgram();


int isRowValid(SudokuCell*** board, int row, int column, int x, int y,  int value , Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
int isColumnValid(SudokuCell*** board, int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
int isBlockValid(SudokuCell*** board ,int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
int isValueValid(SudokuCell*** board, int row, int column, int x, int y, int value, Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
void setCell(Sudoku* sudoku, int x, int y, int z, Move** arrMove, int* p_arrSize);
int isCellErroneous(SudokuCell*** board, int x, int y);

int loadBoardFromPath(Sudoku* sudoku, char* X, Mode mode);

int fileToSudoku(Sudoku* sudoku, FILE* file, char* X, Mode mode);

int updateSudoku(Sudoku* sudoku,char* X, Mode mode, SudokuCell*** newCurrentState, int newRow, int newColumn, int newCntFilledCell);
void findErroneousCells(Sudoku* sudoku);
void addMoveToArrMoveAndIncrementSize(Move **arrMove, int *p_arrSize, int x, int y, int beforeValue, int afterValue,
                                      int beforeErroneous, int afterErroneous);
void updateSudokuCntErroneousCells(int* p_cnt, int beforeErroneous, int afterErroneous);

int neighborsPossibleArr(SudokuCell*** board, int row, int column, int x, int y, int dig);

/****************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************/

int isFixed(SudokuCell*** board, int x, int y);
void findHeadBlock(Cell* head,int row,int column, int x, int y);
/*int isZValid(SudokuCell*** board, int row, int column, int x, int y, int z);
int isSolved(Sudoku* sudoku);
int isRowValid(SudokuCell*** board ,int x, int y, int z);
int isColumnValid(SudokuCell*** board, int x, int y, int z);
int isBlockValid(SudokuCell*** board,int row, int column, int x, int y, int z);
void resetCell(Sudoku* sudoku, int x, int y);
void setCell(Sudoku* sudoku, int x, int y, int z);
SET_STATUS set1(Sudoku* sudoku, int x, int y, int z);
void hint1(Sudoku* sudoku, int x, int y);
void validate1(Sudoku* sudoku);*/


#endif

