#include <stdlib.h>
#include <stdio.h>

#include "doubly_linked_list.h"
#include "solver.h"
#include "main_aux.h"
#include "game.h"
#include "stack.h"



int canBacktrack(Stack* stack,int x,int y){
    if (peek(stack)->currentEmptyCell->x == x && peek(stack)->currentEmptyCell->y == y){
        if (peek(stack)->board[x][y]->numOfOptionalDigits==0){
            return 0;
        }
    }
    return 1;
}


void popToGetToPreviousCell(Stack* stack, int* p_i, int* p_j){

    *p_i=peek(stack)->currentEmptyCell->x;
    *p_j=peek(stack)->currentEmptyCell->y;
    peek(stack)->board[*p_i][*p_j]=0;
    pop(stack);
    peek(stack)->board[*p_i][*p_j]->numOfOptionalDigits -= 1;
    deleteDigitFromArr(peek(stack)->board, *p_i, *p_j, peek(stack)->board[*p_i][*p_j]->optionalDigits[0]); /*board, X, Y, digit -> delete the digit from the array*/
}
void pushToGetToNextCell(Sudoku* sudoku,Stack* stack,StackItem* stackItem,Cell* currentEmptyCell,int i,int j){

    stackItem->board[i][j]->numOfOptionalDigits=sudoku->total_size;
    findThePossibleArray(stackItem->board,sudoku->row,sudoku->column, i, j);
    stackItem->currentEmptyCell = currentEmptyCell;
    push(stack,stackItem);
}

int updateCurrentEmptyCell(Cell* currentEmptyCell,int i, int j){
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
    int i,j;
    for (i=0;i<total_size;i++){
        for (j=0;j<total_size;j++){
            toBoard[i][j]->digit=fromBoard[i][j]->digit;
            toBoard[i][j]->is_fixed=fromBoard[i][j]->is_fixed;
            toBoard[i][j]->numOfOptionalDigits=fromBoard[i][j]->numOfOptionalDigits;
            copyOptionalDigitsArray(fromBoard[i][j]->optionalDigits, toBoard[i][j]->optionalDigits, toBoard[i][j]->numOfOptionalDigits); /*original array, copied array, length*/
            toBoard[i][j]->is_erroneous=fromBoard[i][j]->is_erroneous; /*might not be necessary*/
            toBoard[i][j]->hasSingleLegalValue = fromBoard[i][j]->hasSingleLegalValue; /*might not be necessary*/
        }
    }
}

SudokuCell*** copyBoard(SudokuCell*** board, int total_size){
    SudokuCell*** newBoard;
    createEmptyBoard(newBoard, total_size);
    copyBoardValues(board, newBoard, total_size);
    return newBoard;
}

