#include <stdlib.h>
#include <stdio.h>

#include "doubly_linked_list.h"
#include "solver.h"
#include "main_aux.h"
#include "game.h"
#include "stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gurobi_c.h"


/*void allocateGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype, int total_size){
    if (ind == NULL && sol == val && lb == ub && vtype == NULL && total_size == 0){
        return;
    }
    return;
}*/

/*void freeGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype){
    free(*ind);
    free(*sol);
    free(*val);
    free(*lb);
    free(*ub);
    free(*vtype);
}*/

/*void freeGurobi(GRBenv   *env, GRBmodel *model){
    GRBfreemodel(model);  // Free model //
    GRBfreeenv(env); // Free environment //
}*/

int ILP_Validation(SudokuCell*** board, int row, int column, Command command, int x, int y, int* p_dig){
    if (command == RESET && board == NULL && row==column && x==y && p_dig==NULL){
        return 1;
    }
    return 1;
}



int canBacktrack(Stack* stack,int x,int y){
    if (peek(stack)->currentEmptyCell->x == x && peek(stack)->currentEmptyCell->y == y){
        if (peek(stack)->board[x][y]->numOfOptionalDigits==0){
            return 0;
        }
    }
    return 1;
}


void popToGetToPreviousCell(Stack* stack, int* p_i, int* p_j, int total_size){

    pop(stack, total_size);
    *p_i=peek(stack)->currentEmptyCell->x;
    *p_j=peek(stack)->currentEmptyCell->y;
    peek(stack)->board[*p_i][*p_j]->numOfOptionalDigits -= 1;
    deleteDigitFromArr(peek(stack)->board, *p_i, *p_j, peek(stack)->board[*p_i][*p_j]->optionalDigits[0]); /*board, X, Y, digit -> delete the digit from the array*/
    peek(stack)->board[*p_i][*p_j]->digit=0;
}
void pushToGetToNextCell(Sudoku* sudoku,Stack* stack,StackItem* stackItem,Cell* currentEmptyCell,int i,int j, int firstTime){
    if (firstTime == 0){
        stackItem->board[i][j]->numOfOptionalDigits=sudoku->total_size;
        findThePossibleArray(stackItem->board,sudoku->row,sudoku->column, i, j);
    }
    stackItem->currentEmptyCell = currentEmptyCell;
    push(stack,stackItem);
}

void updateCurrentEmptyCell(Cell* currentEmptyCell,int i, int j){
    currentEmptyCell->x=i;
    currentEmptyCell->y=j;
}

void copyOptionalDigitsArray(int* fromArray, int* toArray, int length){
    int i;
    for(i = 0; i < length; i++) {
        toArray[i] =  fromArray[i];
    }
}

void copyBoardValues(SudokuCell*** fromBoard, SudokuCell*** toBoard, int total_size){
    int i,j, numOfOptionalDigs;
    for (i=0;i<total_size;i++){
        for (j=0;j<total_size;j++){
            toBoard[i][j]->digit=fromBoard[i][j]->digit;
            toBoard[i][j]->is_fixed=fromBoard[i][j]->is_fixed;
            numOfOptionalDigs = fromBoard[i][j]->numOfOptionalDigits;
            toBoard[i][j]->numOfOptionalDigits = numOfOptionalDigs;
            free(toBoard[i][j]->optionalDigits);
            toBoard[i][j]->optionalDigits = (int*)malloc(numOfOptionalDigs*(sizeof(int)));
            if(toBoard[i][j]->optionalDigits == NULL){
                printMallocFailedAndExit();
            }
            copyOptionalDigitsArray(fromBoard[i][j]->optionalDigits, toBoard[i][j]->optionalDigits, numOfOptionalDigs); /*original array, copied array, length*/
            toBoard[i][j]->cnt_erroneous=fromBoard[i][j]->cnt_erroneous; /*might not be necessary*/
        }
    }
}

SudokuCell*** copyBoard(SudokuCell*** board, int total_size){
    SudokuCell*** newBoard = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if (newBoard == NULL){
        printMallocFailedAndExit();
    }
    createEmptyBoard(newBoard, total_size);
    copyBoardValues(board, newBoard, total_size);
    return newBoard;
}

