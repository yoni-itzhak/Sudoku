#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "solver.h"
#include "main_aux.h"
#include "doubly_linked_list.h"


#define DEFAULT_ROW = 3
#define DEFAULT_COLUMN = 3
#define DEFAULT_NUMBER_OF_CELLS = 81

/* TODO: 1. take care of erroneous cells when adding a move to undo\redo list (if the cell has changed and became erroneous - and vice versa)
 *       2. take care of undo\redo list with "generate" or "guess"
 *       3. take care of erroneous in general (when changing cells for example)*/

int ILP_Validation(SudokuCell*** tmpBoard);
void LP_Guesses(Sudoku* sudoku, float x);
void fillCellsWithScoreX(Sudoku* sudoku,float x);
int fillXCells(SudokuCell*** tmpBoard, int x);
void keepYCells(SudokuCell*** tmpBoard, int y);
int LP_Validation(Sudoku* sudoku);
int checkIfBoardIsSolvable();


void freeMemory(Sudoku* sudoku){
    freeBoard (sudoku->currentState, sudoku->total_size);
    freeBoard (sudoku->solution, sudoku->total_size);
    freeList(sudoku->list);
    free(sudoku);
}

int areOnlyWhitespacesLeft(FILE* file, char* X){
    int c;
    while ((c = fgetc(file)) != EOF){
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n' ){
            printLoadedFileLengthNotValid(X);
            printf("file too long\n");
            return 0;
        }
    }
    return 1;
    /*if (feof(file)){
        printf("finish successfully\n");
        return 1;

    }*/
    /*else if (scan != 1){
        printf("not valid\n");
    }*/
}

void scanRowAndColumn(FILE* file, char* X, int* p_row, int* p_column){
    int scan;
    scan = fscanf(file,"%d", p_column);
    /*checkEOF();*/
    if (scan == EOF){
        printLoadedFileEOF(X);
    }
    if (scan!=1){
        printLoadedFileFirstLineNotValid(X);
    }
    scan = fscanf(file,"%d", p_row);
    /*checkEOF();*/
    if (scan == EOF){
        printLoadedFileEOF(X);
    }
    if (scan!=1){
        printLoadedFileFirstLineNotValid(X);
    }
}
/*TODO: take care of cases that there is an empty line in the middle of the file
 * example:
 * 5
 *
 *      7 */
void scanCells(FILE* file, char* X, SudokuCell*** board, Mode mode, int total_size, int* p_cntFilledCell){
    int i,j, scan, dig, fixed, valid;
    char c;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            scan = fscanf(file,"%d", &dig);
            if (scan == EOF){
                printLoadedFileEOF(X);
            }
            if (scan!=1){
                printLoadedFileCellNotValid(X);
            }
            if ((c=fgetc(file)) != EOF){
                if (c=='.'){
                    fixed = 1;
                }
                else if (c== ' ' || c == '\t' || c == '\r' || c == '\n'){
                    fixed = 0;
                }
                else{
                    printLoadedFileCellNotValid(X);
                }
            }
            else{
                if (j == total_size-1 && i == total_size-1){ /*the last cell*/
                    fixed = 0;
                }
                else{ /*not the last cell*/
                    printLoadedFileEOF(X);
                    printf("eof in dig part two\n");
                }
            }
            if (dig > total_size || dig < 0){
                printLoadedFileCellNotValid(X);
            }
            if ( (dig == 0 && fixed ==1) || (mode == EDIT && fixed == 0) ){
                printLoadedFileCellNotValid(X);
            }
            if (mode == EDIT){ /*in EDIT mode, no cel is considered fixed*/
                fixed = 0;
            }
            board[i][j]->digit = dig;
            board[i][j]->is_fixed = fixed;
            if (dig != 0){
                (*p_cntFilledCell)++;
            }
        }
    }
    valid = areOnlyWhitespacesLeft(file, X); /*check if there are only whitespaces left in the file*/
    if (valid == 0){
        printLoadedFileLengthNotValid(X);
    }
    if (mode == EDIT){
        valid = checkIfBoardIsSolvable();
        if (valid == 0){
            printLoadedFileNotSolvable(X);
        }
    }
}

