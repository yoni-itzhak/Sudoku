

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

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*loading files*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int check_EOF_and_invalid_scan(char* path, int scan, void (*errorFunc)(char*));
int scanRowAndColumn(FILE* file, char* path, int* p_row, int* p_column);
int areOnlyWhitespacesLeft(FILE* file);
int scanCells(FILE* file, char* path, SudokuCell*** board, Mode mode, int total_size, int* p_cntFilledCell);
/* TODO: not finished! */ int check_validation_of_loaded_board(Sudoku *tmpSudoku, char *path, Mode newMode,
                                                               int editWithoutPath, SudokuCell ***newCurrentState,
                                                               int newRow, int newColumn, int newCntFilledCell);
/* TODO: not finished! */ int updateSudoku(Sudoku* sudoku, char* path, Mode newMode, int editWithoutPath, SudokuCell*** newCurrentState, int newRow, int newColumn, int newCntFilledCell);
int fileToSudoku(Sudoku* sudoku, FILE* file, char* X, Mode mode);
int loadBoardFromPath(Sudoku *sudoku, char *X, Mode mode);
void solve(Sudoku* sudoku, char* X);
void editWithPath(Sudoku* sudoku, char* X);
void editWithoutPath(Sudoku* sudoku);
void edit(Sudoku* sudoku, char* X);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*Erroneous checking*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int isErroneous(Sudoku* sudoku);
int isCellErroneous(SudokuCell*** board, int x, int y);
void findErroneousCellsForLoadedBoard(SudokuCell ***board, int total_size, int row, int column, int *p_cntErroneousCells);
int isErroneousBetween2FixedCells(Sudoku* sudoku);
void setCntNeighborsErroneous(SudokuCell*** board, int x, int y, NeighborsType neighborsType, int cntNeighborsErroneous);
void updateSudokuCntErroneousCells(int* p_cntErroneousCells, int beforeErroneous, int afterErroneous);
void neighborsErroneous(Sudoku* sudoku, int x, int y, int dig, int z, Move** arrMove, int* p_arrSize);
void cutErroneousInHalf(SudokuCell*** board, int total_size);
void erroneousAmongNeighbors(SudokuCell ***board, int x, int y, int i, int j, int dig, Move **arrMove, int *p_arrSize, NeighborsType neighborsType, int *p_cntTotalErroneousCells, int *p_cntNeighborsErroneous);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Values validation */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int isRowValid(SudokuCell*** board, int row, int column, int x, int y,  int value , Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
int isColumnValid(SudokuCell*** board, int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
void findHeadBlock(Cell* head,int row,int column, int x, int y);
int isBlockValid(SudokuCell*** board ,int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
int isValueValid(SudokuCell*** board, int row, int column, int x, int y, int value, Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells);
void neighborsLoadFile(SudokuCell*** board, int x, int y, int dig, int row, int column, int* p_cntErroneousCells);
void neighborsToDig(Sudoku *sudoku, int x, int y, int dig, Move **arrMove, int *p_arrSize);
void neighborsFromDig(Sudoku *sudoku, int x, int y, int dig, Move **arrMove, int *p_arrSize);
int neighborsPossibleArr(SudokuCell*** board, int row, int column, int x, int y, int dig);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Board and cells actions and characteristics*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int isContainsValue(Sudoku* sudoku, int x, int y);
int isFilled(Sudoku* sudoku);
int isFixed(SudokuCell*** board, int x, int y);
int isFixedInEdit(Sudoku *sudoku, int i, int j);
void lastCellToBeFilled(Sudoku* sudoku);
void backToOriginalState(SudokuCell*** originalBoard, SudokuCell*** invalidBoard, int total_size);
void updateSudokuCntFilledCells(int* p_cntFilledCell, int fromValue, int toValue);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Moves array*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void addMoveToArrMoveAndIncrementSize(Move **arrMove, int *p_arrSize, int x, int y, int beforeValue, int afterValue, int beforeErroneous, int afterErroneous);
void addArrMoveToList(Sudoku *sudoku, Move** arrMove, int arrSize);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Generate*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void createGenerateMovesArr(Move **arrMove, Sudoku *sudoku, SudokuCell ***tmpBoard);

void generate(Sudoku* sudoku, int x, int y);

void createEmptyCellsArr(Sudoku *sudoku, Cell** emptyCellsArr);

void swapArrayCells(Cell** cellsArr, int index_1, int index_2);

int fillXCells(SudokuCell*** tmpBoard, int x, Cell** emptyCellsArr, int numOfEmptyCells, int row, int column);

void resetCells(SudokuCell*** board, Cell** cellsArr, int numOfCells);

void keepYCells(SudokuCell*** tmpBoard, int y, Sudoku* sudoku);


/* TODO: 1. take care of erroneous cells when adding a move to undo\redo list (if the cell has changed and became erroneous - and vice versa)
 *       2. take care of undo\redo list with "generate" or "guess"
 *       3. take care of erroneous in general (when changing cells for example)*/



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Mark errors*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void mark_errors(Sudoku* sudoku,int x);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Set*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void setCell(Sudoku* sudoku, int x, int y, int z, Move** arrMove, int* p_arrSize);
void set(Sudoku* sudoku, int x, int y, int z);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Print board*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void print_board(Sudoku* sudoku);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Validate*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void validate(Sudoku* sudoku);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Guess*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void guess(Sudoku* sudoku, float x);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Redo\Undo */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int hasMoveToUndo(Sudoku* sudoku);
int hasMoveToRedo(Sudoku* sudoku);
void setPointerToPreviousMove(Sudoku* sudoku);
void setPointerToNextMove(Sudoku* sudoku);
void updateTheBoard(Sudoku* sudoku, Move* move, Command command);
int isTrivialSet(Move* currentMove);
void undoMove(Sudoku* sudoku, Command command);
void undo(Sudoku* sudoku);
void redoMove(Sudoku* sudoku);
void redo(Sudoku* sudoku);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Save */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int checkIfWriteFailed(char *X, int isWrite);
void saveBoardInFile(Sudoku* sudoku, char* X);
void save(Sudoku* sudoku, char* X);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Hint */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void hint(Sudoku* sudoku, int x, int y);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Guess hint */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void guess_hint(Sudoku* sudoku, int x, int y);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Num solutions */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void num_solutions(Sudoku* sudoku);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Autofill */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void markSingleLegalValue(Sudoku* sudoku);
int hasSingleLegalValue(Sudoku* sudoku, int i, int j);
int fillObviousValues(Sudoku* sudoku, int autoFillBeforeILP);
void autofill(Sudoku* sudoku);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Reset */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void undoAllMoves(Sudoku* sudoku);
void reset(Sudoku* sudoku);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Exit program */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void exitProgram();


#endif

