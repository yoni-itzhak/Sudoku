#include <stdlib.h>
#include <stdio.h>

#include "doubly_linked_list.h"
#include "solver.h"
#include "main_aux.h"
#include "game.h"
#include "stack.h"



/* Copyright 2019, Gurobi Optimization, LLC */
/*
  Sudoku example.

  The Sudoku board is a 9x9 grid, which is further divided into a 3x3 grid
  of 3x3 grids.  Each cell in the grid must take a value from 0 to 9.
  No two grid cells in the same row, column, or 3x3 subgrid may take the
  same value.

  In the MIP formulation, binary variables x[i,j,v] indicate whether
  cell <i,j> takes value 'v'.  The constraints are as follows:
    1. Each cell must take exactly one value (sum_v x[i,j,v] = 1)
    2. Each value is used exactly once per row (sum_i x[i,j,v] = 1)
    3. Each value is used exactly once per column (sum_j x[i,j,v] = 1)
    4. Each value is used exactly once per 3x3 subgrid (sum_grid x[i,j,v] = 1)

  Input datasets for this example can be found in examples/data/sudoku*.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gurobi_c.h"


void freeGurobi(GRBenv   *env, GRBmodel *model){
    /* Free model */

    GRBfreemodel(model);

    /* Free environment */

    GRBfreeenv(env);
}