void fileToSudoku(Sudoku* sudoku, FILE* file, char* X, Mode mode){
    int total_size, num, row, column, numOfCells, cntFilledCell=0, cntErroneousCells=0;
    SudokuCell*** board = (SudokuCell***)malloc(sizeof(SudokuCell**));
    if (board == NULL){
        printMallocFailed();
    }

    scanRowAndColumn(file, X, &row, &column); /*find m & n*/
    if (row * column > 99 || row <=0 || column<=0){
        printLoadedFileFirstLineNotValid(X);
    }
    total_size = row * column;
    createEmptyBoard(board, total_size); /* for empty board*/

    /*scan cells and check if all valid and solvable*/
    scanCells(file, X, board, mode, total_size, &cntFilledCell);

    updateSudoku(sudoku, mode, board,row ,column, cntFilledCell);

}

void loadBoardFromPath(Sudoku* sudoku, char* X, Mode mode){

    FILE* file;
    int isClosed;

    file = fopen(X,"r");
    if (file == NULL){
        printOpenFileFailed(X);
    }

    fileToSudoku(sudoku, file,X, mode);
    /*createSudoku(sudoku, row, column, numOfCells);
    loadBoardFromPath(sudoku, X);*/

    fclose(file);
    if (feof(file)){
        printCloseFileFailed(X);
    }

    /* 'fclose' has faild* - maybe should be -1 @@@@@@@@@@@@@@@*/
    /*isClosed = fclose(file);
    if (isClosed == EOF){
        printCloseFileFailed(X);
    }*/

}

/*TODO: check about clause b. in this command - what should we do with the unsaved current game board*/
void solve(Sudoku* sudoku, char* X){
    loadBoardFromPath(sudoku,X,SOLVE);
}

/*TODO: check about clause d. in this command - what should we do with the unsaved current game board*/
void editWithPath(Sudoku* sudoku, char* X){
    loadBoardFromPath(sudoku,X,EDIT);
}

/*TODO: check about clause d. in this command - what should we do with the unsaved current game board*/
void editWithoutPath(Sudoku* sudoku){

    SudokuCell*** emptyBoard = (SudokuCell***)malloc(sizeof(SudokuCell**));
    if (emptyBoard == NULL){
        printMallocFailed();
    }
    createEmptyBoard(emptyBoard, 9); /* for empty 9X9 board*/
    updateSudoku(sudoku,EDIT, emptyBoard, 3, 3, 0);

}

void updateSudoku(Sudoku* sudoku, Mode mode, SudokuCell*** newCurrentState, int newRow, int newColumn, int newCntFilledCell){

    freeBoard(sudoku->currentState, sudoku->total_size);
    freeBoard(sudoku->solution, sudoku->total_size); /*check if we should calculate the new SOLUTION board* @@@@@@@@@@@@@@@@@@@@@@@@ */

    sudoku->currentState = newCurrentState;
    sudoku->row=newRow;
    sudoku->column=newColumn;
    sudoku->total_size = newRow * newColumn;
    sudoku->cntFilledCell = newCntFilledCell;

    freeList(sudoku->list);
    sudoku->list = getNewList();
    /*sudoku->markError remain the same*/
    sudoku->mode = mode;

    if (mode == SOLVE){
        findErroneousCells(sudoku);

    }
    else{
        sudoku->cntErroneousCells = 0;
    }

}

int isContainsValue(Sudoku* sudoku, int x, int y){
    return sudoku->currentState[x][y]!=0;
}
int isThereXEmptyCells(Sudoku* sudoku, int x){
    int total_cells = sudoku->total_size*sudoku->total_size;
    int empty_cells = total_cells - sudoku->cntFilledCell;
    if (empty_cells >= x){
        return 1;
    }
    return 0;
}

/*
 * @params - function receives pointer to the main Sudoku.
 *
 * The function checks if the board is filled according to the sudoku's filledCell counter.
 *
 * @return - 1 if filled, 0 otherwise.
 */
int isFilled(Sudoku* sudoku){
    int total_cells = sudoku->total_size*sudoku->total_size;
    if (sudoku->cntFilledCell==total_cells){
        return 1;
    }
    return 0;
}


