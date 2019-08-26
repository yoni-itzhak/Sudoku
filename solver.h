#ifndef HW4_SOLVER_H
#define HW4_SOLVER_H

#include "stack.h"
#include "doubly_linked_list.h"

/**
 * solver summary
 *
 * A container that takes care of the creation of the board, validation and solving.
 * The container supports the following functions:
 *
 * SudokuCell                   - A struct that defines a cell in the sudoku table
 * HasSolution                  - Enum to define if a board has a solution or not
 * Sudoku                       - A struct that holds the sudoku board - current and solution
 * Cell                         - A struct that defines a temporary sudoku cell in the DeterSudoku
 * DeterSudoku                  - A struct that is used temporarily during the deterministic function
 * deleteDigitFromArr           - Deletes the given digit from the array.
 * findThePossibleArray         - Finds all the digits that are valid for a specific cell and stores
 *                                them in the "optionalDigits" array
 * makeSolutionBoardFix         - Turns all the cells from the temporary board to fixed
 * findNextEmptyCell            - Searches the next empty cell in the board
 * freeBoard                    - Frees all board's allocated memory
 * createEmptyBoard             - Creates an empty board, malloc all board's required memory
 * NonEmptyToFixed              - Copies all the cells from sudoku's currentState to the board
 * Backtracking                 - The function visits all empty cells from left to right, then top to bottom,
 *                                filling in digits sequentially, or backtracking when the number is
 *                                found to be not valid.
 * boardSolver                  - The function calls deterministicBacktracking to solve the current board
 *                                and update the solution if needed
 * createSudoku                 - Creates the Sudoku fields and fills them with appropriate values
 * generateBoard                - Calls to "boardSolver" with isRandom=1 for the random backtracking
 * randomDeletingCells          - picks each time a randomized X,Y to be fixed in the board
 * pickRandomNumberFromTheArray - Picks random index from the array
 */

typedef enum{
    INIT,
    EDIT,
    SOLVE
}Mode;

typedef struct{
    int digit;
    int is_fixed;
    int cnt_erroneous; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    int* optionalDigits;
    int numOfOptionalDigits; /*initialized to total_size and will save the length of the "optionalDigits" array*/
    /*int hasSingleLegalValue; only for "autofill" command. initialized to 0. will be 1 if the cell has only single legal value*/
}SudokuCell;


typedef struct{
    SudokuCell*** currentState;
    SudokuCell*** solution;
    int row;
    int column;
    int total_size;
    int cntFilledCell;
    int cntErroneousCells; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    List* list; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    int markErrors; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    Mode mode; /*remember to take care of this field. maybe should be a separate parameter @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    int justStarted; /* 1 if this the first game*/
}Sudoku;

typedef struct{
    int x;
    int y;
}Cell;


int canBacktrack(Stack* stack,int x,int y);
void popToGetToPreviousCell(Stack* stack, int* p_i, int* p_j);
void pushToGetToNextCell(Sudoku* sudoku,Stack* stack,StackItem* stackItem,Cell* currentEmptyCell,int i,int j);
int updateCurrentEmptyCell(Cell* currentEmptyCell,int i, int j);
void copyBoardValues(SudokuCell*** fromBoard, SudokuCell*** toBoard, int total_size);
SudokuCell*** copyBoard(SudokuCell*** board, int total_size);
void updateCellAndOptionalDigits(StackItem* stackItem,SudokuCell*** tmpItemBoard, int total_size, int i,int j);
int exhaustiveBacktracking(Sudoku* sudoku, int x, int y);

void deleteDigitFromArr(SudokuCell*** board, int x, int y, int dig);
void findThePossibleArray(SudokuCell*** board,int row, int column, int x, int y);
void createEmptyBoard(SudokuCell*** board, int total_size);

int ILP_Validation(SudokuCell*** tmpBoard, int row, int column, Command command, int x, int y, int* p_dig);

void findNextEmptyCell(SudokuCell*** board,int total_size,Cell* cell, int x,int y);
void freeBoard(SudokuCell*** board, int total_size);
void currentStateToFixed(Sudoku *sudoku, SudokuCell ***board, int total_size);

#endif