int ILP_Validation(SudokuCell*** board, int row, int column, Command command, int x, int y, int* p_dig){

    GRBenv   *env   = NULL;
    GRBmodel *model = NULL;

    int total_size = row * column;

    int       ind[total_size];
    double    sol[total_size*total_size*total_size];
    double    val[total_size];
    double    lb[total_size*total_size*total_size];
    double    ub[total_size*total_size*total_size];
    char      vtype[total_size*total_size*total_size];
    int       optimstatus;
    double    objval;
    int       i, j, v, ig, jg, count, dig, cnt, isSolvable;
    int       error = 0;


    /* Create an empty model */


    for (i = 0; i < total_size; i++) {
        for (j = 0; j < total_size; j++) {
            dig = board[i][j]->digit;
            if (dig != 0){
                lb[i*total_size*total_size+j*total_size+(dig-1)] = 1; /* lower bound = 1 for fixed cell */
                ub[i*total_size*total_size+j*total_size+(dig-1)] = 1;

                for (v = 0; v < total_size; v++) {
                    if ((v+1)!=dig){
                        lb[i*total_size*total_size+j*total_size+v] = 0;
                        ub[i*total_size*total_size+j*total_size+v] = 0;
                    }
                    vtype[i*total_size*total_size+j*total_size+v] = GRB_BINARY;
                }
            }
            else {
                board[i][j]->numOfOptionalDigits = total_size;
                findThePossibleArray(board, row, column, i, j);

                if (board[i][j]->numOfOptionalDigits != 0) {
                    cnt = 0;
                    for (v = 0; v < total_size; v++) {
                        if ((v + 1) == board[i][j]->optionalDigits[cnt]) {
                            lb[i * total_size * total_size + j * total_size + v] = 0; /* lower bound = 1 for fixed cell */
                            ub[i * total_size * total_size + j * total_size + v] = 1;
                            cnt++;
                        } else {
                            lb[i * total_size * total_size + j * total_size + v] = 0;
                            ub[i * total_size * total_size + j * total_size + v] = 0;
                        }
                        vtype[i * total_size * total_size + j * total_size + v] = GRB_BINARY;
                    }
                }
                else { /*numOfOptionalDigits = 0 -> the cell <i,j> has no valid values*/
                    return 0;
                }
            }
        }
    }

    /* Create environment */

    error = GRBloadenv(&env, "sudoku.log");
    if (error) {
        printf("ERROR %d GRBloadenv(): %s\n", error, GRBgeterrormsg(env));
        return -1;
    }

    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) {
        printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));

        GRBfreeenv(env);

        return -1;
    }

    /* Create an empty model named "sudoku" */

    error = GRBnewmodel(env, &model, "sudoku", total_size*total_size*total_size, NULL, lb, ub, vtype, NULL);
    if (error) {
        printf("ERROR %d GRBnewmodel(): %s\n", error, GRBgeterrormsg(env));

        GRBfreeenv(env);

        return -1;
    }

    /*first constraint type - each cell gets a value*/

    for (i = 0; i < total_size; i++) {
        for (j = 0; j < total_size; j++) {
            for (v = 0; v < total_size; v++) {
                ind[v] = i*total_size*total_size + j*total_size + v;
                val[v] = 1.0;
            }

            error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                printf("ERROR %d 1st GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                freeGurobi(env, model);
                return -1;
            }
        }
    }

    /* second constraint type - each value must appear once in each column */

    for (v = 0; v < total_size; v++) {
        for (j = 0; j < total_size; j++) {
            for (i = 0; i < total_size; i++) {
                ind[i] = i*total_size*total_size + j*total_size + v;
                val[i] = 1.0;
            }

            error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                printf("ERROR %d 2nd GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                freeGurobi(env, model);
                return -1;
            }
        }
    }

    /* third constraint type - each value must appear once in each row */

    for (v = 0; v < total_size; v++) {
        for (i = 0; i < total_size; i++) {
            for (j = 0; j < total_size; j++) {
                ind[j] = i*total_size*total_size + j*total_size + v;
                val[j] = 1.0;
            }

            error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                printf("ERROR %d 3rd GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                freeGurobi(env, model);
                return -1;
            }
        }
    }

    /* fourth constraint type - each value must appear once in each subgrid */

    /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    /*check row and column (SUBGRID)*/

    for (v = 0; v < total_size; v++) {
        for (ig = 0; ig < row; ig++) {
            for (jg = 0; jg < column; jg++) {
                count = 0;
                for (i = ig*row; i < (ig+1)*row; i++) {
                    for (j = jg*column; j < (jg+1)*column; j++) {
                        ind[count] = i*total_size*total_size + j*total_size + v;
                        val[count] = 1.0;
                        count++;
                    }
                }

                error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
                if (error) {
                    printf("ERROR %d 4th GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                    freeGurobi(env, model);
                    return -1;
                }
            }
        }
    }

    /* Optimize model */

    error = GRBoptimize(model);
    if (error) {
        printf("ERROR %d GRBoptimize(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        return -1;
    }

    /* Write model to 'sudoku.lp' */

    error = GRBwrite(model, "sudoku.lp");
    if (error) {
        printf("ERROR %d GRBwrite(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        return -1;
    }

    /* Capture solution information */

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) {
        printf("ERROR %d GRBgetintattr(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        return -1;
    }

    /*error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    if (error) goto QUIT;*/


    /* get the solution - the assignment to each variable */
    /* 3-- number of variables, the size of "sol" should match */
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, total_size*total_size*total_size, sol);
    if (error) {
        printf("ERROR %d GRBgetdblattrarray(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        return -1;
    }

    /* print results */
    printf("\nOptimization complete\n");

    /* solution found */
    if (optimstatus == GRB_OPTIMAL){
        isSolvable = 1;
        printf("Optimal objective: %.4e\n", objval);
        if (command == HINT){
            for (v=0; v<total_size; v++){
                if (sol[x*total_size*total_size + y*total_size + v] == 1.0){
                    *(p_dig) = v+1;
                }
            }
        }
    }

    /* no solution found */
    else if (optimstatus == GRB_INF_OR_UNBD){
        isSolvable = 0;
        printf("Model is infeasible or unbounded\n");
    }
    /* error or calculation stopped */
    else{
        isSolvable = 0;
        printf("Optimization was stopped early\n");
    }

    printf("\n");

    freeGurobi(env, model);

    return isSolvable;
}





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
        printMallocFailedAndExit();
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
        printMallocFailedAndExit();
    }
    fixedBoard = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if(fixedBoard == NULL){
        printMallocFailedAndExit();
    }
    firstEmptyCell = (Cell*)malloc(sizeof(Cell));
    if(firstEmptyCell == NULL){
        printMallocFailedAndExit();
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
        printMallocFailedAndExit();
    }
    sol = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if(sol == NULL){
        printMallocFailedAndExit();
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