void updateCellAndOptionalDigits(StackItem* stackItem,SudokuCell*** tmpItemBoard, int total_size, int i,int j){
    int dig=0;
    tmpItemBoard = copyBoard(stackItem->board, total_size); /*@@need to malloc tmpItemBoard + need to copy all the fields in each cell*/
    dig = tmpItemBoard[i][j]->optionalDigits[0];
    tmpItemBoard[i][j]->numOfOptionalDigits -= 1;
    deleteDigitFromArr(tmpItemBoard, i, j, dig); /*delete the number from the array*/
    tmpItemBoard[i][j]->digit = dig; /*update the board with the chosen digit*/
}
/*assuming that receiving <X,Y> who the first empty cell*/
int exhaustiveBacktracking(Sudoku* sudoku, int x, int y){
    int total_size = sudoku->total_size,i=x,j=y, cntSolution=0, row = sudoku->row, column = sudoku->column;
    SudokuCell*** tmpItemBoard; /*will be malloc in "updateCellAndOptionalDigits" func*/
    Stack* stack = newStack(total_size*total_size+1);
    StackItem* stackItem = (StackItem*)malloc(sizeof(StackItem));
    Cell* nextEmptyCell = (Cell*)malloc(sizeof(Cell));
    Cell* currentEmptyCell = (Cell*)malloc(sizeof(Cell));
    if(stackItem == NULL || nextEmptyCell == NULL || currentEmptyCell == NULL){
        printMallocFailed();
    }

    stackItem->board = copyBoard(sudoku->currentState, total_size);
    updateCurrentEmptyCell(currentEmptyCell,x,y);
    pushToGetToNextCell(sudoku,stack,stackItem,currentEmptyCell,x,y);
    while (canBacktrack(stack,x,y)) { /*while the array of the first empty cell isn't empty*/
        while (peek(stack)->board[i][j]->numOfOptionalDigits>0){ /*while the array of the current empty cell isn't empty*/
            stackItem=peek(stack);
            updateCellAndOptionalDigits(stackItem, tmpItemBoard,total_size, i,j);
            findNextEmptyCell(tmpItemBoard, total_size, nextEmptyCell, i, j); /*search for the next empty cell*/
            if (nextEmptyCell->x == -1 && nextEmptyCell->y == -1) { /* -1 if there are no more empty cells = board is full*/
                cntSolution++;
                continue; /* "increase" the value of cell <i,j>*/
            }
            else{ /*the board isn't full*/
                updateCurrentEmptyCell(currentEmptyCell,i,j);
                i=nextEmptyCell->x;
                j=nextEmptyCell->y;
            }
            stackItem->board=tmpItemBoard;
            pushToGetToNextCell(sudoku,stack,stackItem,currentEmptyCell,i,j);
        }
        if (canBacktrack(stack,x,y)==0){
            break;
        }
        popToGetToPreviousCell(stack, &i, &j);
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
        printMallocFailed();
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
        printMallocFailed();
    }
    free(board[x][y]->optionalDigits);
    board[x][y]->optionalDigits = cellArr;
    for (i=0;i<total_size;i++){
        cellArr[i]=i+1;
    }
    for (dig=1; dig<total_size+1;dig++){
        is_valid = isZValid(board,row,column,x,y,dig);
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
            printMallocFailed();
        }
        for(j=0; j<total_size;j++){
            board[i][j] = (SudokuCell*)malloc(sizeof(SudokuCell));
            if(board[i][j] == NULL){
                printMallocFailed();
            }
            board[i][j]->digit=0;
            board[i][j]->is_fixed=0;
            board[i][j]->is_erroneous=0;
            board[i][j]->numOfOptionalDigits=total_size;
            board[i][j]->optionalDigits=(int*)malloc(total_size* sizeof(int));
            if(board[i][j]->optionalDigits == NULL){
                printMallocFailed();
            }
        }
    }
}

/****************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************/


/*
 * @params - function receives DeterSudoku*, the Sudoku size (for "freeBoard func.), and HasSolution enum.
 *
 * The function frees deterSudoku's fields, ant itself.
 *
 * note - if there is a solution to the board, the function will not free the tmpBoard.
 */
void freeDeterSudoku(DeterSudoku* deterSudoku, int total_size, HasSolution hasSolution){
    free(deterSudoku->firstEmpty);
    if (hasSolution==NO_SOLUTION){
        freeBoard(deterSudoku->tmpBoard,total_size);
    }
    free(deterSudoku);
}

/*
 * @params - function receives SudokuCell*** tmpBoard and the Sudoku size.
 *
 * The function turns all the cells from the tmpBoard to fixed.
 */

void makeSolutionBoardFix(SudokuCell*** tmpBoard, int total_size){
    int i,j;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            tmpBoard[i][j]->is_fixed=1;
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
 * @params - function receives SudokuCell*** board and the Sudoku size.
 *
 * The function frees all board's allocated memory.
 */
void freeBoard(SudokuCell*** board, int total_size){
    int i,j;
    for(i=0; i< total_size; i++){
        for(j=0; j<total_size;j++){
            free(board[i][j]->optionalDigits);
            free(board[i][j]);
        }
        free(board[i]);
    }
    free(board);
}



/*
 * @params - function receives pointer to the main Sudoku, SudokuCell*** board and the Sudoku size.
 *
 * The function copies all the cells from Sudoku's currentState to the board (from input).
 * after that, turns not-Emtpy cells to fixed.
 */
void currentStateToFixed(Sudoku* sudoku, SudokuCell*** board, int total_size){

    int cnt=0,i,j,dig, finishCopy=0;
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
                finishCopy=1; /*all filled cells were copied*/
                break;
            }
        }
        if (finishCopy==1){ /*all filled cells were copied*/
            break;
        }
    }
}


/*
 * @params - function receives pointer to the main Sudoku.
 *
 * The function allocates the required memory for DeterSudoku and it's fields and updates them with appropriate values.
 *
 * Also, calls "deterministicBacktracking". if the board has a solution,
 * the function update sudoku->solution with the new one.
 *
 * in any event, frees all the allocated memory for solving the board.
 *
 * @return - according to "deterministicBacktracking".
 */