int isErroneous(Sudoku* sudoku){
    if (sudoku->cntErroneousCells==0){ /*zero cells are erroneous*/
        return 0;
    }
    return 1;
}

SET_STATUS lastCellToBeFilled(Sudoku* sudoku){
    if (sudoku->mode==SOLVE && isFilled(sudoku)){ /*in Solve mode and last cell was filled*/
        if (isErroneous(sudoku)==1){ /*board is not valid*/
            /*we remain in Solve mode*/
            printSolutionIsErroneous();
            return UNSOLVED; /*full but not SOLVED*/
        }
        else{ /* isErroneous(sudoku)==0 -> board is valid*/
            printSolved();
            sudoku->mode=INIT;
            return SOLVED;
        }
    }
    return UNSOLVED;
}

void mark_errors(Sudoku* sudoku,int x){
    if (sudoku->mode != SOLVE){ /*not in Solve mode*/
        /*print appropriate massage*/
    }
    else{ /*in Solve mode*/
        sudoku->markErrors = x;
    }
}

void addMoveToArrMove(Move** arrMove, int* p_arrSize, int x, int y, int beforeValue, int afterValue, int beforeErroneous, int afterErroneous){
    Move* newMove;
    newMove = getNewMove(x, y, beforeValue, afterValue, beforeErroneous, afterErroneous);
    arrMove[*p_arrSize] = newMove;
    (*p_arrSize)++;
}

void valueValidationAmongNeighbors(SudokuCell*** board, int i, int j, int dig, Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells, int* p_cntNeighborsErroneous){
    int beforeErroneous, afterErroneous;
    if (!isFixed(board, i, j)) { /*cell isn't fixed -> should be mark as erroneous */
        beforeErroneous = board[i][j]->cnt_erroneous;

        if (neighborsType == NEIGHBORS_WITH_DIG){
            board[i][j]->cnt_erroneous--;
        }
        else{/*neighborsType == NEIGHBORS_WITH_Z || neighborsType == NEIGHBORS_WITH_LOAD_FILE)*/
            board[i][j]->cnt_erroneous++;
        }
        (*p_cntNeighborsErroneous)++;
        afterErroneous = board[i][j]->cnt_erroneous;
        updateSudokuCntErroneousCells(p_cntTotalErroneousCells, beforeErroneous, afterErroneous);

        if (neighborsType != NEIGHBORS_WITH_LOAD_FILE){
            addMoveToArrMove(arrMove, p_arrSize, i, j, dig, dig, beforeErroneous, afterErroneous);
        }
    }
}

void setCntNeighborsErroneous(SudokuCell*** board, int x, int y, NeighborsType neighborsType, int cntNeighborsErroneous){
    if (neighborsType == NEIGHBORS_WITH_DIG){
        board[x][y]->cnt_erroneous-=cntNeighborsErroneous;
    }
    else{/*neighborsType == NEIGHBORS_WITH_Z || neighborsType == NEIGHBORS_WITH_LOAD_FILE)*/
        board[x][y]->cnt_erroneous+=cntNeighborsErroneous;
    }
}

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
int isRowValid(SudokuCell*** board, int row, int column, int x, int y,  int value , Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells){
    int dig, i, isValid = 1, cntNeighborsErroneous=0, beforeErroneous, afterErroneous;
    for(i = 0;i < column;i++){
        dig = board[row][i]->digit;
        if (dig == value && (x != row || y != i)) { /*same digit but not same cell*/
            valueValidationAmongNeighbors(board, row, i, dig, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells, &cntNeighborsErroneous);
            isValid = 0;
        }
    }
    setCntNeighborsErroneous(board, x, y, neighborsType, cntNeighborsErroneous); /*updating <X,Y>'s cnt_erroneous*/

    return isValid;
}

int isColumnValid(SudokuCell*** board, int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells){
    int dig, i, isValid = 1, cntNeighborsErroneous=0, beforeErroneous, afterErroneous;
    for (i = 0; i < row; i++) {
        dig = board[i][column]->digit;
        if (dig == value && (x != i || y != column)){ /*same digit but not same cell*/

            valueValidationAmongNeighbors(board, i, column, dig, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells, &cntNeighborsErroneous);
            isValid = 0;
        }
    }
    setCntNeighborsErroneous(board, x, y, neighborsType, cntNeighborsErroneous); /*updating <X,Y>'s cnt_erroneous*/

    return isValid;
}

