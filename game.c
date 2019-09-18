#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "main_aux.h"

int _validate_hint(Sudoku* sudoku, int x, int y){
    if(isErroneous(sudoku)==1){
        printErroneousBoard();
        return 0;
    }
    else if (isFixed(sudoku->currentState,x,y)==1){
        printFixed();
        return 0;
    }
    else if(isContainsValue(sudoku,x,y)==1){
        printContainsValue();
        return 0;
    }
    return 1;
}

int _validate_gurobi(int isSolvable){
    if ( isSolvable != 1 ){
        if (isSolvable == 0){
            printUnsolvableBoard();
            return 0;
        }
        else if (isSolvable == -1){
            printGurobiFailed();
            return 0;
        }
    }
    return 1;
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*loading files*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


/*
 * @params - function receives path, the returned value from 'fscanf' and an appropriate error function.
 *
 * The function checks if the returned value from 'fscanf' is EOF or valid\invalid char.
 */
int check_EOF_and_invalid_scan(char* path, int scan, void (*errorFunc)(char*)){
    if (scan == EOF){ /*checkEOF*/
        printLoadedFileEOF(path);
        return 0;
    }
    if (scan!=1){ /*invalid char was scanned*/
        (*errorFunc)(path);
        return 0;
    }
    return 1;
}

/*
 * @params - function receives a file, its path, and pointers to row & column variables.
 *
 * The function tries to scan the first two digits in the file - row and column, and stores them in their variables.
 */
int scanRowAndColumn(FILE* file, char* path, int* p_row, int* p_column){
    int scan, isValid;
    scan = fscanf(file,"%d", p_column); /*scanning column*/
    isValid = check_EOF_and_invalid_scan(path, scan, printLoadedFileFirstLineNotValid);
    if (isValid == 0){
        return 0;
    }
    scan = fscanf(file,"%d", p_row); /*scanning row*/
    isValid = check_EOF_and_invalid_scan(path, scan, printLoadedFileFirstLineNotValid);
    if (isValid == 0){
        return 0;
    }
    return 1;
}

/*
 * @params - function receives a file.
 *
 * The function checks if there are only whitespaces left in the file.
 */
int areOnlyWhitespacesLeft(FILE* file){
    int c;
    while ((c = fgetc(file)) != EOF){
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n' ){
            return 0;
        }
    }
    return 1;
}

/*
 * @params - function receives a file, its path, and pointers to row & column variables.
 *
 * The function tries to scan the first two digits in the file - row and column, and stores them in their variables.
 */
int scanCells(FILE* file, char* path, SudokuCell*** board, Mode mode, int total_size, int* p_cntFilledCell){
    int i,j, scan, dig, fixed, isValid;
    char c;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            scan = fscanf(file,"%d", &dig); /*scan the dig for cell <i,j> */
            isValid = check_EOF_and_invalid_scan(path, scan, printLoadedFileCellNotValid);
            if (isValid == 0){
                return 0;
            }
            if ((c=fgetc(file)) != EOF){ /*scan whether the cell <i,j> is fixed*/
                if (c=='.'){
                    fixed = 1;
                }
                else if (c== ' ' || c == '\t' || c == '\r' || c == '\n'){
                    fixed = 0;
                }
                else{
                    printLoadedFileCellNotValid(path);
                    return 0;
                }
            }
            else{ /* arrive to EOF after scanning a dig */
                if (j == total_size-1 && i == total_size-1){ /*EOF but we scanned the last cell*/
                    fixed = 0;
                }
                else{ /*EOF before the last cell*/
                    printLoadedFileEOF(path);
                    return 0;
                }
            }
            if (dig > total_size || dig < 0){ /* if dig is in valid range */
                printLoadedFileCellNotValid(path);
                return 0;
            }
            if ( (dig == 0 && fixed ==1) || (mode == EDIT && fixed == 0 && dig !=0 ) ){ /*if the cell characteristics' is valid */
                printLoadedFileCellNotValid(path);
                return 0;
            }
            if (mode == EDIT){ /*in EDIT mode, no cell will considered fixed*/
                fixed = 0;
            }
            board[i][j]->digit = dig;
            board[i][j]->is_fixed = fixed;
            if (dig != 0){
                (*p_cntFilledCell)++; /* increments the 'cntFilledCell' counter */
            }
        }
    }
    isValid = areOnlyWhitespacesLeft(file); /*check if there are only whitespaces left in the file*/
    if (isValid == 0){
        printLoadedFileLengthNotValid(path); /* file is too long */
    }
    return isValid;
}

/*
 * @params - function receives a Sudoku pointer, a file's path and the new Sudoku's parameters.
 *
 * The function checks the validation and the erroneous of the loaded board, with contact to the sudoku's mode.
 */