HasSolution boardSolver(Sudoku* sudoku, int isRandom){

    HasSolution hasSolution;
    int total_size = sudoku->total_size;
    DeterSudoku* deterSudoku;
    SudokuCell*** fixedBoard;
    Cell* firstEmptyCell;
    deterSudoku = (DeterSudoku*)malloc(sizeof(DeterSudoku));
    if(deterSudoku == NULL){
        printMallocFailed();
    }
    fixedBoard = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if(fixedBoard == NULL){
        printMallocFailed();
    }
    firstEmptyCell = (Cell*)malloc(sizeof(Cell));
    if(firstEmptyCell == NULL){
        printMallocFailed();
    }
    createEmptyBoard(fixedBoard,total_size);
    currentStateToFixed(sudoku, fixedBoard, total_size);/*copy the sudoku.currentState to fixedBoard*/
    findNextEmptyCell(fixedBoard, total_size,firstEmptyCell,0,0); /*search for the first empty cell in the board*/
    /*fill the fields of deterSudoku*/
    deterSudoku->firstEmpty = firstEmptyCell;
    deterSudoku->tmpBoard=fixedBoard;
    deterSudoku->row=sudoku->row;
    deterSudoku->column=sudoku->column;

    /*search for a solution, 1 is sent to define it will be done randomly*/
    hasSolution = Backtracking(deterSudoku, isRandom, total_size, firstEmptyCell->x, firstEmptyCell->y);
    if (hasSolution==SOLUTION){
        makeSolutionBoardFix(deterSudoku->tmpBoard, total_size); /*turns the cells of the deterministic solution to fixed*/
        freeBoard(sudoku->solution,total_size); /*free old solution memory*/
        sudoku->solution=deterSudoku->tmpBoard; /*store the new solution*/
        freeDeterSudoku(deterSudoku,total_size,hasSolution);
        return SOLUTION;
    }
    else {/*hasSolution==NO_SOLUTION*/
        freeDeterSudoku(deterSudoku,total_size,hasSolution);
        return NO_SOLUTION;
    }
}

/*
 * @params - function receives pointer to the main Sudoku.
 *
 * The function calls to "boardSolver" with isRandom=1 for the random backtracking.
 */
void generateBoard(Sudoku *sudoku) {
    boardSolver(sudoku,1);
}

/*
 * @params - function receives pointer to the main Sudoku and the number of cells that the user chose to fill.
 *
 * The function picks each time a randomized X,Y to be fixed in the board.
 */
void randomDeletingCells(Sudoku *sudoku, int numOfFixedCells) {/*Deleting randomized cells*/
    int x, y;
    while (numOfFixedCells != 0) {
        x = rand() %(sudoku->total_size);
        y = rand() %(sudoku->total_size);
        if (sudoku->currentState[y][x]->is_fixed) {/* if is_fixed=1 that means that that the cell is fixed*/
            continue;
        } else {/*setting the cell and making it fixed*/
            sudoku->currentState[y][x]->is_fixed = 1;
            sudoku->currentState[y][x]->digit = sudoku->solution[y][x]->digit;
            numOfFixedCells--;
        }
    }
}
/*
 * @params - function receives pointer to the main Sudoku, the row and column values and the number of cells to be fixed.
 *
 * The function creates the Sudoku fields and fills them with appropriate values.
 */

void createSudoku(Sudoku* sudoku,int row, int column, int num_of_cells) {

    int total_size=row*column;
    SudokuCell*** tmp;
    SudokuCell*** sol;
    tmp = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if(tmp == NULL){
        printMallocFailed();
    }
    sol = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if(sol == NULL){
        printMallocFailed();
    }
    createEmptyBoard(tmp, total_size);
    createEmptyBoard(sol, total_size);
    sudoku->currentState=tmp;
    sudoku->solution=sol;
    sudoku->total_size=total_size;
    sudoku->cntFilledCell=num_of_cells;
    sudoku->column=column;
    sudoku->row=row;
}

/*
 * @params - function receives pointer to an array and its length.
 *
 * The function picks random index from the array.
 *
 * @return - the number placed in array[index].
 */
int pickRandomNumberFromTheArray(int* tmpArr, int numOfOptionalDigits){
    int randNum;
    if(numOfOptionalDigits==1){
        return tmpArr[0];
    }
    randNum=rand()%numOfOptionalDigits;
    return tmpArr[randNum];
}