/*
 * @params - function receives Cell* head, the Sudoku row and column, and the required X and Y.
 *
 * The function finds the top-left cell in the block of cell[X][Y].
 */

void findHeadBlock(Cell* head,int row,int column, int x, int y){
    head->x= x - (x%row);
    head->y= y - (y%column);
}

int isBlockValid(SudokuCell*** board ,int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells){
    int dig, i, j, isValid = 1, cntNeighborsErroneous, beforeErroneous, afterErroneous;
    Cell* head=(Cell*)malloc(sizeof(Cell));
    if(head == NULL){
        printMallocFailed();
    }
    findHeadBlock(head,row, column,x,y);
    for (i=0;i<row;i++){
        for (j=0; j<column; j++){
            dig = board[i + head->x][j+head->y]->digit;
            if (dig == value && (x != i + head->x || y != j+head->y)) {

                valueValidationAmongNeighbors(board, i + head->x, j+head->y, dig, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells, &cntNeighborsErroneous);
                isValid = 0;
            }
        }
    }
    free(head);

    setCntNeighborsErroneous(board, x, y, neighborsType, cntNeighborsErroneous); /*updating <X,Y>'s cnt_erroneous*/

    return isValid;
}

int isValueValid(SudokuCell*** board, int row, int column, int x, int y, int value, Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells) {

    int r, c, b;
    r = isRowValid(board, x, row * column, x, y, value, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells);
    c = isColumnValid(board, row * column, y, x, y, value, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells);
    b = isBlockValid(board, row, column, x, y, value, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells);
    if (r == 1 && c == 1 && b == 1) {
        return 1;
    }
    return 0;
}

void updateSudokuCntErroneousCells(int* p_cnt, int beforeErroneous, int afterErroneous){

    if( beforeErroneous == 0 && afterErroneous > 0){
        (*p_cnt)++;
    }
    else if ( beforeErroneous > 0 && afterErroneous == 0){
        (*p_cnt)--;
    }
}

void neighborsWithLoadFile(Sudoku* sudoku, int x, int y, int dig){

    isValueValid(sudoku->currentState ,sudoku->row, sudoku->column, x, y, dig, NULL, NULL, NEIGHBORS_WITH_LOAD_FILE, &sudoku->cntErroneousCells);

}

void neighborsWithZ(Sudoku* sudoku, int x, int y, int dig, Move** arrMove, int* p_arrSize){

    isValueValid(sudoku->currentState ,sudoku->row, sudoku->column, x, y, dig, arrMove, p_arrSize, NEIGHBORS_WITH_Z, &sudoku->cntErroneousCells);

}

void neighborsWithDig(Sudoku* sudoku, int x, int y, int dig, Move** arrMove, int* p_arrSize){

    isValueValid(sudoku->currentState, sudoku->row, sudoku->column, x, y, dig, arrMove, p_arrSize, NEIGHBORS_WITH_DIG, &sudoku->cntErroneousCells);

}


void neighborsErroneous(Sudoku* sudoku, int x, int y, int dig, int z, Move** arrMove, int* p_arrSize) {

    neighborsWithDig(sudoku, x, y, dig, arrMove, p_arrSize);
    sudoku->currentState[x][y]->cnt_erroneous=0; /*reset the erroneous of the cell*/
    neighborsWithZ(sudoku, x, y, z, arrMove, p_arrSize);

}

/*count and mark all the erroneous cells*/
void findErroneousCells(Sudoku* sudoku){
    int i,j;
    for (i=0; i<sudoku->total_size; i++){
        for (j=0; j<sudoku->total_size; j++){
            neighborsWithLoadFile(sudoku, i, j, sudoku->currentState[i][j]->digit);
        }
    }
}

