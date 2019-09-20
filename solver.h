#ifndef HW4_SOLVER_H
#define HW4_SOLVER_H

#include "parser.h"
#include "structs.h"
#include "gurobi_c.h"

/**
 * solver summary
 *
 * A container that takes care of two main parts of the code:
 * 1. The Gurobi functions: LP and ILP
 * 2. Exhaustive backtracking: The main algorithm for num_solutions
 *
 */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Gurobi & co.*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void allocateGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype, int total_size, double** obj, int isLP);
void freeGurobi(GRBenv *env, GRBmodel *model);
void freeGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype,double** obj, int isLP);
int GRBSolver(Sudoku *sudoku, SudokuCell ***board, int isLP, int row, int column, Command command, int x, int y, float threshold, int *p_dig);
void freePossibleSolArr(WeightedCell*** possible_sol_arr, int total_size);
void MallocAndFindPossibleSolArr(WeightedCell*** possible_sol_arr, int* possible_sol_arr_size, double* sol, Sudoku* sudoku, int x, int y, float threshold);
int choose_weighted_rand(WeightedCell** arr, int arr_size);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Exhaustive backtracking*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int canBacktrack(Stack* stack,int x,int y);
void popToGetToPreviousCell(Stack* stack, int* p_i, int* p_j, int total_size);
void pushToGetToNextCell(Sudoku* sudoku,Stack* stack,StackItem* stackItem,Cell* currentEmptyCell,int i,int j, int firstTime);
void updateCurrentEmptyCell(Cell* currentEmptyCell,int i, int j);
void updateCellAndOptionalDigits(StackItem* stackItem,SudokuCell**** tmpItemBoard, int total_size, int i,int j);
int exhaustiveBacktracking(Sudoku* sudoku, SudokuCell*** fixedBoard, int x, int y);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Board functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void copyOptionalDigitsArray(int* fromArray, int* toArray, int length);
void copyBoardValues(SudokuCell*** fromBoard, SudokuCell*** toBoard, int total_size, int copyFixedCellsOnly);
SudokuCell*** copyBoard(SudokuCell*** board, int total_size, int copyFixedCellsOnly);
void deleteDigitFromArr(SudokuCell*** board, int x, int y, int dig);
void findThePossibleArray(SudokuCell*** board,int row, int column, int x, int y);
void findNextEmptyCell(SudokuCell*** board,int total_size,Cell* cell, int x,int y);
void currentStateToFixed(Sudoku* sudoku, SudokuCell*** board, int total_size);

#endif

