#ifndef HW4_SOLVER_H
#define HW4_SOLVER_H

#include "parser.h"
#include "structs.h"
#include "gurobi_c.h"

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



void allocateGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype, int total_size, double** obj, int isLP);
void freeGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype,double** obj, int isLP);


int canBacktrack(Stack* stack,int x,int y);
void popToGetToPreviousCell(Stack* stack, int* p_i, int* p_j, int total_size);
void pushToGetToNextCell(Sudoku* sudoku,Stack* stack,StackItem* stackItem,Cell* currentEmptyCell,int i,int j, int firstTime);
void updateCurrentEmptyCell(Cell* currentEmptyCell,int i, int j);
void copyBoardValues(SudokuCell*** fromBoard, SudokuCell*** toBoard, int total_size, int copyFixedCellsOnly);
SudokuCell*** copyBoard(SudokuCell*** board, int total_size, int copyFixedCellsOnly);
void updateCellAndOptionalDigits(StackItem* stackItem,SudokuCell**** tmpItemBoard, int total_size, int i,int j);
int exhaustiveBacktracking(Sudoku* sudoku, SudokuCell*** fixedBoard, int x, int y);

void deleteDigitFromArr(SudokuCell*** board, int x, int y, int dig);
void findThePossibleArray(SudokuCell*** board,int row, int column, int x, int y);


int ILP_Validation(SudokuCell*** tmpBoard, int row, int column, Command command, int x, int y, int* p_dig);
int LP_Validation(SudokuCell*** board, int row, int column, Command command, int x, int y, int* sol_arr, int* arr_size, int* movesArr);

void findNextEmptyCell(SudokuCell*** board,int total_size,Cell* cell, int x,int y);

void currentStateToFixed(Sudoku *sudoku, SudokuCell ***board, int total_size);
int choose_weighted_rand(WeightedCell* arr, int arr_size)

#endif