void updateCellAndOptionalDigits(StackItem* stackItem,SudokuCell**** tmpItemBoard, int total_size, int i,int j){
    int dig=0;
    (*tmpItemBoard) = copyBoard(stackItem->board, total_size); /*@@need to malloc tmpItemBoard + need to copy all the fields in each cell*/
    dig = (*tmpItemBoard)[i][j]->optionalDigits[0];
    (*tmpItemBoard)[i][j]->numOfOptionalDigits -= 1;
    deleteDigitFromArr((*tmpItemBoard), i, j, dig); /*delete the number from the array*/
    (*tmpItemBoard)[i][j]->digit = dig; /*update the board with the chosen digit*/
}
/*assuming that receiving <X,Y> who the first empty cell*/
int exhaustiveBacktracking(Sudoku* sudoku, SudokuCell*** fixedBoard, int x, int y){
    int total_size = sudoku->total_size,i=x,j=y, cntSolution=0;
    SudokuCell*** tmpItemBoard = NULL; /*will be malloc in "updateCellAndOptionalDigits" func*/
    Stack* stack = newStack(total_size*total_size+1);
    StackItem* stackItem = (StackItem*)malloc(sizeof(StackItem));
    Cell* nextEmptyCell = (Cell*)malloc(sizeof(Cell));
    Cell* currentEmptyCell = (Cell*)malloc(sizeof(Cell));
    if(stackItem == NULL || nextEmptyCell == NULL || currentEmptyCell == NULL){
        printMallocFailedAndExit();
    }
    printf("here2\n");
    printBoard(fixedBoard, total_size, sudoku->row, sudoku->column);
    stackItem->board = copyBoard(fixedBoard, total_size);
    stackItem->board[i][j]->numOfOptionalDigits=sudoku->total_size;
    findThePossibleArray(stackItem->board,sudoku->row,sudoku->column, i, j);
    printBoard(stackItem->board, total_size, sudoku->row, sudoku->column);
    updateCurrentEmptyCell(currentEmptyCell,x,y);
    pushToGetToNextCell(sudoku,stack,stackItem,currentEmptyCell,x,y, 1);
    while (canBacktrack(stack,x,y)) { /*while the array of the first empty cell isn't empty*/
        while (peek(stack)->board[i][j]->numOfOptionalDigits>0){ /*while the array of the current empty cell isn't empty*/
            stackItem=peek(stack);
            updateCellAndOptionalDigits(stackItem, &tmpItemBoard,total_size, i,j);
            findNextEmptyCell(tmpItemBoard, total_size, nextEmptyCell, i, j); /*search for the next empty cell*/
            if (nextEmptyCell->x == -1 && nextEmptyCell->y == -1) { /* -1 if there are no more empty cells = board is full*/
                cntSolution++;
                tmpItemBoard[i][j]->digit=0;
                freeBoard(stackItem->board, total_size); /*TODO: remove the asterisk*/
                stackItem->board=tmpItemBoard;
                continue; /* "increase" the value of cell <i,j>*/
            }
            else{ /*the board isn't full*/
                currentEmptyCell = (Cell*)malloc(sizeof(Cell)); /*TODO: check if get free at the end*/
                if (currentEmptyCell == NULL) {
                    printMallocFailedAndExit();
                }

                i=nextEmptyCell->x;
                j=nextEmptyCell->y;

                updateCurrentEmptyCell(currentEmptyCell,i,j);
            }
            stackItem = (StackItem*)malloc(sizeof(StackItem)); /*TODO: check if get free at the end*/
            if (stackItem == NULL) {
                printMallocFailedAndExit();
            }
            stackItem->board=tmpItemBoard;
            pushToGetToNextCell(sudoku,stack,stackItem,currentEmptyCell,i,j, 0);
        }
        if (canBacktrack(stack,x,y)==0){
            break;
        }
        popToGetToPreviousCell(stack, &i, &j, total_size);
    }
    free(nextEmptyCell);
    freeStack(stack, total_size);
    return cntSolution;
}



/*
 * @params - function receives pointer to DeterSudoku, X, Y and a digit.
 *
 * The function deletes the given digit from the array.
 */
void deleteDigitFromArr(SudokuCell*** board, int x, int y, int dig) {
    int i;
    int j = 0;
    int len = board[x][y]->numOfOptionalDigits;
    int* tmpArr = board[x][y]->optionalDigits;
    int* newArr = (int*) malloc(len * sizeof(int));
    if(newArr == NULL){
        printMallocFailedAndExit();
    }
    for (i = 0; i < len + 1; i++) {
        if (dig != tmpArr[i]) {
            newArr[j] = tmpArr[i];
            j++;
        }
    }
    board[x][y]->optionalDigits = newArr;
    free(tmpArr);
}
/*
 * @params - function receives pointer to DeterSudoku, the sudoku size, X, and Y.
 *
 * The function finds all the digits that are valid for cell[X][Y] and stores them in the "optionalDigits" array.
 */
