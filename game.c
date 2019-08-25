#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "solver.h"
#include "main_aux.h"
#include "doubly_linked_list.h"

/* TODO: 1. take care of erroneous cells when adding a move to undo\redo list (if the cell has changed and became erroneous - and vice versa)
 *       2. take care of undo\redo list with "generate" or "guess"
 *       3. take care of erroneous in general (when changing cells for example)*/

int ILP_Validation(SudokuCell*** tmpBoard);
void LP_Guesses(Sudoku* sudoku, float x);
void fillCellsWithScoreX(Sudoku* sudoku,float x);
int LP_Validation(Sudoku* sudoku);
int checkIfBoardIsSolvable();

void freeMemory(Sudoku* sudoku){
    freeBoard (sudoku->currentState, sudoku->total_size);
    freeBoard (sudoku->solution, sudoku->total_size);
    freeList(sudoku->list);
    free(sudoku);
}

/* TODO: check in email (or yuval porat) when we the load is valid*/

int createEmptyCellsArr(Sudoku *sudoku, Cell** emptyCellsArr){
    int  i=0, j=0, index=0;
    for(;i<sudoku->total_size; ++i){
        for (;j<sudoku->total_size;++j){
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
    SudokuCell* chosen_cell;
    for(;x>0;--x){
        empty_cell_index = rand()%numOfEmptyCells;
        empty_cell_coordinates = emptyCellsArr[empty_cell_index];
        chosen_cell = tmpBoard[empty_cell_coordinates->x][empty_cell_coordinates->y];
        chosen_cell->numOfOptionalDigits = row*column;
        findThePossibleArray(tmpBoard, row, column, empty_cell_coordinates->x, empty_cell_coordinates->y);
        num_of_optional_digits = chosen_cell->numOfOptionalDigits;
        if (num_of_optional_digits>0){
            rand_dig_index = rand()%(num_of_optional_digits);
        }
        else{
            return 0;
        }
        chosen_cell->digit = chosen_cell->optionalDigits[rand_dig_index];
        swapArrayCells(emptyCellsArr, empty_cell_index, numOfEmptyCells-1);
        numOfEmptyCells--;
    }
    return 1;
}

void freeCellsArray(Cell **arr, int arr_size){
    int i=0;
    for(;i<arr_size;++i){
        free(arr[i]);
    }
    free(arr);
}

void resetCells(SudokuCell*** board, Cell** cellsArr, int numOfCells){
    int i=0;
    for(;i<numOfCells;++i){
        board[cellsArr[i]->x][cellsArr[i]->y]->digit=0;
    }
}

void keepYCells(SudokuCell*** tmpBoard, int y, Sudoku* sudoku){
    int i=0, row_index=0, column_index=0, num_of_available_cells = sudoku->total_size*sudoku->total_size, chosen_cell_index;
    Cell** all_available_cells = (Cell**)malloc(sizeof(Cell*)*num_of_available_cells);
    if(all_available_cells==NULL){
        printMallocFailedAndExit();
    }
    for(;row_index<sudoku->total_size; ++row_index){
        for(;column_index<sudoku->total_size; ++column_index){
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

int areOnlyWhitespacesLeft(FILE* file, char* X){
    int c;
    while ((c = fgetc(file)) != EOF){
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n' ){
            printLoadedFileLengthNotValid(X);
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

int scanRowAndColumn(FILE* file, char* X, int* p_row, int* p_column){
    int scan;
    scan = fscanf(file,"%d", p_column);
    /*checkEOF();*/
    if (scan == EOF){
        printLoadedFileEOF(X);
        return 0;
    }
    if (scan!=1){
        printLoadedFileFirstLineNotValid(X);
        return 0;
    }
    scan = fscanf(file,"%d", p_row);
    /*checkEOF();*/
    if (scan == EOF){
        printLoadedFileEOF(X);
        return 0;
    }
    if (scan!=1){
        printLoadedFileFirstLineNotValid(X);
        return 0;
    }
    return 1;
}
/*TODO: take care of cases that there is an empty line in the middle of the file
 * example:
 * 5
 *
 *      7 */
int scanCells(FILE* file, char* X, SudokuCell*** board, Mode mode, int total_size, int* p_cntFilledCell){
    int i,j, scan, dig, fixed, isValid;
    char c;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            scan = fscanf(file,"%d", &dig);
            if (scan == EOF){
                printLoadedFileEOF(X);
                return 0;
            }
            if (scan!=1){/*if for example there is a char */
                printLoadedFileCellNotValid(X);
                return 0;
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
                    return 0;
                }
            }
            else{
                if (j == total_size-1 && i == total_size-1){ /*the last cell*/
                    fixed = 0;
                }
                else{ /*not the last cell*/
                    printLoadedFileEOF(X);
                    return 0;
                }
            }
            if (dig > total_size || dig < 0){
                printLoadedFileCellNotValid(X);
                return 0;
            }
            if ( (dig == 0 && fixed ==1) || (mode == EDIT && fixed == 0) ){
                printLoadedFileCellNotValid(X);
                return 0;
            }
            if (mode == EDIT){ /*in EDIT mode, no cell is considered fixed*/
                fixed = 0;
            }
            board[i][j]->digit = dig;
            board[i][j]->is_fixed = fixed;
            if (dig != 0){
                (*p_cntFilledCell)++;
            }
        }
    }
    isValid = areOnlyWhitespacesLeft(file, X); /*check if there are only whitespaces left in the file*/
    if (isValid == 0){
        printLoadedFileLengthNotValid(X);
        return 0;
    }
    if (mode == EDIT){
        isValid = checkIfBoardIsSolvable();
        if (isValid == 0){
            printLoadedFileNotSolvable(X);
            return 0;
        }
    }
}

void fileToSudoku(Sudoku* sudoku, FILE* file, char* X, Mode mode){
    int total_size, row, column, cntFilledCell=0, isValid;
    SudokuCell*** board = (SudokuCell***)malloc(sizeof(SudokuCell**));
    if (board == NULL){
        printMallocFailedAndExit();
    }
    /*if scanRowAndColumn OR scanCells failed, need to finish the function
     * check that the old boared doesn't disappear*/
    isValid = scanRowAndColumn(file, X, &row, &column); /*find m & n*/
    if (isValid == 0){
        return;
    }
    if (row * column > 99 || row <=0 || column<=0){
        printLoadedFileFirstLineNotValid(X);
    }
    total_size = row * column;
    createEmptyBoard(board, total_size); /* for empty board*/
    /*scan cells and check if all valid and solvable*/
    isValid = scanCells(file, X, board, mode, total_size, &cntFilledCell);
    if (isValid == 0){
        return;
    }
    updateSudoku(sudoku, X, mode, board,row ,column, cntFilledCell);

}

void loadBoardFromPath(Sudoku* sudoku, char* X, Mode mode){
    FILE* file;
    /*int isClosed;*/
    file = fopen(X,"r");
    if (file == NULL){
        printOpenFileFailed(X);
        return;
    }
    fileToSudoku(sudoku, file, X, mode);
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
void editWithoutPath(Sudoku* sudoku) {

    SudokuCell ***emptyBoard = (SudokuCell ***) malloc(sizeof(SudokuCell **));
    if (emptyBoard == NULL) {
        printMallocFailedAndExit();
    }
    createEmptyBoard(emptyBoard, 9); /* for empty 9X9 board*/
    updateSudoku(sudoku, NULL, EDIT, emptyBoard, 3, 3, 0);
}

void edit(Sudoku* sudoku, char* X){
    if (X == NULL){
        editWithoutPath(sudoku);
    }
    else{
        editWithPath(sudoku, X);
    }
}

int isFixedAndErroneous(Sudoku* sudoku){
    int i,j;
    for (i=0; i<sudoku->total_size; i++) {
        for (j = 0; j < sudoku->total_size; j++) {
            if (isFixed(sudoku->currentState, i, j) && isCellErroneous(sudoku->currentState, i, j)) {
                return 0;
            }
        }
    }
    return 1;
}

void updateSudoku(Sudoku* sudoku, char* X, Mode mode, SudokuCell*** newCurrentState, int newRow, int newColumn, int newCntFilledCell){

    int isValid;
    /*TODO: check what to do if we are in INIT mode -> we dont have a board to free*/
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
        isValid = isFixedAndErroneous(sudoku);
        if (isValid == 0){
            printLoadedFileNotSolvable(X);
        }
    }
    else{
        sudoku->cntErroneousCells = 0;
    }
}

int isContainsValue(Sudoku* sudoku, int x, int y){
    return sudoku->currentState[x][y]!=0;
}

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
    sudoku->markErrors = x;
}

void addMoveToArrMoveAndIncrementSize(Move **arrMove, int *p_arrSize, int x, int y, int beforeValue, int afterValue,
                                      int beforeErroneous, int afterErroneous){
    Move* newMove;
    newMove = getNewMove(x, y, beforeValue, afterValue, beforeErroneous, afterErroneous);
    arrMove[*p_arrSize] = newMove;
    (*p_arrSize)++;
}

void valueValidationAmongNeighbors(SudokuCell*** board, int i, int j, int dig, Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells, int* p_cntNeighborsErroneous){
    int beforeErroneous, afterErroneous;
    if (!isFixed(board, i, j) && neighborsType != NEIGHBORS_POSSIBLE_ARRAY) { /*cell isn't fixed -> should be mark as erroneous */
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

void setCntNeighborsErroneous(SudokuCell*** board, int x, int y, NeighborsType neighborsType, int cntNeighborsErroneous){
    if (neighborsType == NEIGHBORS_FROM_DIG){
        board[x][y]->cnt_erroneous-=cntNeighborsErroneous;
    }
    else if(neighborsType == NEIGHBORS_TO_DIG || neighborsType == NEIGHBORS_LOAD_FILE){
        board[x][y]->cnt_erroneous+=cntNeighborsErroneous;
    }
}

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
int isRowValid(SudokuCell*** board, int row, int column, int x, int y,  int value , Move** arrMove, int* p_arrSize, NeighborsType neighborsType, int* p_cntTotalErroneousCells){
    int dig, i, isValid = 1, cntNeighborsErroneous=0;
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
    int dig, i, isValid = 1, cntNeighborsErroneous=0;
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
    int dig, i, j, isValid = 1, cntNeighborsErroneous;
    Cell* head=(Cell*)malloc(sizeof(Cell));
    if(head == NULL){
        printMallocFailedAndExit();
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

void neighborsLoadFile(Sudoku *sudoku, int x, int y, int dig){

    isValueValid(sudoku->currentState ,sudoku->row, sudoku->column, x, y, dig, NULL, NULL, NEIGHBORS_LOAD_FILE, &sudoku->cntErroneousCells);

}

void neighborsToDig(Sudoku *sudoku, int x, int y, int dig, Move **arrMove, int *p_arrSize){

    isValueValid(sudoku->currentState ,sudoku->row, sudoku->column, x, y, dig, arrMove, p_arrSize, NEIGHBORS_TO_DIG, &sudoku->cntErroneousCells);

}

void neighborsFromDig(Sudoku *sudoku, int x, int y, int dig, Move **arrMove, int *p_arrSize){

    isValueValid(sudoku->currentState, sudoku->row, sudoku->column, x, y, dig, arrMove, p_arrSize, NEIGHBORS_FROM_DIG, &sudoku->cntErroneousCells);

}

int neighborsPossibleArr(SudokuCell*** board, int row, int column, int x, int y, int dig){

    return isValueValid(board ,row, column, x, y, dig, NULL, NULL, NEIGHBORS_POSSIBLE_ARRAY, NULL);

}


void neighborsErroneous(Sudoku* sudoku, int x, int y, int dig, int z, Move** arrMove, int* p_arrSize) {
    if (dig!=0) {
        neighborsFromDig(sudoku, x, y, dig, arrMove, p_arrSize);
    }
    sudoku->currentState[x][y]->cnt_erroneous=0; /*reset the erroneous of the cell*/
    if (z!=0){
        neighborsToDig(sudoku, x, y, z, arrMove, p_arrSize);
    }
}

/*count and mark all the erroneous cells*/
void findErroneousCells(Sudoku* sudoku){
    int i,j;
    for (i=0; i<sudoku->total_size; i++){
        for (j=0; j<sudoku->total_size; j++){
            neighborsLoadFile(sudoku, i, j, sudoku->currentState[i][j]->digit);
        }
    }
}

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

SET_STATUS set(Sudoku* sudoku, int x, int y, int z){
    int fixed, dig;
    int arrSize=0, total_cells = sudoku->total_size*sudoku->total_size;
    Move** arrMove = (Move**)malloc(total_cells*(sizeof(Move*)));
    if (arrMove == NULL){
        printMallocFailedAndExit();
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
    return UNSOLVED;
}

void print_board(Sudoku* sudoku){
    printSudoku(sudoku);
}
void validate(Sudoku* sudoku){
    int isSolvable;
    if (isErroneous(sudoku)==1){ /*board is erroneous*/
        printErroneousBoard();
    }
    else{ /*in Solve/Edit mode AND not erroneous*/
        isSolvable = ILP_Validation(sudoku);
        if (isSolvable==0){ /*board is unsolvable*/
            printUnsolvableBoard();
        }
        else{ /*board is solvable*/
            printSolvableBoard();
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

void createGenerateMovesArr(Move** arrMove, Sudoku* sudoku, SudokuCell*** tmpBoard, int* ptrNumOfMoves){
    int i=0, j=0;
    for (;i<sudoku->total_size; ++i){
        for(;j<sudoku->total_size;++j){
            if(sudoku->currentState[i][j]->digit != tmpBoard[i][j]->digit){
                addMoveToArrMoveAndIncrementSize(arrMove, ptrNumOfMoves, i, j, sudoku->currentState[i][j]->digit,
                                                 tmpBoard[i][j]->digit, 0, 0);
            }
        }
    }
}

void createMovesArr(Move **arrMove, Sudoku *sudoku, SudokuCell ***tmpBoard){
    int numOfMoves=0;
    createGenerateMovesArr(arrMove, sudoku, tmpBoard, &numOfMoves);
    addArrMoveToList(sudoku, arrMove, numOfMoves);
}

State generate(Sudoku* sudoku, int x, int y){/* TODO: narrow function */
    int isSolvable, areXCellsFilled, isValid=1, cntNoSolution=0;
    int total_cells = sudoku->total_size*sudoku->total_size;
    SudokuCell*** tmpBoard = copyBoard(sudoku->currentState, sudoku->total_size); /*save the original board for cases that "fillXCells" or "ILP_Validation" will fail*/
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
            isSolvable = ILP_Validation(tmpBoard);/* should fill out the board */
            if (isSolvable==0){
                cntNoSolution++;
                backToOriginalState(sudoku->currentState,tmpBoard, sudoku->total_size);
                continue;
            }
            /*arrive here when BOTH "fillXCells" and "ILP_Validation" succeed*/
            break;
        }
        freeCellsArray(emptyCellsArr, numOfEmptyCells);
        if(cntNoSolution<1000){
            Move** arrMove = (Move**)malloc(total_cells* sizeof(Move*));
            if (arrMove == NULL) {
                printMallocFailedAndExit();
            }
            keepYCells(tmpBoard, y, sudoku);
            createMovesArr(arrMove, sudoku, tmpBoard);
            freeBoard(sudoku->currentState, sudoku->total_size);
            sudoku->currentState = copyBoard(tmpBoard, sudoku->total_size);
        }
        else{
            printErrorInPuzzleGenerator();
            isValid=0;
        }
    }
    freeBoard(tmpBoard, sudoku->total_size);
    if(y==total_cells && isValid){
        return STATE_SOLVED;
    }
    return STATE_LOOP;
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

void undoMove(Sudoku* sudoku, Command command){
    int i;
    Move** currentArrMove = getCurrentMove(sudoku->list)->arrMove;
    int currentArrSize = getCurrentMove(sudoku->list)->arrSize;
    Move* currentMove;
    for (i = currentArrSize - 1; i>=0; i--){
        currentMove = currentArrMove[i];
        updateTheBoard(sudoku, currentMove ,UNDO);
        if (currentMove->afterValue != currentMove->beforeValue && command != RESET){ /*print only if the value of the cell was changed*/
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
    if (hasMoveToRedo(sudoku)==0){ /*there are no moves to redo*/
        printNoMovesToRedo();
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
    if (isErroneous(sudoku)==1 || isFixed(sudoku,x,y)==1 || isContainsValue(sudoku,x,y)==1){ /*board is erroneous OR cell is fixed OR contains a value*/
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
    if (isErroneous(sudoku)==1){ /*board is erroneous*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve mode AND not erroneous*/
        firstEmptyCell = (Cell*)malloc(sizeof(Cell));
        if(firstEmptyCell == NULL){
            printMallocFailedAndExit();
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
        printMallocFailedAndExit();
    }
    arrMove[0]=newMove;
    insertAtTail(sudoku->list, newMove,1);
}*/

void addArrMoveToList(Sudoku *sudoku, Move** arrMove, int arrSize){
    insertAtTail(sudoku->list, arrMove, arrSize);
}


/*TODO: need to deal with situation that 2 obvious cells become erroneous*/
void fillObviousValues(Sudoku* sudoku){
    int arrSize=0, dig, i, j, total_size = sudoku->total_size;
    int total_cells = sudoku->total_size*sudoku->total_size;
    Move** arrMove = (Move**)malloc(total_cells*(sizeof(Move*)));
    if (arrMove == NULL){
        printMallocFailedAndExit();
    }
    markSingleLegalValue(sudoku);
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            if (hasSingleLegalValue(sudoku,i,j)){
                dig = sudoku->currentState[i][j]->optionalDigits[0];
                setCell(sudoku, i, j, dig, arrMove, &arrSize);
               /* updateObviousCell(sudoku,i,j, arrMove, arrSize);*/

               /*check why we need to increment arrSize
                arrSize++;*/
            }
        }
    }
    addArrMoveToList(sudoku, arrMove, arrSize);
}

void autofill(Sudoku* sudoku){
    if (isErroneous(sudoku)==1){ /*board is erroneous*/
        /*print an error message and the command is not executed*/
    }
    else{ /*in Solve mode AND not erroneous*/
        fillObviousValues(sudoku);
    }
}
void undoAllMoves(Sudoku* sudoku){
    while (hasMoveToUndo(sudoku)==1){
        undoMove(sudoku, RESET);
    }
}

void reset(Sudoku* sudoku){
    undoAllMoves(sudoku);
    print_board(sudoku);
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

int isCellErroneous(SudokuCell*** board, int x, int y){
    return board[x][y]->cnt_erroneous;
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


