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


void freeMemory(Sudoku* sudoku){
    freeBoard (sudoku->currentState, sudoku->total_size);
    freeBoard (sudoku->solution, sudoku->total_size);
    freeList(sudoku->list);
    free(sudoku);
}

void updateSudoku(Sudoku* sudoku, Mode mode, SudokuCell*** newCurrentState, int newRow, int newColumn, int newCntFilledCell, int newCntErroneousCells);

void loadBoardFromPath(Sudoku* sudoku, char* X);

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
void scanCells(FILE* file, char* X, SudokuCell*** board, Mode mode, int total_size, int* p_cntFilledCell, int* p_cntErroneousCells){
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
                *p_cntFilledCell++;
            }
        }
    }


    valid = areOnlyWhitespacesLeft(file, X); /*check if there are only whitespaces left in the file*/
    if (valid == 0){
        printLoadedFileLengthNotValid(X);
    }
    valid = checkIfBoardIsSolvable();
    if (valid == 0){
        printLoadedFileNotSolvable(X);
    }
    updateCntErroneousCells(p_cntErroneousCells);

    /*gets here if done*/

}


void fileToSudoku(Sudoku* sudoku, FILE* file, char* X, Mode mode){

    int total_size, num, row, column, numOfCells, cntFilledCell=0, cntErroneousCells=0;
    SudokuCell*** board = (SudokuCell***)malloc(sizeof(SudokuCell**));
    if (board == NULL){
        printMallocFailed();
    }

    scanRowAndColumn(file, X, &row, &column); /*find m & n*/
    if (row * column > 99 || row <=0 || column<=0){
        printLoadedFileFirstLineNotValid();
    }
    total_size = row * column;
    createEmptyBoard(board, total_size); /* for empty board*/

    /*scan cells and check if all valid and solvable*/
    scanCells(file, X, board, mode, total_size, &cntFilledCell, &cntErroneousCells);

    updateSudoku(sudoku, mode, board,row ,column, cntFilledCell, cntErroneousCells);

}
/*TODO: combine 'solve' and 'editWithPath' to one function*/