void setCell(Sudoku* sudoku, int x, int y, int z, Move** arrMove, int* p_arrSize){
    int beforeErroneous, afterErroneous;
    Move* newMove;
    int dig = sudoku->currentState[x][y]->digit;

    beforeErroneous = sudoku->currentState[x][y]->cnt_erroneous;
    neighborsErroneous(sudoku, x, y, dig, z, arrMove, p_arrSize);

    if (dig == 0){
        sudoku->cntFilledCell++;
    }
    if (z == 0){
        sudoku->cntFilledCell--;
        /*sudoku->currentState[x][y]->is_erroneous=0;*/
    }
    sudoku->currentState[x][y]->digit=z;

    /*if (sudoku->currentState[x][y]->cnt_erroneous > 0) {
        changedErroneous = 1;
    }*/
    afterErroneous = sudoku->currentState[x][y]->cnt_erroneous;
    updateSudokuCntErroneousCells(sudoku, beforeErroneous, afterErroneous);

    addMoveToArrMove(arrMove, p_arrSize, x, y, dig, z, beforeErroneous, afterErroneous);

    /*deleteDigitFromArr(sudoku->currentState, x,y, z);*/
    /*addArrMoveToList(sudoku, arrMove, *p_arrSize);*/
}

SET_STATUS set(Sudoku* sudoku, int x, int y, int z){
    int zValid, fixed, dig;
    int arrSize=0, total_cells = sudoku->total_size*sudoku->total_size;
    Move* newMove;
    Move** arrMove = (Move**)malloc(total_cells*(sizeof(Move*)));
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else{ /*in Solve/Edit mode*/
        if (arrMove == NULL){
            printMallocFailed();
        }
        fixed = isFixed(sudoku->currentState,x,y);
        if (fixed && sudoku->mode==SOLVE){ /*in Solve mode fixed cells can't be updated (unlike in Edit mode) @@@@@@@@@@@@@@@@@@@@@@@@@@@@ check if we should make all cells in EDIT non-fixed*/
            printFixed();
            return UNSOLVED;
        }
        /*arrive here when changing the cell is legal*/
        dig = sudoku->currentState[x][y]->digit;
        if (z != dig) {

            setCell(sudoku, x, y, z, arrMove, &arrSize);
            addArrMoveToList(sudoku, arrMove, arrSize);

            print_board(sudoku);
            return lastCellToBeFilled(sudoku);
        }
    }
    return UNSOLVED;
}

void print_board(Sudoku* sudoku){
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else{ /*in Solve/Edit mode*/
        printSudoku(sudoku);
    }
}
void validate(Sudoku* sudoku){
    int isSolvable;
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else if (isErroneous(sudoku)==1){ /*board is erroneous*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve/Edit mode AND not erroneous*/
        isSolvable = ILP_Validation(sudoku);
        if (isSolvable==0){ /*board is unsolvable*/
            /*print that the board is UNSOLVABLE*/
        }
        else{ /*board is solvable*/
            /*print that the board is SOLVABLE*/
        }
    }
}
void guess(Sudoku* sudoku, float x){
    if (sudoku->mode!=SOLVE){
        /*print appropriate massage*/
    }
    else if (isErroneous(sudoku)==1){ /*board is erroneous*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve mode AND not erroneous*/
        LP_Guesses(sudoku, x);
        fillCellsWithScoreX(sudoku,x);
    }
}

void backToOriginalState(SudokuCell*** originalBoard, SudokuCell*** invalidBoard, int total_size){

    copyBoardValues(originalBoard, invalidBoard, total_size); /*fromBoard, toBoard, size*/
}

void generate(Sudoku* sudoku, int x, int y){
    int isSolvable, legalValue;
    int cntNoSolution=0;
    SudokuCell*** tmpBoard = copyBoard(sudoku->currentState, sudoku->total_size); /*save the original board for cases that "fillXCells" or "ILP_Validation" will fail*/
    if (sudoku->mode!=EDIT){
        /*print appropriate massage*/
    }
    else if(isThereXEmptyCells(sudoku,x)==0){
        /*print that there are less then X empty cells*/
    }
    else{ /*in Edit mode AND there are X empty cells*/
        while (cntNoSolution<1000){
            legalValue = fillXCells(tmpBoard, x);
            if (legalValue==0){
                cntNoSolution++;
                backToOriginalState(sudoku->currentState,tmpBoard, sudoku->total_size);
                continue;
            }
            isSolvable = ILP_Validation(tmpBoard);
            if (isSolvable==0){
                cntNoSolution++;
                backToOriginalState(sudoku->currentState,tmpBoard, sudoku->total_size);
                continue;
            }
            /*arrive here when BOTH "fillXCells" and "ILP_Validation" were succeed*/
            break;
        }
        keepYCells(tmpBoard, y);
    }
    freeBoard(tmpBoard, sudoku->total_size);
}