int check_validation_of_loaded_board(Sudoku *tmpSudoku, char *path, Mode newMode, int editWithoutPath, SudokuCell ***newCurrentState, int newRow, int newColumn, int newCntFilledCell){
    int isValid;
    tmpSudoku->mode=newMode;
    tmpSudoku->currentState=newCurrentState;
    tmpSudoku->row=newRow;
    tmpSudoku->column=newColumn;
    tmpSudoku->total_size=newRow*newColumn;
    tmpSudoku->cntFilledCell=newCntFilledCell;
    tmpSudoku->cntErroneousCells=0;

    if (editWithoutPath == 0){
        findErroneousCellsForLoadedBoard(tmpSudoku->currentState, tmpSudoku->total_size, tmpSudoku->row,tmpSudoku->column, &tmpSudoku->cntErroneousCells);

        if (tmpSudoku->mode == SOLVE){
            isValid = isErroneousBetween2FixedCells(tmpSudoku); /*checks for erroneous only between fixed cells*/
            if (isValid == 0){ /* means that there are two fixed cells that make each other erroneous*/
                printLoadedFileNotSolvable(path);
                freeBoard(newCurrentState, tmpSudoku->total_size);
                return 0;
            }
            if (isFilled(tmpSudoku) && !isErroneous(tmpSudoku)){
                printLoadedFileFilledAndSolved();
                print_board(tmpSudoku);
                freeBoard(newCurrentState, tmpSudoku->total_size);
                tmpSudoku->mode=INIT;
                return 0; /*TODO: check if this good */
            }
        }
        else if (tmpSudoku->mode == EDIT){ /* TODO: check if this is necessary - in EDIT mode, the loaded board should be solvable*/
            /* TODO: message from xaim to Yoni - I think that in EDIT mode we should load the board anyway because all of its cells are not fixed - but we should check this*/
            /* TODO: VALIDATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
        }
    }
    return 1;
}

/*
 * @params - function receives a Sudoku pointer, the file's path, and the new Sudoku's parameters (new mode, board, row, column, and counter of the filled cell).
 *
 * The function update the new parameters in the exists Sudoku struct, in subject to the validation of the board.
 */
/*TODO: not finished ! */
int updateSudoku(Sudoku* sudoku, char* path, Mode newMode, int editWithoutPath, SudokuCell*** newCurrentState, int newRow, int newColumn, int newCntFilledCell){

    int isValid;
    Sudoku* tmpSudoku = (Sudoku*)malloc(sizeof(Sudoku));
    if (tmpSudoku == NULL){
        printMallocFailedAndExit();
    }
    isValid = check_validation_of_loaded_board(tmpSudoku, path, newMode, editWithoutPath, newCurrentState, newRow,
                                               newColumn, newCntFilledCell); /*check if the new parameters are valid*/ /* TODO: need to work on this function */
    if (isValid == 0){ /* the loaded board is not valid. we return to the previous state*/
        free(tmpSudoku);
        return 0;
    }

    /*now we are free to update the new Sudoku's parameters*/
    if (sudoku->justStarted == 0){
        /*TODO: check what to do if we are in INIT mode -> we dont have a board to free*/
        freeBoard(sudoku->currentState, sudoku->total_size);
        freeList(sudoku->list);
    }
    if (sudoku->justStarted == 1){ /*there are no board or list to free */
        sudoku->justStarted = 0;
    }

    /*freeBoard(sudoku->solution, sudoku->total_size); check if we should calculate the new SOLUTION board* @@@@@@@@@@@@@@@@@@@@@@@@ */
    sudoku->currentState = tmpSudoku->currentState;
    sudoku->row=tmpSudoku->row;
    sudoku->column=tmpSudoku->column;
    sudoku->total_size = tmpSudoku->total_size;
    sudoku->cntFilledCell = tmpSudoku->cntFilledCell;
    sudoku->cntErroneousCells=tmpSudoku->cntErroneousCells;
    sudoku->mode = tmpSudoku->mode;

    sudoku->list = getNewList();
    /*sudoku->markError remain the same*/


    free(tmpSudoku);
    return 1;
}

/*
 * @params - function receives a Sudoku pointer, a file and the file's path, and the new mode.
 *
 * The function uses auxiliary functions to scan the board from loaded file, and to check the validation of the board, and update the exists Sudoku struct (if everything went well).
 */
int fileToSudoku(Sudoku* sudoku, FILE* file, char* X, Mode mode){
    int total_size, row, column, cntFilledCell=0, isValid;
    SudokuCell*** board;
    /* TODO: check that the old board doesn't disappear if the scan was failed*/
    isValid = scanRowAndColumn(file, X, &row, &column); /*find m & n*/
    if (isValid == 0){
        return 0;
    }
    if (row * column > 99 || row <=0 || column<=0){
        printLoadedFileFirstLineNotValid(X);
        return 0;
    }
    total_size = row * column;
    board = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if (board == NULL){
        printMallocFailedAndExit();
    }
    createEmptyBoard(board, total_size); /* for empty board*/
    /*scan cells and check if all valid and solvable*/
    isValid = scanCells(file, X, board, mode, total_size, &cntFilledCell);
    if (isValid == 0){
        freeBoard(board,total_size);
        return 0;
    }
    return updateSudoku(sudoku, X, mode, 0, board,row ,column, cntFilledCell);
}

/*
 * @params - function receives a Sudoku pointer, a path to a file, and the new mode.
 *
 * The function uses auxiliary functions to open & load & close the file.
 */
int loadBoardFromPath(Sudoku *sudoku, char *X, Mode mode){
    FILE* file;
    int isValid = 0, isClosed;
    file = fopen(X,"r");
    if (file == NULL){
        printOpenFileFailed(X);
        return 0;
    }
    isValid = fileToSudoku(sudoku, file, X, mode);
    isClosed = fclose(file);
    if (isClosed == -1){
        printCloseFileFailed(X);
        return 0;
    }
    return isValid;
}

/*TODO: check about clause d. in this command - what should we do with the unsaved current game board*/
/*
 * @params - function receives a Sudoku pointer, a path to a file.
 *
 * The function uses 'loadBoardFromPath' function (an auxiliary func) to load the file, and switch to SOLVE mode.
 */
void solve(Sudoku* sudoku, char* X){
    int isValid;
    isValid = loadBoardFromPath(sudoku, X, SOLVE);
    if (isValid == 1){
        print_board(sudoku);
    }
}

/*TODO: check about clause d. in this command - what should we do with the unsaved current game board*/
/*
 * @params - function receives a Sudoku pointer, a path to a file.
 *
 * The function uses 'loadBoardFromPath' function (an auxiliary func) to load the file, and switch to EDIT mode.
 */
void editWithPath(Sudoku* sudoku, char* X){
    int isValid;
    isValid = loadBoardFromPath(sudoku, X, EDIT);
    if (isValid == 1){
        print_board(sudoku);
    }
    /*else{
        printEditFileFailed(X);
    }*/
}

/*TODO: check about clause d. in this command - what should we do with the unsaved current game board*/
/*
 * @params - function receives a Sudoku pointer.
 *
 * The function switch to EDIT mode with an empty 9X9 board.
 */
void editWithoutPath(Sudoku* sudoku){
    SudokuCell ***emptyBoard = (SudokuCell ***) malloc(9*sizeof(SudokuCell **));
    if (emptyBoard == NULL) {
        printMallocFailedAndExit();
    }
    createEmptyBoard(emptyBoard, 9); /* for empty 9X9 board*/
    updateSudoku(sudoku, NULL, EDIT, 1, emptyBoard, 3, 3, 0);
    print_board(sudoku);
}

/*
 * @params - function receives a Sudoku pointer and char* (could be a path to a file, or NULL).
 *
 * The function calls 'editWithoutPath' if the char* is NULL, else calls to 'editWithPath'.
 */
void edit(Sudoku* sudoku, char* X){
    if (X == NULL){
        editWithoutPath(sudoku);
    }
    else{
        editWithPath(sudoku, X);
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*Erroneous checking*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


/*
 * @params - function receives a Sudoku pointer.
 *
 * The function checks if the board is erroneous, and returns accordingly.
 */
int isErroneous(Sudoku* sudoku){
    if (sudoku->cntErroneousCells==0){ /*zero cells are erroneous*/
        return 0;
    }
    return 1;
}

/*
 * @params - function receives a pointer to 'SudokuCell' board and cell indexes.
 *
 * The function checks if the cell <x,y> is erroneous, and returns accordingly.
 */
int isCellErroneous(SudokuCell*** board, int x, int y){
    return board[x][y]->cnt_erroneous;
}

/*
 * @params - function receives pointer to 'SudokuCell' board, total_size, row, column and pointer to the counter 'cntErroneousCells'.
 *
 * The function counts and marks all the erroneous cells of the new loaded board - by calling 'neighborsLoadFile' function.
 */
void findErroneousCellsForLoadedBoard(SudokuCell ***board, int total_size, int row, int column, int *p_cntErroneousCells){
    int i,j;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            neighborsLoadFile(board, i, j, board[i][j]->digit, row, column, p_cntErroneousCells);
            /*if (board[i][j]->cnt_erroneous>0){
                printf("%d %d with %d\n", i,j, board[i][j]->cnt_erroneous);
            }*/
        }
    }
    cutErroneousInHalf(board, total_size);
    (*p_cntErroneousCells) *=2;
}

/*
 * @params - function receives a Sudoku pointer.
 *
 * The function creates a tmp board that includes only the fixed cell in the original board, checks if there is a collision between 2 fixed cells and returns accordingly.
 */

int isErroneousBetween2FixedCells(Sudoku* sudoku){

    SudokuCell*** fixedBoard = copyBoard(sudoku->currentState, sudoku->total_size, 1); /* 1 for 'copyFixedCellsOnly' */
    int i,j, isValid=1,cntErroneousCells;


    cntErroneousCells = sudoku->cntErroneousCells;
    findErroneousCellsForLoadedBoard(fixedBoard, sudoku->total_size, sudoku->row, sudoku->column,
                                     &sudoku->cntErroneousCells);
    sudoku->cntErroneousCells = cntErroneousCells;

    for (i=0; i<sudoku->total_size; i++) {
        for (j = 0; j < sudoku->total_size; j++) {
            if (isFixed(fixedBoard, i, j) && isCellErroneous(fixedBoard, i, j)) {
                isValid = 0;
            }
        }
    }

    freeBoard(fixedBoard, sudoku->total_size);
    return isValid;
}


/*
 * @params - function receives pointer to 'SudokuCell' board, cell indexes, a 'NeighborsType' and a counter 'cntNeighborsErroneous'.
 *
 * The function sets the erroneousness of the cell <x,y> according to the NeighborsType and the counter 'CntNeighborsErroneous'
 */
void setCntNeighborsErroneous(SudokuCell*** board, int x, int y, NeighborsType neighborsType, int cntNeighborsErroneous){
    if (neighborsType == NEIGHBORS_FROM_DIG){
        board[x][y]->cnt_erroneous-=cntNeighborsErroneous;
    }
    else if(neighborsType == NEIGHBORS_TO_DIG || neighborsType == NEIGHBORS_LOAD_FILE){
        board[x][y]->cnt_erroneous+=cntNeighborsErroneous;
    }
}

/*
 * @params - function receives pointer to the Sudoku counter 'cntErroneousCells', and 2 variables that include the erroneous 'before' and 'after' the change in the cell.
 *
 * The function update the value which stored in the 'cntErroneousCells' pointer, according to the 'before' and 'after' values.
 */
void updateSudokuCntErroneousCells(int* p_cntErroneousCells, int beforeErroneous, int afterErroneous){
    if( beforeErroneous == 0 && afterErroneous > 0){
        (*p_cntErroneousCells)++;
    }
    else if ( beforeErroneous > 0 && afterErroneous == 0){
        (*p_cntErroneousCells)--;
    }
}

/*
 * @params - function receives a Sudoku pointer, cell indexes, the previous value of cell <x,y>, the new value, and the array moves (and its size).
 *
 * The function calls the auxiliary functions 'neighborsFromDig' and 'neighborsToDig' (if necessary) to update the erroneousness of cell <x,y>. */
void neighborsErroneous(Sudoku* sudoku, int x, int y, int dig, int z, Move** arrMove, int* p_arrSize) {
    if (dig!=0) {
        neighborsFromDig(sudoku, x, y, dig, arrMove, p_arrSize);
    }
    sudoku->currentState[x][y]->cnt_erroneous=0; /*reset the erroneous of the cell*/
    if (z!=0){
        neighborsToDig(sudoku, x, y, z, arrMove, p_arrSize);
    }
}

/*
 * @params - function receives pointer to 'SudokuCell' board and its total_size.
 *
 * The function "cuts" the erroneous counter for each cell by half. An auxiliary function for 'findErroneousCellsForLoadedBoard'.
 */
void cutErroneousInHalf(SudokuCell*** board, int total_size){
    int i,j;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            board[i][j]->cnt_erroneous /=2;
        }
    }
}

/*
* @params - function receives a pointer to 'SudokuCell' board, cell indexes <x,y>, another cell <i,j> with the same value, the value, the array moves (and its size), a 'NeighborsType' and pointers to the counters 'cntErroneousCells' and 'cntNeighborsErroneous'.
*
* The function updates the erroneousness of cells <x,y> (by cntNeighborsErroneous) and <i,j>, and the counter cntErroneousCells of the Sudoku, according to the 'NeighborsType'.
*/
void erroneousAmongNeighbors(SudokuCell ***board, int x, int y, int i, int j, int dig, Move **arrMove, int *p_arrSize, NeighborsType neighborsType, int *p_cntTotalErroneousCells, int *p_cntNeighborsErroneous){
    int beforeErroneous, afterErroneous;
    /*TODO: check why it had a !isFixed(board, i, j)*/
    if (neighborsType != NEIGHBORS_POSSIBLE_ARRAY) { /*cell isn't fixed -> should be mark as erroneous */
        if ( isFixed(board, i, j) && isFixed(board, x, y) && neighborsType == NEIGHBORS_LOAD_FILE ) {
            board[i][j]->cnt_erroneous++;
        }
        else{
            beforeErroneous = board[i][j]->cnt_erroneous;
            if (neighborsType == NEIGHBORS_FROM_DIG){
                board[i][j]->cnt_erroneous--;
            }
            else if (neighborsType == NEIGHBORS_TO_DIG || neighborsType == NEIGHBORS_LOAD_FILE){
                board[i][j]->cnt_erroneous++;
            }
            (*p_cntNeighborsErroneous)++;
            afterErroneous = board[i][j]->cnt_erroneous;
            updateSudokuCntErroneousCells(p_cntTotalErroneousCells, beforeErroneous, afterErroneous);
            if (neighborsType != NEIGHBORS_LOAD_FILE){
                addMoveToArrMoveAndIncrementSize(arrMove, p_arrSize, i, j, dig, dig, beforeErroneous, afterErroneous);
            }
        }
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Values validation */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/*
 * @params - function receives SudokuCell*** board, the required row and column for the specific check, the cell indexes and its checked value, the array moves (and its size), a 'NeighborsType' and pointer to the counter 'cntTotalErroneousCells'.
 *
 * The function checks if the value appears in the required row (if the row will stay valid).
 * if not, calls the the auxiliary functions 'erroneousAmongNeighbors' and 'setCntNeighborsErroneous' to update the erroneousness of cell <x,y>, <i,j> and 'cntTotalErroneousCells'.
 *
 * @return - 1 if valid, 0 otherwise.
 */
int isRowValid(SudokuCell*** board, int row, int column, int x, int y,  int value , Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells){
    int dig, i, isValid = 1, cntNeighborsErroneous=0;
    for(i = 0;i < column;i++){
        dig = board[row][i]->digit;
        if (dig != 0 && dig == value && (x != row || y != i)) { /*same digit but not same cell*/
            erroneousAmongNeighbors(board, x, y, row, i, dig, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells, &cntNeighborsErroneous);
            isValid = 0;
        }
    }
    setCntNeighborsErroneous(board, x, y, neighborsType, cntNeighborsErroneous); /*updating <X,Y>'s cnt_erroneous*/
    return isValid;
}

/*
 * @params - function receives SudokuCell*** board, the required row and column for the specific check, the cell indexes and its checked value, the array moves (and its size), a 'NeighborsType' and pointer to the counter 'cntTotalErroneousCells'.
 *
 * The function checks if the value appears in the required column (if the column will stay valid).
 * if not, calls the the auxiliary functions 'erroneousAmongNeighbors' and 'setCntNeighborsErroneous' to update the erroneousness of cell <x,y>, <i,j> and 'cntTotalErroneousCells'.
 *
 * @return - 1 if valid, 0 otherwise.
 */
int isColumnValid(SudokuCell*** board, int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells){
    int dig, i, isValid = 1, cntNeighborsErroneous=0;
    for (i = 0; i < row; i++) {
        dig = board[i][column]->digit;
        if (dig != 0 && dig == value && (x != i || y != column)){ /*same digit but not same cell*/
            erroneousAmongNeighbors(board, x, y, i, column, dig, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells, &cntNeighborsErroneous);
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

/*
 * @params - function receives SudokuCell*** board, the required row and column for the specific check, the cell indexes and its checked value, the array moves (and its size), a 'NeighborsType' and pointer to the counter 'cntTotalErroneousCells'.
 *
 * The function search for the most top-left cell in the block of input cell by calling "findHeadBlock".
 * Then, the function checks if the value appears in the required block (if the block will stay valid).
 * if not, calls the the auxiliary functions 'erroneousAmongNeighbors' and 'setCntNeighborsErroneous' to update the erroneousness of cell <x,y>, <i,j> and 'cntTotalErroneousCells'.
 *
 * @return - 1 if valid, 0 otherwise.
 */
int isBlockValid(SudokuCell*** board ,int row, int column, int x, int y, int value ,Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells){
    int dig, i, j, isValid = 1, cntNeighborsErroneous=0;
    Cell* head=(Cell*)malloc(sizeof(Cell));
    if(head == NULL){
        printMallocFailedAndExit();
    }
    findHeadBlock(head,row, column,x,y);
    for (i=0;i<row;i++){
        for (j=0; j<column; j++){
            dig = board[i + head->x][j+head->y]->digit;
            if (dig != 0 && dig == value && (x != i + head->x || y != j+head->y)) {
                erroneousAmongNeighbors(board, x, y, i + head->x, j + head->y, dig, arrMove, p_arrSize, neighborsType, p_cntTotalErroneousCells, &cntNeighborsErroneous);
                isValid = 0;
            }
        }
    }
    free(head);
    setCntNeighborsErroneous(board, x, y, neighborsType, cntNeighborsErroneous); /*updating <X,Y>'s cnt_erroneous*/
    return isValid;
}

/*
 * @params - function receives SudokuCell*** board, the required row and column for the specific check, the cell indexes and its checked value, the array moves (and its size), a 'NeighborsType' and pointer to the counter 'cntTotalErroneousCells'.
 *
 * The function checks what are the consequences (erroneousness and et cetera) of setting the value the wanted cell
 * by scanning it's row, column and block with calls to the appropriate auxiliary functions.
 *
 * @return - 1 if valid, 0 otherwise.
 */
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

/*
 * @params - function receives SudokuCell*** board, the row and column of the Sudoku, the cell indexes and its checked value and pointer to the counter 'cntTotalErroneousCells'.
 *
 * The function calls 'isValueValid' function (with the NeighborsType 'NEIGHBORS_LOAD_FILE') that checks the value validation and the future erroneousness of the <x,y>'s neighbors (and update it).
 *
 */
void neighborsLoadFile(SudokuCell*** board, int x, int y, int dig, int row, int column, int* p_cntErroneousCells){

    isValueValid(board ,row, column, x, y, dig, NULL, NULL, NEIGHBORS_LOAD_FILE, p_cntErroneousCells);

}

/*
 * @params - function receives a Sudoku pointer, the row and column of the Sudoku, the cell indexes and its checked value, the array moves (and its size).
 *
 * The function calls 'isValueValid' function (with the NeighborsType 'NEIGHBORS_TO_DIG') that checks the value validation and the future erroneousness of the <x,y>'s neighbors (and update it).
 *
 */
void neighborsToDig(Sudoku *sudoku, int x, int y, int dig, Move **arrMove, int *p_arrSize){

    isValueValid(sudoku->currentState ,sudoku->row, sudoku->column, x, y, dig, arrMove, p_arrSize, NEIGHBORS_TO_DIG, &sudoku->cntErroneousCells);

}

/*
 * @params - function receives a Sudoku pointer, the row and column of the Sudoku, the cell indexes and its checked value, the array moves (and its size).
 *
 * The function calls 'isValueValid' function (with the NeighborsType 'NEIGHBORS_FROM_DIG') that checks the value validation and the future erroneousness of the <x,y>'s neighbors (and update it).
 *
 */
void neighborsFromDig(Sudoku *sudoku, int x, int y, int dig, Move **arrMove, int *p_arrSize){

    isValueValid(sudoku->currentState, sudoku->row, sudoku->column, x, y, dig, arrMove, p_arrSize, NEIGHBORS_FROM_DIG, &sudoku->cntErroneousCells);

}

/*
 * @params - function receives SudokuCell*** board, the row and column of the Sudoku, the cell indexes and its checked value.
 *
 * The function calls 'isValueValid' function (with the NeighborsType 'NEIGHBORS_POSSIBLE_ARRAY') that checks the value validation.
 *
 * @return - 1 if valid, 0 otherwise.
 */
int neighborsPossibleArr(SudokuCell*** board, int row, int column, int x, int y, int dig){

    return isValueValid(board ,row, column, x, y, dig, NULL, NULL, NEIGHBORS_POSSIBLE_ARRAY, NULL);

}


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Board and cells actions and characteristics*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int isContainsValue(Sudoku* sudoku, int x, int y){
    return sudoku->currentState[x][y]->digit!=0;
}

int isFilled(Sudoku* sudoku){
    int total_cells = sudoku->total_size*sudoku->total_size;
    if (sudoku->cntFilledCell==total_cells){
        return 1;
    }
    return 0;
}

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

/* A function that checks if in EDIT mode AND cell <i,j> contains value
 * because according to orders - this cell marked as fixed in the saved file*/
int isFixedInEdit(Sudoku *sudoku, int i, int j){

    if (sudoku->mode == EDIT && sudoku->currentState[i][j]->digit != 0){
        return 1;
    }
    return 0;
}

void lastCellToBeFilled(Sudoku* sudoku){
    if (sudoku->mode==SOLVE && isFilled(sudoku)){ /*in Solve mode and last cell was filled*/
        if (isErroneous(sudoku)==1){ /*board is not valid*/
            /*we remain in Solve mode*/
            printSolutionIsErroneous();
            /*full but not SOLVED*/
        }
        else{ /* isErroneous(sudoku)==0 -> board is valid*/
            printSolved();
            sudoku->mode=INIT;
        }
    }
}

void backToOriginalState(SudokuCell*** originalBoard, SudokuCell*** invalidBoard, int total_size){
    copyBoardValues(originalBoard, invalidBoard, total_size, 0); /*fromBoard, toBoard, size, copyFixedCellsOnly*/
}

void updateSudokuCntFilledCells(int* p_cntFilledCell, int fromValue, int toValue){
    if (fromValue == 0 && toValue != 0){
        (*p_cntFilledCell)++;
    }
    if (fromValue != 0 && toValue == 0){
        (*p_cntFilledCell)--;
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Moves array*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void addMoveToArrMoveAndIncrementSize(Move **arrMove, int *p_arrSize, int x, int y, int beforeValue, int afterValue, int beforeErroneous, int afterErroneous){
    Move* newMove;
    newMove = getNewMove(x, y, beforeValue, afterValue, beforeErroneous, afterErroneous);
    arrMove[*p_arrSize] = newMove;
    (*p_arrSize)++;
}

void addArrMoveToList(Sudoku *sudoku, Move** arrMove, int arrSize){
    deleteFromCurrent(sudoku->list);
    insertAtTail(sudoku->list, arrMove, arrSize);
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Generate*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void createGenerateMovesArr(Move **arrMove, Sudoku *sudoku, SudokuCell ***tmpBoard){
    int numOfMoves=0;
    int i, j;
    for (i=0;i<sudoku->total_size; ++i){
        for(j=0;j<sudoku->total_size;++j){
            if(sudoku->currentState[i][j]->digit != tmpBoard[i][j]->digit){
                addMoveToArrMoveAndIncrementSize(arrMove, &numOfMoves, i, j, sudoku->currentState[i][j]->digit,
                                                 tmpBoard[i][j]->digit, 0, 0);
            }
        }
    }
    addArrMoveToList(sudoku, arrMove, numOfMoves);
}

void generate(Sudoku* sudoku, int x, int y){/* TODO: narrow function */
    int isSolvable, areXCellsFilled, cntNoSolution=0;
    int isValid=1;
    int total_cells = sudoku->total_size*sudoku->total_size;
    Move** arrMove;
    SudokuCell*** tmpBoard = copyBoard(sudoku->currentState, sudoku->total_size, 0); /* 0 for copying the all board */
    /*saves the original board for cases that "fillXCells" or "ILP_Validation" will fail*/
    int numOfEmptyCells = total_cells - sudoku->cntFilledCell;
    Cell** emptyCellsArr;
    if(numOfEmptyCells < x){
        printNotEnoughEmptyCells(x, numOfEmptyCells);
        isValid=0;
    }
    else if(isErroneous(sudoku)==1){
        printCannotGenerateBoardWithErrors();
        isValid=0;
    }
    else{ /*in Edit mode AND there are at least X empty cells AND the board has no errors*/
        emptyCellsArr = (Cell**)malloc(numOfEmptyCells*(sizeof(Cell*)));
        if (emptyCellsArr == NULL){
            printMallocFailedAndExit();
        }
        createEmptyCellsArr(sudoku, emptyCellsArr);
        while (cntNoSolution<1000){
            areXCellsFilled = fillXCells(tmpBoard, x, emptyCellsArr, numOfEmptyCells,
                                         sudoku->row, sudoku->column);
            if (areXCellsFilled==0){
                cntNoSolution++;
                backToOriginalState(sudoku->currentState, tmpBoard, sudoku->total_size);
                continue;
            }
            isSolvable = ILP_Validation(tmpBoard, sudoku->row, sudoku->column, GENERATE, -1, -1, NULL);/* should fill out the board */
            if (isSolvable != 1 ){
                if (isSolvable == -1){
                    printGurobiFailed();
                }
                cntNoSolution++;
                backToOriginalState(sudoku->currentState,tmpBoard, sudoku->total_size);
                continue;
            }
            /*arrive here when BOTH "fillXCells" and "ILP_Validation" succeed*/
            break;
        }
        freeCellsArray(emptyCellsArr, numOfEmptyCells);
        if(cntNoSolution<1000){
            arrMove = (Move**)malloc(total_cells* sizeof(Move*));
            if (arrMove == NULL) {
                printMallocFailedAndExit();
            }
            keepYCells(tmpBoard, y, sudoku);
            createGenerateMovesArr(arrMove, sudoku, tmpBoard);
            freeBoard(sudoku->currentState, sudoku->total_size);
            sudoku->currentState = copyBoard(tmpBoard, sudoku->total_size, 0); /* 0 for copying the all board */
            sudoku->cntFilledCell = y;
        }
        else{
            printErrorInPuzzleGenerator();
            isValid=0;
        }
    }
    freeBoard(tmpBoard, sudoku->total_size);
    /*TODO: message for Yoni from xaim - I think that in EDIT mode (note: this command is only available in EDIT), even if the board is "solved" we shouldn't move to INIT (and also according to the some students in the whatsapp group). update me with your decision*/
    if(isValid){
        if(y==total_cells){
            printSolved();
            sudoku->mode=INIT;
        }
        print_board(sudoku);
    }
}

void createEmptyCellsArr(Sudoku *sudoku, Cell** emptyCellsArr){
    int  i, j, index=0;
    for(i=0;i<sudoku->total_size; ++i){
        for (j=0;j<sudoku->total_size;++j){
            if (sudoku->currentState[i][j]->digit == 0){
                emptyCellsArr[index] = (Cell*)malloc(sizeof(Cell));
                if(emptyCellsArr[index] == NULL){
                    printMallocFailedAndExit();
                }
                emptyCellsArr[index]->x = i;
                emptyCellsArr[index]->y = j;
                ++index;
            }
        }
    }
}

void swapArrayCells(Cell** cellsArr, int index_1, int index_2){
    Cell* tmp;
    tmp = cellsArr[index_1];
    cellsArr[index_1] = cellsArr[index_2];
    cellsArr[index_2] = tmp;
}

int fillXCells(SudokuCell*** tmpBoard, int x, Cell** emptyCellsArr, int numOfEmptyCells, int row, int column){
    int empty_cell_index, rand_dig_index, num_of_optional_digits;
    Cell* empty_cell_coordinates;
    for(;x>0;--x){
        empty_cell_index = rand()%numOfEmptyCells;
        empty_cell_coordinates = emptyCellsArr[empty_cell_index];
        tmpBoard[empty_cell_coordinates->x][empty_cell_coordinates->y]->numOfOptionalDigits = row*column;
        findThePossibleArray(tmpBoard, row, column, empty_cell_coordinates->x, empty_cell_coordinates->y);
        num_of_optional_digits = tmpBoard[empty_cell_coordinates->x][empty_cell_coordinates->y]->numOfOptionalDigits;
        if (num_of_optional_digits>0){
            rand_dig_index = rand()%(num_of_optional_digits);
        }
        else{
            return 0;
        }
        tmpBoard[empty_cell_coordinates->x][empty_cell_coordinates->y]->digit = tmpBoard[empty_cell_coordinates->x][empty_cell_coordinates->y]->optionalDigits[rand_dig_index];
        swapArrayCells(emptyCellsArr, empty_cell_index, numOfEmptyCells-1);
        numOfEmptyCells--;
    }
    return 1;
}

void resetCells(SudokuCell*** board, Cell** cellsArr, int numOfCells){
    int i;
    for(i=0;i<numOfCells;++i){
        board[cellsArr[i]->x][cellsArr[i]->y]->digit=0;
    }
}

void keepYCells(SudokuCell*** tmpBoard, int y, Sudoku* sudoku){
    int i=0, row_index, column_index, num_of_available_cells = sudoku->total_size*sudoku->total_size, chosen_cell_index;
    Cell** all_available_cells = (Cell**)malloc(sizeof(Cell*)*num_of_available_cells);
    if(all_available_cells==NULL){
        printMallocFailedAndExit();
    }
    for(row_index=0;row_index<sudoku->total_size; ++row_index){
        for(column_index=0;column_index<sudoku->total_size; ++column_index){
            all_available_cells[i] = (Cell*)malloc(sizeof(Cell));
            if(all_available_cells[i]==NULL){
                printMallocFailedAndExit();
            }
            all_available_cells[i]->x = row_index;
            all_available_cells[i]->y = column_index;
            ++i;
        }
    }
    /*makeAllCellsUnfixed(tmpBoard, sudoku->total_size);*/
    for(;y>0;--y) {
        chosen_cell_index = rand()%num_of_available_cells;
        swapArrayCells(all_available_cells, chosen_cell_index, num_of_available_cells-1);
        --num_of_available_cells;
    }
    resetCells(tmpBoard, all_available_cells, num_of_available_cells);
    freeCellsArray(all_available_cells, num_of_available_cells);
}


/* TODO: 1. take care of erroneous cells when adding a move to undo\redo list (if the cell has changed and became erroneous - and vice versa)
 *       2. take care of undo\redo list with "generate" or "guess"
 *       3. take care of erroneous in general (when changing cells for example)*/



/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Mark errors*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void mark_errors(Sudoku* sudoku,int x){
    sudoku->markErrors = x;
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Set*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void setCell(Sudoku* sudoku, int x, int y, int z, Move** arrMove, int* p_arrSize){
    int beforeErroneous, afterErroneous;
    int dig = sudoku->currentState[x][y]->digit;
    beforeErroneous = sudoku->currentState[x][y]->cnt_erroneous;
    neighborsErroneous(sudoku, x, y, dig, z, arrMove, p_arrSize);
    if (dig == 0){
        sudoku->cntFilledCell++;
    }
    if (z == 0){
        sudoku->cntFilledCell--;
    }
    sudoku->currentState[x][y]->digit=z;
    afterErroneous = sudoku->currentState[x][y]->cnt_erroneous;
    updateSudokuCntErroneousCells(&sudoku->cntErroneousCells, beforeErroneous, afterErroneous);

    addMoveToArrMoveAndIncrementSize(arrMove, p_arrSize, x, y, dig, z, beforeErroneous, afterErroneous);
    /*deleteDigitFromArr(sudoku->currentState, x,y, z);*/
}

void set(Sudoku* sudoku, int x, int y, int z){
    int fixed, dig;
    int arrSize=0, total_cells = sudoku->total_size*sudoku->total_size;
    Move** arrMove = (Move**)malloc(total_cells*(sizeof(Move*)));
    if (arrMove == NULL){
        printMallocFailedAndExit();
    }
    fixed = isFixed(sudoku->currentState,x,y);
    /*TODO: think if the condition 'sudoku->mode==SOLVE' can be remove*/
    if (fixed && sudoku->mode==SOLVE){ /*in Solve mode fixed cells can't be updated (unlike in Edit mode) @@@@@@@@@@@@@@@@@@@@@@@@@@@@ check if we should make all cells in EDIT non-fixed*/
        printFixed();
        return;
    }
    /*arrive here when changing the cell is legal*/
    dig = sudoku->currentState[x][y]->digit;
    if (z != dig) {
        setCell(sudoku, x, y, z, arrMove, &arrSize);
        addArrMoveToList(sudoku, arrMove, arrSize);
        lastCellToBeFilled(sudoku);
    }
    else{
        addMoveToArrMoveAndIncrementSize(arrMove, &arrSize, x, y, dig, z, sudoku->currentState[x][y]->cnt_erroneous, sudoku->currentState[x][y]->cnt_erroneous);
        addArrMoveToList(sudoku, arrMove, arrSize);
        printSameValueCell();
    }
    print_board(sudoku);
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Print board*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void print_board(Sudoku* sudoku){
    printSudoku(sudoku);
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Validate*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void validate(Sudoku* sudoku){
    int isSolvable;
    if (isErroneous(sudoku)==1){ /*board is erroneous*/
        printErroneousBoard();
    }
    else{ /*in Solve/Edit mode AND not erroneous*/
        isSolvable = ILP_Validation(sudoku->currentState, sudoku->row, sudoku->column, VALIDATE, -1, -1, NULL);
        if (isSolvable != 1 ){
            if (isSolvable == 0){
                printUnsolvableBoard();
            }
            else if (isSolvable == -1){
                printGurobiFailed();
            }
            return;
        }
        else{ /*board is solvable*/
            printSolvableBoard();
        }
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Guess*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void guess(Sudoku* sudoku, float x){
    int isSolvable;
    if (isErroneous(sudoku)==1){ /*board is erroneous*/
        printErroneousBoard();
    }
    else{
        isSolvable = LP_Validation(sudoku, sudoku->row, sudoku->column, GUESS, -1, -1, x);
        if (_validate_gurobi(isSolvable)){
            if(sudoku->cntFilledCell == ((sudoku->total_size)*(sudoku->total_size))){
                printSolved();
                sudoku->mode=INIT;
            }
            print_board(sudoku);
        }
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Redo\Undo */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int hasMoveToUndo(Sudoku* sudoku){/*TODO: CREATE DUMMY FUNCTION!*/
    /*return hasPrev(sudoku->list);*/
    if (isDummyNode(sudoku->list)){ /*-1 for dummy node*/
        return 0;
    }
    return 1;
}
int hasMoveToRedo(Sudoku* sudoku){ /*TODO: CHECK THIS*/
    /*return sudoku->list->current->arrSize == -1 || hasNext(sudoku->list);*/
    return hasNext(sudoku->list);
}

void setPointerToPreviousMove(Sudoku* sudoku){
    moveToPrev(sudoku->list);
}

void setPointerToNextMove(Sudoku* sudoku){
    moveToNext(sudoku->list);
}

/* command=UNDO if we want to redo the move.
 * command=REDO if we want to undo the move*/
void updateTheBoard(Sudoku* sudoku, Move* move, Command command) {
    int dig;
    dig = sudoku->currentState[move->cell->x][move->cell->y]->digit;
    updateSudokuCntErroneousCells(&sudoku->cntErroneousCells, move->beforeErroneous, move->afterErroneous);
    if (command == UNDO) { /*undo the move*/
        updateSudokuCntFilledCells(&sudoku->cntFilledCell, dig, move->beforeValue);
        sudoku->currentState[move->cell->x][move->cell->y]->digit = move->beforeValue;
        sudoku->currentState[move->cell->x][move->cell->y]->cnt_erroneous = move->beforeErroneous;
    }
    else if (command == REDO) { /*redo the move*/
        updateSudokuCntFilledCells(&sudoku->cntFilledCell, dig, move->afterValue);
        sudoku->currentState[move->cell->x][move->cell->y]->digit = move->afterValue;
        sudoku->currentState[move->cell->x][move->cell->y]->cnt_erroneous = move->afterErroneous;
    }
}

int isTrivialSet(Move* currentMove){
    if (currentMove->beforeValue == currentMove->afterValue && currentMove->afterErroneous == currentMove->beforeErroneous){
        return 1;
    }
    return 0;
}

void undoMove(Sudoku* sudoku, Command command){
    int i;
    Move** currentArrMove = getCurrentMove(sudoku->list)->arrMove;
    int currentArrSize = getCurrentMove(sudoku->list)->arrSize;
    Move* currentMove;
    for (i = currentArrSize - 1; i>=0; i--){
        currentMove = currentArrMove[i];
        if (currentMove->cell->x != -1){ /*if this is the case, it was an empty autofill*/
            updateTheBoard(sudoku, currentMove ,UNDO);
        }
        else{
            printEmptyAutofill();
        }
        if ((currentMove->afterValue != currentMove->beforeValue && command != RESET) || isTrivialSet(currentMove)){ /*print only if the value of the cell was changed*/
            printChangeInBoard(currentMove->cell, currentMove->afterValue, currentMove->beforeValue); /*print the change that was made*/
        }
    }
    setPointerToPreviousMove(sudoku);
}
void undo(Sudoku* sudoku){
    if (hasMoveToUndo(sudoku)==0){ /*there are no moves to undo*/
        printNoMovesToUndo();
    }
    else { /*in Solve/Edit mode AND has move to undo*/
        undoMove(sudoku, UNDO);
        print_board(sudoku);
    }
}

void redoMove(Sudoku* sudoku){
    int i;
    Move** currentArrMove;
    int currentArrSize;
    Move* currentMove;
    setPointerToNextMove(sudoku);
    currentArrMove = getCurrentMove(sudoku->list)->arrMove;
    currentArrSize = getCurrentMove(sudoku->list)->arrSize;
    /*TODO: A reminder for Ron */
    /*for (i = currentArrSize - 1; i>=0; i--){*/
    for (i=0;i<currentArrSize;i++){
        currentMove = currentArrMove[i];
        if (currentMove->cell->x != -1){ /*if this is the case, it was an empty autofill*/
            updateTheBoard(sudoku, currentMove ,REDO);
        }
        else{
            printEmptyAutofill();
        }
        if ((currentMove->afterValue != currentMove->beforeValue) || isTrivialSet(currentMove)){ /*print only if the value of the cell was changed*/
            printChangeInBoard(currentMove->cell, currentMove->beforeValue, currentMove->afterValue); /*print the change that was made*/
        }
    }
}
void redo(Sudoku* sudoku){
    if (hasMoveToRedo(sudoku)==0){ /*there are no moves to redo*/
        printNoMovesToRedo();
    }
    else { /*in Solve/Edit mode AND has move to redo*/
        redoMove(sudoku);
        print_board(sudoku);
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Save */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

int checkIfWriteFailed(char *X, int isWrite){
    if (isWrite < 0){
        printWriteToFileFailed(X);
        return 0;
    }
    return 1;
}

void saveBoardInFile(Sudoku* sudoku, char* X){
    FILE* file;
    int isValid, isClosed, i, j, dig, fixed, total_size = sudoku->total_size;

    file = fopen(X,"w");
    if (file == NULL){
        printOpenFileFailed(X);
        return;
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
            if (fixed==1 || isFixedInEdit(sudoku, i, j)){ /*cell is fixed*/
                isValid = checkIfWriteFailed(X, fprintf(file, "%d.", dig));
            }
            else{
                isValid = checkIfWriteFailed(X, fprintf(file, "%d", dig));
            }
            if(isValid==0){
                return;
            }
            if (j == total_size-1){ /*end of row*/
                isValid = checkIfWriteFailed(X, fprintf(file, "\n"));
            }
            else{
                isValid = checkIfWriteFailed(X, fprintf(file, " "));
            }
            if(isValid==0){
                return;
            }
        }
    }

    isClosed = fclose(file);
    if (isClosed == -1){
        printCloseFileFailed(X);
        return;
    }
}
void save(Sudoku* sudoku, char* X) {
    int isSolvable;
    if (sudoku->mode == EDIT){
        if (isErroneous(sudoku)==1) { /* 1 if board is erroneous*/
            printErroneousBoard();
            return;
        }
        isSolvable = ILP_Validation(sudoku->currentState, sudoku->row, sudoku->column, SAVE , -1, -1, NULL);
        if (isSolvable != 1 ){
            if (isSolvable == 0){
                printUnsolvableBoard();
            }
            else if (isSolvable == -1){
                printGurobiFailed();
            }
            return;
        }
    }
    /*now clear for saving the file*/
    saveBoardInFile(sudoku,X);
}


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Hint */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void hint(Sudoku* sudoku, int x, int y){
    int isSolvable, dig;
    if (_validate_hint(sudoku, x, y)){ /*in Solve mode AND all valid*/
        isSolvable = ILP_Validation(sudoku->currentState, sudoku->row, sudoku->column, HINT, x, y, &dig);
        if (_validate_gurobi(isSolvable)){ /*board is solvable*/
            printHint(x+1,y+1,dig);/*prints the value of cell <X,Y> found by the ILP solution*/
        }
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Guess hint */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

void guess_hint(Sudoku* sudoku, int x, int y){
    int isSolvable;
    if(_validate_hint(sudoku, x, y)){
        isSolvable = LP_Validation(sudoku, sudoku->row, sudoku->column, GUESS_HINT, x, y, -1.0);
        _validate_gurobi(isSolvable);
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Num solutions */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void num_solutions(Sudoku* sudoku){
    int numOfSolution, total_size = sudoku->total_size;
    Cell* firstEmptyCell;
    SudokuCell*** fixedBoard;
    if (isErroneous(sudoku)==1){ /*board is erroneous*/
        printErroneousBoard();
    }
    else{ /*in Solve mode AND not erroneous*/
        firstEmptyCell = (Cell*)malloc(sizeof(Cell));
        fixedBoard = (SudokuCell***)malloc(total_size*sizeof(SudokuCell**));/*TODO: allocating memory */
        if(firstEmptyCell == NULL || fixedBoard == NULL){
            printMallocFailedAndExit();
        }
        createEmptyBoard(fixedBoard,total_size);
        currentStateToFixed(sudoku, fixedBoard, total_size);/*copy the sudoku.currentState to fixedBoard*/
        findNextEmptyCell(fixedBoard, total_size,firstEmptyCell,0,0); /*search for the first empty cell in the board*/
        if (firstEmptyCell->x == -1 && firstEmptyCell->y == -1){ /* no empty cells */
            numOfSolution = 1;
        }
        else{
            numOfSolution = exhaustiveBacktracking(sudoku, fixedBoard, firstEmptyCell->x, firstEmptyCell->y);
        }
        printNumOfSolution(numOfSolution);
        freeBoard(fixedBoard,total_size);
        free(firstEmptyCell);
    }
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Autofill */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
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
        printMallocFailedAndExit();
    }
    arrMove[0]=newMove;
    insertAtTail(sudoku->list, newMove,1);
}*/


/*TODO: need to deal with situation that 2 obvious cells become erroneous*/
int fillObviousValues(Sudoku* sudoku, int autoFillBeforeILP){
    int arrSize=0, dig, i, j, total_size = sudoku->total_size, cntAutoFilled=0;
    int total_cells = sudoku->total_size*sudoku->total_size;
    Move** arrMove;
    if (autoFillBeforeILP == 0){ /*regular autoFill*/
        arrMove = (Move**)malloc(total_cells*(sizeof(Move*)));
        if (arrMove == NULL){
            printMallocFailedAndExit();
        }
    }
    markSingleLegalValue(sudoku);
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            if (hasSingleLegalValue(sudoku,i,j)){
                dig = sudoku->currentState[i][j]->optionalDigits[0];
                if (autoFillBeforeILP == 0){ /*regular autoFill*/
                    setCell(sudoku, i, j, dig, arrMove, &arrSize);
                }
                else{ /*autoFillBeforeILP == 1*/
                }
                cntAutoFilled++;
                /* updateObviousCell(sudoku,i,j, arrMove, arrSize);*/

                /*check why we need to increment arrSize
                 arrSize++;*/
            }
        }
    }
    if (cntAutoFilled == 0){
        addMoveToArrMoveAndIncrementSize(arrMove, &arrSize, -1, -1, -1, -1 , -1, -2);
        addArrMoveToList(sudoku, arrMove, arrSize);
        return 0;
    }
    if (autoFillBeforeILP == 0){ /*regular autoFill*/
        addArrMoveToList(sudoku, arrMove, arrSize);
    }

    return 1;

}

void autofill(Sudoku* sudoku){
    int isAutoFilled;
    if (isErroneous(sudoku)==1){ /*board is erroneous*/
        printErroneousBoard();
    }
    else{ /*in Solve mode AND not erroneous*/
        isAutoFilled = fillObviousValues(sudoku, 0); /* 0 for regular autoFill */
        if (isAutoFilled == 0){ /*filled zero cells*/
            printNoAutoFilledCells();
        }
        else{
            if(isFilled(sudoku) && !isErroneous(sudoku)){
                printSolved();
                sudoku->mode = INIT;
            }
            print_board(sudoku);
        }

    }
}


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Reset */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void undoAllMoves(Sudoku* sudoku){
    while (hasMoveToUndo(sudoku)==1){
        undoMove(sudoku, RESET);
    }
}

void reset(Sudoku* sudoku){
    undoAllMoves(sudoku);
    print_board(sudoku);
}


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Exit program */
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void exitProgram(){
    printExitMessage();
}


/****************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************
 * **************************************************************************/





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