/*TODO: check about clause b. in this command - what should we do with the unsaved current game board*/
void solve(Sudoku* sudoku, char* X){
    FILE* file;
    int isClosed;

    file = fopen(X,"r");
    if (file == NULL){
        printOpenFileFailed(X);
    }

    fileToSudoku(sudoku, file,X, SOLVE);
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

/*TODO: check about clause d. in this command - what should we do with the unsaved current game board*/
void editWithPath(Sudoku* sudoku, char* X){

    FILE* file;
    int isClosed;

    file = fopen(X,"r");
    if (file == NULL){
        printOpenFileFailed(X);
    }

    fileToSudoku(sudoku, file,X, EDIT);
    /*createSudoku(sudoku, row, column, numOfCells);
    loadBoardFromPath(sudoku, X);*/

    isClosed = fclose(file);
    if (isClosed == EOF){ /* 'fclose' has faild* - maybe should be -1 @@@@@@@@@@@@@@@*/
        printCloseFileFailed(X);
    }

}

/*TODO: check about clause d. in this command - what should we do with the unsaved current game board*/
void editWithoutPath(Sudoku* sudoku){

    SudokuCell*** emptyBoard = (SudokuCell***)malloc(sizeof(SudokuCell**));
    if (emptyBoard == NULL){
        printMallocFailed();
    }
    createEmptyBoard(emptyBoard, 9); /* for empty 9X9 board*/
    updateSudoku(sudoku,EDIT, emptyBoard, 3, 3, 0, 0);

}

void updateSudoku(Sudoku* sudoku, Mode mode, SudokuCell*** newCurrentState, int newRow, int newColumn, int newCntFilledCell, int newCntErroneousCells){

    freeBoard(sudoku->currentState, sudoku->total_size);
    freeBoard(sudoku->solution, sudoku->total_size); /*check if we should calculate the new SOLUTION board* @@@@@@@@@@@@@@@@@@@@@@@@ */

    sudoku->currentState = newCurrentState;
    sudoku->row=newRow;
    sudoku->column=newColumn;
    sudoku->total_size = newRow * newColumn;
    sudoku->cntFilledCell = newCntFilledCell;
    sudoku->cntErroneousCells = newCntErroneousCells;

    freeList(sudoku->list);
    sudoku->list = getNewList();

    /*sudoku->markError remain the same*/

    sudoku->mode = mode;

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
    int erroneousBoard;
    if (sudoku->mode==SOLVE && isFilled(sudoku)){ /*in Solve mode and last cell was filled*/
        erroneousBoard=isErroneous(sudoku); /*check if the board is valid*/
        if (erroneousBoard==1){ /*board is not valid*/
            /*we remain in Solve mode*/
            printSolutionIsErroneous();
            return UNSOLVED; /*full but not SOLVED*/
        }
        else{ /* erroneousBoard=0 -> board is valid*/
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

SET_STATUS set(Sudoku* sudoku, int x, int y, int z){
    int zValid, fixed;
    if (sudoku->mode == INIT){ /* in Init mode*/
        /*print appropriate massage*/
    }
    else{ /*in Solve/Edit mode*/
        fixed = isFixed(sudoku,x,y);
        if (fixed && sudoku->mode==SOLVE){ /*in Solve mode fixed cells can't be updated (unlike in Edit mode) @@@@@@@@@@@@@@@@@@@@@@@@@@@@ check if we should make all cells in EDIT non-fixed*/
            printFixed();
            return UNSOLVED;
        }
        /*arrive here when changing the cell is legal*/
        if (z==0){
            resetCell(sudoku,x,y);
        }
        else{ /*z!=0*/
            if (z!=sudoku->currentState[x][y]->digit){
                zValid = isZValid(sudoku->currentState,sudoku->row,sudoku->column,x,y,z);
                if (zValid==0) { /* z isn't valid*/
                    /*printInvalidValue();*/
                    sudoku->currentState[x][y]->is_erroneous = 1;
                }
                setCell(sudoku,x,y,z);
                print_board(sudoku);
                return lastCellToBeFilled(sudoku);
            }
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

/* command=1 if we want to redo the move.
 * command=0 if we want to undo the move*/
void updateTheBoard(Sudoku* sudoku, Move* move, int command){
    if (command==0){ /*undo the move*/
        sudoku->currentState[move->cell->x][move->cell->y]=move->before;
    }
    else{ /*redo the move (command=1)*/
        sudoku->currentState[move->cell->x][move->cell->y]=move->after;
    }
}

void undoMove(Sudoku* sudoku){
    int i;
    Move** currentArrMove = getCurrentMove(sudoku->list)->arrMove;
    int currentArrSize = getCurrentMove(sudoku->list)->arrSize;
    Move* currentMove;
    for (i=0; i < currentArrSize; i++){
        currentMove = currentArrMove[i];
        updateTheBoard(sudoku, currentMove ,0);
        printChangeInBoard(currentMove->cell, currentMove->after, currentMove->before); /*print the change that was made*/
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
    for (i=0; i < currentArrSize; i++){
        currentMove = currentArrMove[i];
        updateTheBoard(sudoku,currentMove,1);
        printChangeInBoard(currentMove->cell, currentMove->before, currentMove->after); /*print the change that was made*/
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

void checkWriteFailed(char* X, int isWrite){
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

    checkWriteFailed(X, fprintf(file, "%d %d\n", sudoku->column, sudoku->row)); /* write the first line: m n*/
    /*isWrite = fprintf(file, "%d %d\n", sudoku->column, sudoku->row);
    if (isWrite < 0){
        printWriteToFileFailed(X);
    }*/

    for (i=0; i < total_size; i++){
        for (j=0; j < total_size; j++){
            dig = sudoku->currentState[i][j]->digit;
            fixed = sudoku->currentState[i][j]->is_fixed;
            if (fixed==1 || fixedInEdit(sudoku, i, j)){ /*cell is fixed*/
                checkWriteFailed(X, fprintf(file, "%d.",dig));
            }
            else{
                checkWriteFailed(X, fprintf(file, "%d",dig));
            }
            if (j == total_size-1){ /*end of row*/
                checkWriteFailed(X, fprintf(file, "\n"));
            }
            else{
                checkWriteFailed(X, fprintf(file, " "));
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
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 * should enter before 'hint' command*/




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
    SudokuCell*** fixedBoard;
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
            if (sudoku->currentState[i][j]->numOfOptionalDigits==1){
                sudoku->currentState[i][j]->hasSingleLegalValue=1;
            }
        }
    }
}

int hasSingleLegalValue(Sudoku* sudoku, int i, int j){
    return sudoku->currentState[i][j]->hasSingleLegalValue;
}

void updateObviousCell(Sudoku* sudoku, int i, int j, Move** arrMove, int arrSize){
    int dig = sudoku->currentState[i][j]->optionalDigits[0];
    Move* newMove = getNewMove(i,j,0,dig);
    arrMove[arrSize]=newMove;
    deleteDigitFromArr(sudoku->currentState, i,j, dig);
    sudoku->currentState[i][j]->digit=dig;
    sudoku->currentState[i][j]->hasSingleLegalValue=0;
}

void addOneMoveToList(Sudoku *sudoku, int x, int y, int value, int z){
    Move* newMove = getNewMove(x,y,value,z);
    Move** arrMove = (Move**)malloc(sizeof(Move*));
    if (arrMove == NULL){
        printMallocFailed();
    }
    arrMove[0]=newMove;
    insertAtTail(sudoku->list, newMove,1);
}

void addArrMoveToList(Sudoku *sudoku, Move** arrMove, int arrSize){
    insertAtTail(sudoku->list, arrMove, arrSize);
}


void fillObviousValues(Sudoku* sudoku){
    int arrSize=0;
    Move** arrMove = (Move**)malloc(sizeof(Move*));
    if (arrMove == NULL){
        printMallocFailed();
    }
    markSingleLegalValue(sudoku);
    int i,j, total_size = sudoku->total_size;
    for (i=0; i<total_size; i++){
        for (j=0; j<total_size; j++){
            if (hasSingleLegalValue(sudoku,i,j)){
                updateObviousCell(sudoku,i,j, arrMove, arrSize);
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
    Move* currentMove;
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
 * @params - function receives pointer to the main Sudoku, and the cell's indexes.
 *
 * if the cell wasn't reset, the function resets it and update the Sudoku's counters.
 */
void resetCell(Sudoku* sudoku, int x, int y){
    if (sudoku->currentState[x][y]->digit!=0){
        addOneMoveToList(sudoku, x, y, sudoku->currentState[x][y]->digit, 0);
        sudoku->cntFilledCell--;
        sudoku->currentState[x][y]->is_erroneous=0;
        sudoku->currentState[x][y]->digit=0;
    }
}
/*
 * @params - function receives pointer to the main Sudoku, the cell's indexes and a value Z.
 *
 * The function sets the cell.
 * if the cell was reset previously, it updates the Sudoku's counters.
 */
void setCell(Sudoku* sudoku, int x, int y, int z){
    addOneMoveToList(sudoku, x, y, sudoku->currentState[x][y]->digit, z);
    if (sudoku->currentState[x][y]->digit==0){
        sudoku->cntFilledCell++;
    }
    sudoku->currentState[x][y]->digit=z;
}

/*
 * @params - function receives pointer to the main Sudoku.
 *
 * The function checks if the board is solved according to the sudoku's filledCell counter.
 *
 * @return - 1 if solved, 0 otherwise.
 */
int isSolved(Sudoku* sudoku){
    int total_cells = sudoku->total_size*sudoku->total_size;
    if (sudoku->cntFilledCell==total_cells){
        return 1;
    }
    return 0;
}
/*
 * @params - function receives SudokuCell*** board, the required row and column for the specific check, and a value Z.
 *
 * The function checks if the value Z appears in the required row (if the row will stay valid).
 *
 * @return - 1 if valid, 0 otherwise.
 */

int isRowValid(SudokuCell*** board, int row, int column, int z){
    int cell;
    int i;
    for(i=0;i<column;i++){
        cell = board[row][i]->digit;
        if (cell==z){
            return 0;
        }
    }
    return 1;
}
/*
 * @params - function receives SudokuCell*** board, the required row and column for the specific check, and a value Z.
 *
 * The function checks if the value Z appears in the required column (if the column will stay valid).
 *
 * @return - 1 if valid, 0 otherwise.
 */
int isColumnValid(SudokuCell*** board, int row, int column, int z){
    int cell;
    int i;
    for (i = 0; i < row; i++) {
        cell = board[i][column]->digit;
        if (cell == z) {
            return 0;
        }
    }
    return 1;
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
 * @params - function receives SudokuCell*** board, the required row and column for the specific check,
 * the cell's indexes and a value Z.
 *
 * The function search for the most top-left cell in the block of input cell by calling "findHeadBlock".
 * Then, the function checks if the value Z appears in the required block (if the block will stay valid).
 *
 * @return - 1 if valid, 0 otherwise.
 */
int isBlockValid(SudokuCell*** board,int row, int column, int x, int y, int z){
    int i,j,cell;
    Cell* head=(Cell*)malloc(sizeof(Cell));
    if(head == NULL){
        printMallocFailed();
    }
    findHeadBlock(head,row, column,x,y);
    for (i=0;i<row;i++){
        for (j=0; j<column; j++){
            cell = board[i + head->x][j+head->y]->digit;
            if (cell == z) {
                free(head);
                return 0;
            }
        }
    }
    free(head);
    return 1;
}

/*
 * @params - function receives SudokuCell*** board, the required row and column for the specific check,
 * the cell's indexes and a value Z.
 *
 * The function checks if the value Z can be set in the wanted cell
 * by scanning it's row, column and block with calls to the appropriate functions.
 *
 * @return - 1 if valid, 0 otherwise.
 */
int isZValid(SudokuCell*** board, int row, int column, int x, int y, int z){

    int r,c,b;
    r=isRowValid(board,x,row*column,z);
    c=isColumnValid(board,row*column,y,z);
    b=isBlockValid(board,row,column,x,y,z);
    if (r==1&&c==1&&b==1){
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
int isFixed(Sudoku *sudoku, int x, int y) {
    return sudoku->currentState[x][y]->is_fixed;
}

/*
 * @params - function receives pointer to the main Sudoku, the cell's indexes and a value Z.
 *
 * The function checks if the value Z can be set in the wanted cell
 * by checking if the cell is fixed and the value is valid with calls to the appropriate functions.
 * Also, prints an appropriate message to the user.
 *
 * if the cell was set, it checks if the board is solved by calling "isSolved".
 *
 * @return - SOLVED if the board was solved, UNSOLVED otherwise (enum).
 */

SET_STATUS set1(Sudoku *sudoku, int x, int y, int z) {

    int zValid, solved, fixed;
    fixed= isFixed(sudoku,x,y);
    if (fixed){
        printFixed();
        return UNSOLVED;
    }
    if (z==0){
        resetCell(sudoku,x,y);
    }
    else{ /* z!=0*/
        if (z!=sudoku->currentState[x][y]->digit){
            zValid = isZValid(sudoku->currentState,sudoku->row,sudoku->column,x,y,z);
            if (zValid==0){ /* z isn't valid*/
                printInvalidValue();
                return UNSOLVED;
            }
            else{ /* z is valid*/
                setCell(sudoku,x,y,z);
                solved=isSolved(sudoku);
                if (solved){
                    printSudoku(sudoku);
                    printSolved();
                    return SOLVED;
                }
            }
        }
    }
    printSudoku(sudoku);
    return UNSOLVED;
}

/*
 * @params - function receives pointer to the main Sudoku and the cell's indexes.
 *
 * The function prints the cell's value from the Sudoku solution board.
 */
void hint1(Sudoku *sudoku, int x, int y) {
    int cell = sudoku->solution[x][y]->digit;
    printHint(cell);
}

/*
 * @params - function receives pointer to the main Sudoku.
 *
 * The function calls "boardSolver" function and executes the deterministic backtracking
 * in order to search for a solution to the board.
 *
 * The function prints message to the user, according to "boardSolver" result (yes/no for the solution).
 */
void validate1(Sudoku *sudoku) {
    HasSolution hasSolution = boardSolver(sudoku,0); /*calls "boardSolver" with isRandom=0 for deterministic backtracking*/
    if (hasSolution==SOLUTION){
        printSolvable();
    }
    else { /*hasSolution==NO_SOLUTION*/
        printUnsolvable();
    }
}