int hasMoveToUndo(Sudoku* sudoku){
    return hasPrev(sudoku->list);
}

void setPointerToPreviousMove(Sudoku* sudoku){
    moveToPrev(sudoku->list);
}

/* command=UNDO if we want to redo the move.
 * command=REDO if we want to undo the move*/
void updateTheBoard(Sudoku* sudoku, Move* move, Command command){

    updateSudokuCntErroneousCells(&sudoku->cntErroneousCells,move->beforeErroneous, move->afterErroneous);
    if (command == UNDO){ /*undo the move*/
        sudoku->currentState[move->cell->x][move->cell->y]->digit=move->beforeValue;
        sudoku->currentState[move->cell->x][move->cell->y]->cnterroneous=move->beforeErroneous;
    }
    else if (command == REDO){ /*redo the move*/
        sudoku->currentState[move->cell->x][move->cell->y]->digit=move->afterValue;
        sudoku->currentState[move->cell->x][move->cell->y]->cnterroneous=move->afterErroneous;
    }
}

void undoMove(Sudoku* sudoku){
    int i;
    Move** currentArrMove = getCurrentMove(sudoku->list)->arrMove;
    int currentArrSize = getCurrentMove(sudoku->list)->arrSize;
    Move* currentMove;
    for (i = currentArrSize - 1; i>=0; i--){
        currentMove = currentArrMove[i];
        updateTheBoard(sudoku, currentMove ,UNDO);
        if (currentMove->afterValue != currentMove->beforeValue){ /*print only if the value of the cell was changed*/
            printChangeInBoard(currentMove->cell, currentMove->afterValue, currentMove->beforeValue); /*print the change that was made*/
        }
    }
    setPointerToPreviousMove(sudoku);
}
void undo(Sudoku* sudoku){
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else if (hasMoveToUndo(sudoku)==0){ /*there are no moves to undo*/
        /*print that there are no moves to undo*/
    }
    else { /*in Solve/Edit mode AND has move to undo*/
        undoMove(sudoku);
    }
}
void setPointerToNextMove(Sudoku* sudoku){
    moveToNext(sudoku->list);
}