void findThePossibleArray(SudokuCell*** board,int row, int column, int x, int y){
    int dig,i, total_size=row*column;
    int is_valid;
    int* cellArr = (int*)malloc(row*column* sizeof(int));
    if(cellArr == NULL){
        printMallocFailedAndExit();
    }
    free(board[x][y]->optionalDigits);
    board[x][y]->optionalDigits = cellArr;
    for (i=0;i<total_size;i++){
        cellArr[i]=i+1;
    }
    for (dig=1; dig<total_size+1;dig++){
        /*is_valid = isZValid(board,row,column,x,y,dig);*/
        is_valid = neighborsPossibleArr(board, row, column, x, y, dig);
        if (is_valid==0){ /*digit isn't valid*/
            board[x][y]->numOfOptionalDigits-=1;
            deleteDigitFromArr(board,x,y,dig);
        }
    }
}

/*
 * @params - function receives SudokuCell*** board and the Sudoku size.
 *
 * The function create an empty board, malloc all board's required memory. also it resets each cell.
 */
void createEmptyBoard(SudokuCell*** board, int total_size){
    int i,j;
    for(i=0; i<total_size; i++){
        board[i] = (SudokuCell**)malloc(total_size*sizeof(SudokuCell*));
        if(board[i] == NULL){
            printMallocFailedAndExit();
        }
        for(j=0; j<total_size;j++){
            board[i][j] = (SudokuCell*)malloc(sizeof(SudokuCell));
            if(board[i][j] == NULL){
                printMallocFailedAndExit();
            }
            board[i][j]->digit=0;
            board[i][j]->is_fixed=0;
            board[i][j]->cnt_erroneous=0;
            board[i][j]->numOfOptionalDigits=total_size;
            board[i][j]->optionalDigits=(int*)malloc(total_size* sizeof(int));
            if(board[i][j]->optionalDigits == NULL){
                printMallocFailedAndExit();
            }
        }
    }
}

/*
 * @params - function receives SudokuCell***, the Sudoku size, pointer to Cell, and the indexes x,y.
 *
 * The function search the next empty cell in the board.
 *
 * @return - if the board is full, the function will return -1 as Cell's indexes.
 * otherwise, it will return the next empty cell.
 */
void findNextEmptyCell(SudokuCell*** board,int total_size,Cell* cell, int x,int y){
    int i,j;
    for (i=0;i<total_size;i++){
        if (i<x){
            continue;
        }
        for (j=0;j<total_size;j++){
            if (i==x && j<y){
                continue;
            }
            if (board[i][j]->digit==0){
                cell->x=i;
                cell->y=j;
                return;
            }
        }
    }
    cell->x=-1;
    cell->y=-1;
}





/*
 * @params - function receives pointer to the main Sudoku, SudokuCell*** board and the Sudoku size.
 *
 * The function copies all the cells from Sudoku's currentState to the board (from input).
 * after that, turns not-Emtpy cells to fixed.
 */
void currentStateToFixed(Sudoku* sudoku, SudokuCell*** board, int total_size){


    int cnt=0,i,j,dig, finishCopy=0, x,y;
    printf("here\n\n");
    for (i=0;i<total_size;i++){
        for (j=0;j<total_size;j++) {
            if (cnt < sudoku->cntFilledCell) { /*more filled cells are left to copy*/
                dig = sudoku->currentState[i][j]->digit;
                if (dig != 0) { /*dig!=0 -> cell isn't empty*/
                    board[i][j]->digit = dig;
                    board[i][j]->is_fixed = 1;
                    cnt++;
                } else { /*dig=0 -> cell is empty*/
                    board[i][j]->digit = 0;
                    board[i][j]->is_fixed = 0;
                }
            }
            else {
                finishCopy = 1; /*all filled cells were copied*/
                for (x = 0; x < total_size; x++) {
                    for (y = 0; y < total_size; y++) {
                        if (sudoku->currentState[x][y]->digit != board[x][y]->digit) {
                            printf("%d %d with %d %d\n", x, y, sudoku->currentState[x][y]->digit, board[x][y]->digit);
                        }
                    }
                }
                printf("good1\n\n");
                break;
            }
        }
        if (finishCopy==1){ /*all filled cells were copied*/
            for (x = 0; x < total_size; x++) {
                for (y = 0; y < total_size; y++) {
                    if (sudoku->currentState[x][y]->digit != board[x][y]->digit) {
                        printf("%d %d with %d %d\n", x, y, sudoku->currentState[x][y]->digit, board[x][y]->digit);
                    }
                }
            }
            printf("good2\n\n");
            break;
        }
    }
}
