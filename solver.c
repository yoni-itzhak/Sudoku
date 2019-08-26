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


void allocateGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype, int total_size){
    int num_all_vars = total_size*total_size*total_size;
    (*ind) = (int*)malloc(total_size* sizeof(int));
    (*sol) = (double*)malloc(num_all_vars* sizeof(double));
    (*val) = (double*)malloc(total_size*sizeof(double));
    (*lb) = (double*)malloc(num_all_vars* sizeof(double));
    (*ub) = (double*)malloc(num_all_vars* sizeof(double));
    (*vtype) = (char*)malloc(num_all_vars* sizeof(char));
    if ((*ind)==NULL || (*sol)==NULL || (*val)==NULL || (*lb)==NULL || (*ub)==NULL || (*vtype)==NULL){
        printMallocFailedAndExit();
    }
}

void freeGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype){
    free(*ind);
    free(*sol);
    free(*val);
    free(*lb);
    free(*ub);
    free(*vtype);
}

void freeGurobi(GRBenv   *env, GRBmodel *model){
    GRBfreemodel(model);  /* Free model */
    GRBfreeenv(env); /* Free environment /*/
}

/*int ILP_Validation(SudokuCell*** board, int row, int column, Command command, int x, int y, int* p_dig){
    if (command == RESET && board == NULL && row==column && x==y && p_dig==NULL){
        return 1;
    }
    return 0;
}*/

int ILP_Validation(SudokuCell*** board, int row, int column, Command command, int x, int y, int* p_dig){

    GRBenv   *env   = NULL;
    GRBmodel *model = NULL;

    int total_size = row * column;

    int       *ind;
    double    *sol, *val, *lb, *ub;
    char      *vtype;
    int       optimstatus;
    double    objval = 0.0;
    int       i, j, v, ig, jg, count, dig, cnt, isSolvable;
    int       error = 0;

    allocateGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, total_size);
    /* Create an empty model /*/
    for (i = 0; i < total_size; i++) {
        for (j = 0; j < total_size; j++) {
            dig = board[i][j]->digit;
            if (dig != 0){
                lb[i*total_size*total_size+j*total_size+(dig-1)] = 1; /*/ lower bound = 1 for fixed cell /*/
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
                            lb[i * total_size * total_size + j * total_size + v] = 0; /*/ lower bound = 1 for fixed cell /*/
                            ub[i * total_size * total_size + j * total_size + v] = 1;
                            cnt++;
                        } else {
                            lb[i * total_size * total_size + j * total_size + v] = 0;
                            ub[i * total_size * total_size + j * total_size + v] = 0;
                        }
                        vtype[i * total_size * total_size + j * total_size + v] = GRB_BINARY;
                    }
                }
                else { /*/numOfOptionalDigits = 0 -> the cell <i,j> has no valid values/*/
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
                    return 0;
                }
            }
        }
    }

    /*/ Create environment /*/

    error = GRBloadenv(&env, "sudoku.log");
    if (error) {
        printf("ERROR %d GRBloadenv(): %s\n", error, GRBgeterrormsg(env));
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }

    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) {
        printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }

    /*/ Create an empty model named "sudoku" /*/

    error = GRBnewmodel(env, &model, "sudoku", total_size*total_size*total_size, NULL, lb, ub, vtype, NULL);
    if (error) {
        printf("ERROR %d GRBnewmodel(): %s\n", error, GRBgeterrormsg(env));
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }

    /*/first constraint type - each cell gets a value/*/

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
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
                return -1;
            }
        }
    }

    /*/ second constraint type - each value must appear once in each column /*/

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
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
                return -1;
            }
        }
    }

    /*/ third constraint type - each value must appear once in each row /*/

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
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
                return -1;
            }
        }
    }

    /*/ fourth constraint type - each value must appear once in each subgrid /*/

    /*/@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@/*/
    /*/check row and column (SUBGRID)/*/

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
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
                    return -1;
                }
            }
        }
    }

    /*/ Optimize model /*/

    error = GRBoptimize(model);
    if (error) {
        printf("ERROR %d GRBoptimize(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }

    /*/ Write model to 'sudoku.lp' /*/

    error = GRBwrite(model, "sudoku.lp");
    if (error) {
        printf("ERROR %d GRBwrite(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }

    /*/ Capture solution information /*/

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) {
        printf("ERROR %d GRBgetintattr(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }

    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    if (error) {
        printf("ERROR %d GRBgettdblattr(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }


    /*/ get the solution - the assignment to each variable /*/
    /*/ 3-- number of variables, the size of "sol" should match /*/
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, total_size*total_size*total_size, sol);
    if (error) {
        printf("ERROR %d GRBgetdblattrarray(): %s\n", error, GRBgeterrormsg(env));
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
        return -1;
    }

    /*/ print results /*/
    printf("\nOptimization complete\n");

    /*/ solution found /*/
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

    /*/ no solution found /*/
    else if (optimstatus == GRB_INF_OR_UNBD){
        isSolvable = 0;
        printf("Model is infeasible or unbounded\n");
    }
    /*/ error or calculation stopped /*/
    else{
        isSolvable = 0;
        printf("Optimization was stopped early\n");
    }
    printf("\n");
    freeGurobi(env, model);
    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype);
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
    SudokuCell*** newBoard = NULL;
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
    int total_size = sudoku->total_size,i=x,j=y, cntSolution=0;
    SudokuCell*** tmpItemBoard = NULL; /*will be malloc in "updateCellAndOptionalDigits" func*/
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