int hasMoveToRedo(Sudoku* sudoku){
    return hasNext(sudoku->list);
}
void redoMove(Sudoku* sudoku){
    int i;
    Move** currentArrMove = getCurrentMove(sudoku->list)->arrMove;
    int currentArrSize = getCurrentMove(sudoku->list)->arrSize;
    Move* currentMove;
    setPointerToNextMove(sudoku);
    for (i = currentArrSize - 1; i>=0; i--){
        currentMove = currentArrMove[i];
        updateTheBoard(sudoku,currentMove,REDO);
        if (currentMove->afterValue != currentMove->beforeValue){ /*print only if the value of the cell was changed*/
            printChangeInBoard(currentMove->cell, currentMove->beforeValue, currentMove->afterValue); /*print the change that was made*/
        }
    }
}
void redo(Sudoku* sudoku){
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else if (hasMoveToRedo(sudoku)==0){ /*there are no moves to redo*/
        /*print that there are no moves to redo*/
    }
    else { /*in Solve/Edit mode AND has move to redo*/
        redoMove(sudoku);
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * should enter after 'redo' command*/


/* A function that checks if in EDIT mode AND cell <i,j> contains value
 * because according to orders - this cell marked as fixed in the saved file*/
int fixedInEdit(Sudoku* sudoku, int i, int j){

    if (sudoku->mode == EDIT && sudoku->currentState[i][j]->digit != 0){
        return 1;
    }
    return 0;
}

void checkIfWriteFailed(char *X, int isWrite){
    if (isWrite < 0){
        printWriteToFileFailed(X);
    }
}

void saveBoardInFile(Sudoku* sudoku, char* X){
    FILE* file;
    int isClosed, i, j, dig, fixed, total_size = sudoku->total_size;

    file = fopen(X,"w");
    if (file == NULL){
        printOpenFileFailed(X);
    }

    checkIfWriteFailed(X, fprintf(file, "%d %d\n", sudoku->column, sudoku->row)); /* write the first line: m n*/
    /*isWrite = fprintf(file, "%d %d\n", sudoku->column, sudoku->row);
    if (isWrite < 0){
        printWriteToFileFailed(X);
    }*/

    for (i=0; i < total_size; i++){
        for (j=0; j < total_size; j++){
            dig = sudoku->currentState[i][j]->digit;
            fixed = sudoku->currentState[i][j]->is_fixed;
            if (fixed==1 || fixedInEdit(sudoku, i, j)){ /*cell is fixed*/
                checkIfWriteFailed(X, fprintf(file, "%d.", dig));
            }
            else{
                checkIfWriteFailed(X, fprintf(file, "%d", dig));
            }
            if (j == total_size-1){ /*end of row*/
                checkIfWriteFailed(X, fprintf(file, "\n"));
            }
            else{
                checkIfWriteFailed(X, fprintf(file, " "));
            }
        }
    }

    isClosed = fclose(file);
    if (isClosed == EOF){ /* 'fclose' has faild* - maybe should be -1 @@@@@@@@@@@@@@@*/
        printCloseFileFailed(X);
    }
}
void save(Sudoku* sudoku, char* X) {
    if (sudoku->mode == INIT) { /* in Init mode*/
        /*print appropriate massage*/
    } /*in Solve/Edit mode*/
    if (sudoku->mode == EDIT){
        if (isErroneous(sudoku)==1) { /* 1 if board is erroneous*/
            /*print an error message and the command is not executed*/
        }
        if (ILP_Validation(sudoku)==0){ /* 0 if board is unsolvable*/
            /*print that the board is UNSOLVABLE*/
        }
    }
    /*now clear for saving the file*/
    saveBoardInFile(sudoku,X);
}

void hint(Sudoku* sudoku, int x, int y){
    int isSolvable;
    if (sudoku->mode!=SOLVE){
        /*print appropriate massage*/
    }
    else if (isErroneous(sudoku)==1 || isFixed(sudoku,x,y)==1 || isContainsValue(sudoku,x,y)==1){ /*board is erroneous OR cell is fixed OR contains a value*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve mode AND all valid*/
        isSolvable = ILP_Validation(sudoku);
        if (isSolvable==0){ /*board is unsolvable*/
            /*print that the board is UNSOLVABLE*/
        }
        else{ /*board is solvable*/
            /*print the value of cell <X,Y> found by the ILP solution*/
        }
    }
}
void guess_hint(Sudoku* sudoku, int x, int y){
    int isSolvable;
    if (sudoku->mode!=SOLVE){
        /*print appropriate massage*/
    }
    else if (isErroneous(sudoku)==1 || isFixed(sudoku,x,y)==1 || isContainsValue(sudoku,x,y)==1){ /*board is erroneous OR cell is fixed OR contains a value*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve mode AND all valid*/
        isSolvable = LP_Validation(sudoku);
        if (isSolvable==0){ /*board is unsolvable*/
            /*print that the board is UNSOLVABLE*/
        }
        else{ /*board is solvable*/
            /*print all the legal values of cell <X,Y> and their scores (score greater then 0)*/
        }
    }
}
void num_solutions(Sudoku* sudoku){
    int numOfSolution, total_size = sudoku->total_size;
    Cell* firstEmptyCell;
    SudokuCell*** fixedBoard = NULL;
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else if (isErroneous(sudoku)==1){ /*board is erroneous*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve mode AND not erroneous*/
        firstEmptyCell = (Cell*)malloc(sizeof(Cell));
        if(firstEmptyCell == NULL){
            printMallocFailed();
        }
        createEmptyBoard(fixedBoard,total_size);
        currentStateToFixed(sudoku, fixedBoard, total_size);/*copy the sudoku.currentState to fixedBoard*/
        findNextEmptyCell(fixedBoard, total_size,firstEmptyCell,0,0); /*search for the first empty cell in the board*/
        numOfSolution = exhaustiveBacktracking(sudoku, firstEmptyCell->x, firstEmptyCell->y);
        printNumOfSolution(numOfSolution);
        freeBoard(fixedBoard,total_size);
        free(firstEmptyCell);
    }
}

void markSingleLegalValue(Sudoku* sudoku){
    int i,j, total_size = sudoku->total_size;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            sudoku->currentState[i][j]->numOfOptionalDigits=sudoku->total_size;
            findThePossibleArray(sudoku->currentState,sudoku->row,sudoku->column, i, j);
        }
    }
}

int hasSingleLegalValue(Sudoku* sudoku, int i, int j){
    if (sudoku->currentState[i][j]->numOfOptionalDigits == 1){
        return 1;
    }
    return 0;
}

/*void updateObviousCell(Sudoku* sudoku, int i, int j, Move** arrMove, int arrSize){
    int dig = sudoku->currentState[i][j]->optionalDigits[0];

    setCell(sudoku, i, j, dig, arrMove, &arrSize);

    deleteDigitFromArr(sudoku->currentState, i,j, dig);
    sudoku->currentState[i][j]->hasSingleLegalValue=0;
}*/

/*void addOneMoveToList(Sudoku *sudoku, int x, int y, int value, int z){
    Move* newMove = getNewMove(x,y,value,z);
    Move** arrMove = (Move**)malloc(sizeof(Move*));
    if (arrMove == NULL){
        printMallocFailed();
    }
    arrMove[0]=newMove;
    insertAtTail(sudoku->list, newMove,1);
}*/

void addArrMoveToList(Sudoku *sudoku, Move** arrMove, int arrSize){
    insertAtTail(sudoku->list, arrMove, arrSize);
}


/*TODO: need to deal with situation that 2 obvious cells become erroneous*/
void fillObviousValues(Sudoku* sudoku){
    int arrSize=0, dig;
    int total_cells = sudoku->total_size*sudoku->total_size;
    Move** arrMove = (Move**)malloc(total_cells*(sizeof(Move*)));
    if (arrMove == NULL){
        printMallocFailed();
    }
    markSingleLegalValue(sudoku);
    int i,j, total_size = sudoku->total_size;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            if (hasSingleLegalValue(sudoku,i,j)){

                dig = sudoku->currentState[i][j]->optionalDigits[0];
                setCell(sudoku, i, j, dig, arrMove, &arrSize);

               /* updateObviousCell(sudoku,i,j, arrMove, arrSize);*/
                arrSize++;
            }
        }
    }
    addArrMoveToList(sudoku, arrMove, arrSize);
}

void autofill(Sudoku* sudoku){
    if (sudoku->mode!=SOLVE){
        /*print appropriate massage*/
    }
    else if (isErroneous(sudoku)==1){ /*board is erroneous*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve mode AND not erroneous*/
        fillObviousValues(sudoku);
    }
}
void undoAllMoves(Sudoku* sudoku){
    while (hasMoveToUndo(sudoku)==1){
        undoMove(sudoku);
    }
}

void reset(Sudoku* sudoku){
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else { /*in Solve/Edit mode*/
        undoAllMoves(sudoku);
        print_board(sudoku);
    }
}
void exitProgram(Sudoku* sudoku){
    freeMemory(sudoku);
    printExitMessage();
}


/****************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************/

/*
 * @params - function receives pointer to the main Sudoku and the cell's indexes.
 *
 * The function check if the cell is fixed.
 *
 * @return - 1 if fixed, 0 otherwise.
 */
int isFixed(SudokuCell*** board, int x, int y) {
    return board[x][y]->is_fixed;
}

/*
 * @params - function receives pointer to the main Sudoku.
 *
 * The function checks if the board is solved according to the sudoku's filledCell counter.
 *
 * @return - 1 if solved, 0 otherwise.
 */
/*int isSolved(Sudoku* sudoku){
    int total_cells = sudoku->total_size*sudoku->total_size;
    if (sudoku->cntFilledCell==total_cells){
        return 1;
    }
    return 0;
}*/


