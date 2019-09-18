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


#include <stdio.h>
#include <stdlib.h>

#include "gurobi_c.h"
#include "structs.h"

#define ERRORGUR "Error: Gurobi optimizer has failed\n"

void allocateGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype, int total_size, double** obj, int isLP){
    int num_all_vars = total_size*total_size*total_size;
    (*ind) = (int*)malloc(total_size* sizeof(int));
    (*sol) = (double*)malloc(num_all_vars* sizeof(double));
    (*val) = (double*)malloc(total_size*sizeof(double));
    (*lb) = (double*)malloc(num_all_vars* sizeof(double));
    (*ub) = (double*)malloc(num_all_vars* sizeof(double));
    (*vtype) = (char*)malloc(num_all_vars* sizeof(char));
    if (isLP == 1){
        (*obj) = (double*)malloc(num_all_vars* sizeof(double));
        if ( (*obj) == NULL){
            printMallocFailedAndExit();
        }
    }
    if ((*ind)==NULL || (*sol)==NULL || (*val)==NULL || (*lb)==NULL || (*ub)==NULL || (*vtype)==NULL){
        printMallocFailedAndExit();
    }
}

void freeGurobi(GRBenv   *env, GRBmodel *model) {
    if (model != NULL) {
        GRBfreemodel(model); /* Free model */
    }
    if (env != NULL) {
        GRBfreeenv(env); /* Free environment */
    }
}

void freeGurobiArrays(int** ind, double** sol, double** val, double** lb, double** ub, char** vtype,double** obj, int isLP){
    free(*ind);
    free(*sol);
    free(*val);
    free(*lb);
    free(*ub);
    free(*vtype);
    if (isLP){
        free(*obj);
    }
}

int GRSolver(Sudoku* sudoku, SudokuCell*** board, int isLP, int row, int column, Command command, int x, int y, float threshold, int* p_dig){

    GRBenv *env = NULL;
    GRBmodel *model = NULL;

    int total_size = row * column;
    int N = total_size;
    int *ind;
    double *sol, *val, *lb, *ub, *obj; /* TODO: allocate and free 'obj' */
    char *vtype;
    int status, k, random;
    double objval = 0.0;
    int i, j, v, ig, jg, count, isSolvable, possible_sol_arr_size, chosen_val, numOfMoves = 0;
    int error = 0;
    Move **arrMove;
    WeightedCell** possible_sol_arr;

    /* allocate memory */
    if (isLP){
        allocateGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, total_size, &obj, isLP);
    }
    else{ /*TODO: we can remove the else and do it together*/
        allocateGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, total_size, NULL, isLP);
    }

    /*create env */

    error = GRBloadenv(&env, "sudoku.log");
    if (error) {
        if (command != GENERATE){
            printf(ERRORGUR);
            printf("1\n");
        }
        freeGurobi(env, model);
        if (isLP){
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
        }
        else{
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        }
        return -1;
    }

    /*cancel outputs*/

    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) {
        if (command != GENERATE){
            printf(ERRORGUR);
            printf("2\n");
        }
        freeGurobi(env, model);
        if (isLP){
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
        }
        else{
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        }
        return -1;
    }

    /*create objective function*/

    if (isLP){ /* for ILP */
        for (i=0;i<N;i++){
            for (j=0;j<N;j++){
                for (k=1;k<=N;k++){
                    random= (rand()) % (N+1);
                    if(random==0)
                        random++;
                    obj[i*N*N+j*N+k-1]=random;
                }
            }
        }
    }

    /* create model */

    for (i=0;i<total_size;i++){
        for (j=0;j<total_size;j++){
            for (k=1;k<=total_size;k++){
                if (board[i][j]->digit == k) /*@@@@@@@@@@*/{
                    lb[(j*total_size*total_size) + (i*total_size) + k-1] = 1; /*TODO: change i to j*/
                }
                else{
                    lb[(j*total_size*total_size) + (i*total_size) + k-1] = 0;
                }
                if (!isLP){
                    vtype[(j*total_size*total_size) + (i*total_size) + k-1] = GRB_BINARY;
                }
                else{
                    vtype[(j*total_size*total_size) + (i*total_size) + k-1] = GRB_CONTINUOUS;
                }
                ub[(j*total_size*total_size) + (i*total_size) + k-1] = 1;
            }
        }
    }

    /* create new model */
    if (isLP){
        error = GRBnewmodel(env, &model, "sudoku", N * N * N, obj, lb, ub, vtype, NULL );
    }
    else{
        error = GRBnewmodel(env, &model, "sudoku", N * N * N, NULL, lb, ub, vtype, NULL );
    }
    if (error) {
        if (command != GENERATE){
            printf(ERRORGUR);
            printf("3\n");
        }
        freeGurobi(env, model);
        if (isLP){
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
        }
        else{
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        }
        return -1;
    }

    if (isLP){
        /*set objective max*/
        error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
        if (error) {
            if (command != GENERATE){
                printf(ERRORGUR);
                printf("4\n");
            }
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
            return -1;
        }
    }




    /*constrains*/

    /*first constrain - cell*/

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k < N; k++) {
                ind[k] = j * N * N + i * N + k;
                val[k] = 1.0;
            }
            error = GRBaddconstr(model, N, ind, val, GRB_EQUAL, 1.0, NULL );
            if (error) {
                if (command != GENERATE){

                    printf(ERRORGUR);
                    printf("5\n");
                }
                freeGurobi(env, model);
                if (isLP){
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
                }
                else{
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
                }
                return -1;
            }
        }
    }

    /* third constrain - column */

    for (k = 0; k < N; k++) {
        for (j = 0; j < N; j++) {
            for (i = 0; i < N; i++) {
                ind[i] = j * N * N + i * N + k;
                val[i] = 1.0;
            }
            error = GRBaddconstr(model, N, ind, val, GRB_EQUAL, 1.0, NULL );
            if (error) {
                if (command != GENERATE){
                    printf(ERRORGUR);
                    printf("7\n");
                }
                freeGurobi(env, model);
                if (isLP){
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
                }
                else{
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
                }
                return -1;
            }
        }
    }

    /*second constrain - row*/

    for (k = 0; k < N; k++) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                ind[j] = j * N * N + i * N + k;
                val[j] = 1.0;
            }
            error = GRBaddconstr(model, N, ind, val, GRB_EQUAL, 1.0, NULL );
            if (error) {
                if (command != GENERATE){
                    printf(ERRORGUR);
                    printf("6\n");
                }
                freeGurobi(env, model);
                if (isLP){
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
                }
                else{
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
                }
                return -1;
            }
        }
    }


    /* fourth constrain - block */

    for (k = 0; k < total_size; k++) {
        for (ig = 0; ig < row; ig++) {
            for (jg = 0; jg < column; jg++) {
                count = 0;
                for (i = ig*column; i < (ig+1)*column; i++) {
                    for (j = jg*row; j < (jg+1)*row; j++) {
                        ind[count] = j * N * N + i * N + k;
                        val[count] = 1.0;
                        count++;
                    }
                }
                error = GRBaddconstr(model, N, ind, val, GRB_EQUAL, 1.0, NULL );
                if (error) {
                    printf("ERROR %d 4th GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));
                    if (command != GENERATE) {
                        printf(ERRORGUR);
                        printf("8\n");
                    }
                    freeGurobi(env, model);
                    if (isLP) {
                        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
                    } else {
                        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
                    }
                    return -1;
                }
            }
        }
    }

    /* optimize model */

    error = GRBoptimize(model);
    if (error) {
        if (command != GENERATE) {
            printf(ERRORGUR);
            printf("9\n");
        }
        freeGurobi(env, model);
        if (isLP) {
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        } else {
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        }
        return -1;
    }

    /* write model */

    error = GRBwrite(model, "sudoku.lp");
    if (error) {
        if (command != GENERATE) {
            printf(ERRORGUR);
            printf("10\n");
        }
        freeGurobi(env, model);
        if (isLP) {
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        } else {
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        }
        return -1;
    }

    /* get attribute */

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &status);
    if (error) {
        if (command != GENERATE) {
            printf(ERRORGUR);
            printf("11\n");
        }
        freeGurobi(env, model);
        if (isLP) {
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        } else {
            freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        }
        return -1;
    }
    printf("\nOptimization complete\n");
    if(status == GRB_OPTIMAL) {
        error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0,N * N * N, sol);
        if (error) {
            if (command != GENERATE) {
                printf(ERRORGUR);
                printf("12\n");
            }
            freeGurobi(env, model);
            if (isLP) {
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
            } else {
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
            }
            return -1;
        }
        printf("Optimal objective: %.4e\n", objval);
        isSolvable = 1;
    }
    else if ((status == GRB_INF_OR_UNBD) || (status == GRB_INFEASIBLE) || (status == GRB_UNBOUNDED)) {
        printf("Model is infeasible or unbounded\n");
        isSolvable = 0;
    }
    else {
        printf("Optimization was stopped early\n");
        isSolvable = -1;
    }

    if (isSolvable){
        if (isLP == 1){
            if (command == GUESS){
                arrMove = (Move **) malloc((total_size * total_size) * sizeof(Move *));
                if (arrMove == NULL) {
                    printMallocFailedAndExit();
                }
                for (i = 0; i<total_size; i++) {
                    for (j = 0; j<total_size; j++) {
                        if (board[i][j]->digit == 0) {
                            MallocAndFindPossibleSolArr(&possible_sol_arr, &possible_sol_arr_size, sol, sudoku, i, j, threshold);
                            if (possible_sol_arr_size > 0) {
                                chosen_val = choose_weighted_rand(possible_sol_arr, possible_sol_arr_size);
                                board[i][j]->digit = chosen_val;
                                (sudoku->cntFilledCell)++;
                                addMoveToArrMoveAndIncrementSize(arrMove, &numOfMoves, i, j, 0, chosen_val, 0, 0);
                            }
                            freePossibleSolArr(&possible_sol_arr, sudoku->total_size);
                            possible_sol_arr_size = 0;
                        }
                    }
                }
                addArrMoveToList(sudoku, arrMove, numOfMoves);
            }
            if (command == GUESS_HINT){
                MallocAndFindPossibleSolArr(&possible_sol_arr, &possible_sol_arr_size, sol, sudoku, x, y, threshold);
                if(possible_sol_arr_size == 0){
                    isSolvable = 0;
                }
                else{
                    printPossibleSolAar(possible_sol_arr, possible_sol_arr_size, x, y);
                }
                freePossibleSolArr(&possible_sol_arr, sudoku->total_size);

            }
        }
        else if (isLP == 0){
            if (command == VALIDATE){
                /* do VALIDATE */
            }
            if (command == HINT){
                for (v=0; v<total_size; v++){
                    if (sol[y*total_size*total_size + x*total_size + v] == 1.0){
                        *(p_dig) = v+1;
                    }
                }
            }
            if (command == GENERATE){
                for (i=0; i<total_size; i++){
                    for(j=0;j<total_size;j++){
                        for(v=0;v<total_size; v++){
                            if (sol[j * N * N + i * N + v] == 1.0){
                                board[i][j]->digit = v+1;
                            }
                        }
                    }
                }
            }
        }

    }

    /* free gurobi and co. */
    if (isLP){
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype,&obj, 1);
    }
    else{
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
    }

    return isSolvable;
}

/*
 *
 */
void valuesCounter(Sudoku* sudoku, int* counter){
    int i, j, dig;
    for (i=0; i<sudoku->total_size; i++){
        for (j=0; j<sudoku->total_size; j++){
            /*if (sudoku->currentState[i][j]->numOfOptionalDigits==0){
                continue;
            }*/
            dig = sudoku->currentState[i][j]->digit;
            if (dig != 0){
                counter[dig-1]++;
            }
        }
    }
}

/*
 *
 */
void rowCounter(Sudoku* sudoku, int* numInRows){
    int i,j,dig;
    for (i=0; i<sudoku->total_size; i++){
        for (j=0; j<sudoku->total_size; j++){
            /*if (sudoku->currentState[i][j]->numOfOptionalDigits==0){
                continue;
            }*/
            dig = sudoku->currentState[i][j]->digit;
            if (dig != 0){
                numInRows[i]++;
            }
        }
    }
}

/*
 *
 */
void colCounter(Sudoku* sudoku, int* numInCols){
    int i,j,dig;
    for (i=0; i<sudoku->total_size; i++){
        for (j=0; j<sudoku->total_size; j++){
            /*if (sudoku->currentState[i][j]->numOfOptionalDigits==0){
                continue;
            }*/
            dig = sudoku->currentState[i][j]->digit;
            if (dig != 0){
                numInCols[j]++;
            }
        }
    }
}

/*
 *
 */
void blockCounter(Sudoku* sudoku, int* numInBlocks){
    int i, j,dig, bi, bj, indBlock=0;
    for (i=0; i<sudoku->total_size; i+=sudoku->row){
        for (j=0; j<sudoku->total_size; j+=sudoku->column){
            for (bi=i; bi<i+sudoku->row; bi++){
                for (bj=j; bj<j+sudoku->column; bj++){
                    /*if (sudoku->currentState[i][j]->numOfOptionalDigits==0){
                        continue;
                    }*/
                    dig = sudoku->currentState[i][j]->digit;
                    if (dig != 0){
                        numInBlocks[indBlock]++;
                    }
                }
            }
            indBlock++;
        }
    }
}
/*
 *
 */
void objTargetFuncLP(Sudoku* sudoku, double * obj ,int numberOfVariables){
    int i,j, k, ind=0, curRow, curCol, total_size = sudoku->total_size;
    int *numInRow, *numInCol, *numInBlock, *counter;
    numInRow = (int*) malloc(sudoku->total_size * sizeof(int));
    numInCol = (int*) malloc(sudoku->total_size * sizeof(int));
    numInBlock = (int*) malloc(sudoku->total_size * sizeof(int));
    counter = (int*) malloc(sudoku->total_size * sizeof(int));
    for(i = 0 ; i <numberOfVariables;i++){
        obj[i] = 1;
    }
    for (i=0; i<total_size; i++){
        numInRow[i]=0;
        numInCol[i]=0;
        numInBlock[i]=0;
        counter[i]=0;
    }

    valuesCounter(sudoku, counter);
    rowCounter(sudoku, numInRow);
    colCounter(sudoku, numInCol);
    blockCounter(sudoku, numInBlock);

    for (i=0; i<total_size; i++){
        printf("%d\n", i);
        printf("row %d\n",numInRow[i]);
        printf("column %d\n",numInCol[i]);
        printf("block %d\n",numInBlock[i]);
        printf("counter %d\n",counter[i]);
        printf("\n\n");
    }

    for (i=0; i<sudoku->total_size ; i++){
        for (j=0; j<sudoku->total_size ; j++){
            sudoku->currentState[i][j]->numOfOptionalDigits = total_size;
            findThePossibleArray(sudoku->currentState, sudoku->row, sudoku->column, i, j);
            for (k=0; k<sudoku->total_size ; k++){
                if(isNumInArr(k+1, sudoku->currentState[i][j]->optionalDigits, sudoku->currentState[i][j]->numOfOptionalDigits)){
                    ind = i * total_size * total_size + j * total_size +k;
                    obj[ind] += counter[k];
                    obj[ind] += numInRow[i];
                    obj[ind] += numInCol[j];
                    curRow = i-(i%sudoku->row);
                    curCol = j-(j%sudoku->column);
                    obj[ind] += numInBlock[(curRow+(curCol/sudoku->column))];
                }
            }
        }
    }
    free(numInRow);
    free(numInCol);
    free(numInBlock);
    free(counter);
}






void freePossibleSolArr(WeightedCell*** possible_sol_arr, int total_size){
    int i;
    for (i=0; i<total_size; i++){
        free((*possible_sol_arr)[i]);
    }
    free(*possible_sol_arr);
}

void MallocAndFindPossibleSolArr(WeightedCell*** possible_sol_arr, int* possible_sol_arr_size, double* sol, Sudoku* sudoku, int x, int y, float threshold){
    int v, total_size, i;
    float cell_probability;
    SudokuCell*** board = sudoku->currentState;
    total_size = sudoku->total_size;
    board[x][y]->numOfOptionalDigits = total_size;
    findThePossibleArray(board, sudoku->row, sudoku->column, x, y);
    (*possible_sol_arr) = (WeightedCell**)malloc((total_size)* sizeof(WeightedCell*));
    if ((*possible_sol_arr) == NULL) {
        printMallocFailedAndExit();
    }
    for (i=0; i<total_size; i++){
        (*possible_sol_arr)[i] = (WeightedCell*)malloc(sizeof(WeightedCell));
        if ((*possible_sol_arr)[i] == NULL) {
            printMallocFailedAndExit();
        }
        (*possible_sol_arr)[i]->val=0;
        (*possible_sol_arr)[i]->probability=0;
    }
    for (v = 0; v < total_size; v++) {
        cell_probability = sol[x * total_size * total_size + y * total_size + v];
        /*printf("val: %d, prob: %f\n", v, cell_probability);*/
        if (cell_probability > 0 && cell_probability >= threshold &&
            isNumInArr(v+1, board[x][y]->optionalDigits, board[x][y]->numOfOptionalDigits)) {
            (*possible_sol_arr)[(*possible_sol_arr_size)]->val = v + 1;
            (*possible_sol_arr)[(*possible_sol_arr_size)]->probability = cell_probability;
            (*possible_sol_arr_size)++;
        }
    }
}


int choose_weighted_rand(WeightedCell** arr, int arr_size){
    int i;
    double rand_num, sum_of_score=0;
    for (i = 1; i<arr_size; i++) {
        arr[i]->probability += arr[i - 1]->probability;
    }
    sum_of_score = arr[i]->probability;
    for (i = 0; i<arr_size; i++) {
        arr[i]->probability /= sum_of_score;
    }
    rand_num = (double)(rand()/RAND_MAX);
    for (i = 0; i<arr_size; i++) {
        if (rand_num <= arr[i]->probability){
            return arr[i]->val;
        }
    }
    return 0;
}

int LP_Validation(Sudoku* sudoku, int row, int column, Command command, int x, int y, float threshold) {

    GRBenv *env = NULL;
    GRBmodel *model = NULL;

    int total_size = row * column;

    int *ind;
    double *sol, *val, *lb, *ub, *obj; /* TODO: allocate and free 'obj' */
    char *vtype;
    int optimstatus;
    double objval = 0.0;
    int i, j, v, ig, jg, count, dig, cnt, isSolvable, possible_sol_arr_size, chosen_val, numOfMoves = 0;
    int error = 0;
    SudokuCell ***board = sudoku->currentState;
    Move **arrMove;
    WeightedCell** possible_sol_arr;
    allocateGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, total_size, &obj, 1);

    objTargetFuncLP(sudoku, obj,total_size*total_size*total_size);

    /* Create an empty model /*/
    for (i = 0; i < total_size; i++) {
        for (j = 0; j < total_size; j++) {
            dig = board[i][j]->digit;
            if (dig != 0) { /* cell is "fixed" -> 'dig' is the only legal value for this cell */
                lb[i * total_size * total_size + j * total_size + (dig - 1)] = 1; /* lower bound = 1 for fixed cell */
                ub[i * total_size * total_size + j * total_size + (dig - 1)] = 1;
                obj[i * total_size * total_size + j * total_size + (dig - 1)] = 0;
                for (v = 0; v < total_size; v++) {
                    if ((v + 1) != dig) {
                        lb[i * total_size * total_size + j * total_size + v] = 0;
                        ub[i * total_size * total_size + j * total_size + v] = 0;
                        obj[i * total_size * total_size + j * total_size + v] = 0;
                    }
                    vtype[i * total_size * total_size + j * total_size + v] = GRB_CONTINUOUS;
                }
            } else {
                board[i][j]->numOfOptionalDigits = total_size;
                findThePossibleArray(board, row, column, i, j);

                if (board[i][j]->numOfOptionalDigits != 0) { /* the cell has some legal values */
                    cnt = 0;
                    for (v = 0; v < total_size; v++) {
                        if (isNumInArr((v+1), board[i][j]->optionalDigits, board[i][j]->numOfOptionalDigits)){
                            lb[i * total_size * total_size + j * total_size +v] = 0;
                            ub[i * total_size * total_size + j * total_size + v] = 1;
                            /*obj[i * total_size * total_size + j * total_size + v] = rand() % (board[i][j]->numOfOptionalDigits);*/
                            cnt++;
                        } else {
                            lb[i * total_size * total_size + j * total_size + v] = 0;
                            ub[i * total_size * total_size + j * total_size + v] = 0;
                            obj[i * total_size * total_size + j * total_size + v] = 0;
                        }
                        vtype[i * total_size * total_size + j * total_size + v] = GRB_CONTINUOUS;
                    }
                }
                    /*TODO: Maybe should not finish the function in LP_validation - enter zeros at all arrays*/
                else { /*/numOfOptionalDigits = 0 -> the cell <i,j> has no valid values/*/
                    for (v = 0; v < total_size; v++) {
                        lb[i * total_size * total_size + j * total_size + v] = 0;
                        ub[i * total_size * total_size + j * total_size + v] = 0;
                        obj[i * total_size * total_size + j * total_size + v] = 0;
                        vtype[i * total_size * total_size + j * total_size + v] = GRB_CONTINUOUS;
                    }
                }
            }
        }
    }

    error = GRBloadenv(&env, "sudoku.log");
    if (error) {
        /*printf("ERROR %d GRBloadenv(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) {
        /*printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));*/
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    /*/ Create an empty model named "sudoku" /*/

    error = GRBnewmodel(env, &model, "sudoku", total_size * total_size * total_size, obj, lb, ub, vtype, NULL);
    if (error) {
        /*printf("ERROR %d GRBnewmodel(): %s\n", error, GRBgeterrormsg(env));*/
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    /*find objective function*/

    /* Add variables */

    /* coefficients - for x,y,z (cells 0,1,2 in "obj") */

    /* variable types - for x,y,z (cells 0,1,2 in "vtype") */
    /* other options: GRB_INTEGER, GRB_CONTINUOUS */
    /*vtype[0] = GRB_BINARY; vtype[1] = GRB_BINARY; vtype[2] = GRB_BINARY;*/

    /* add variables to model */
   /*error = GRBaddvars(model, total_size * total_size * total_size, 0, NULL, NULL, NULL, obj, NULL, NULL, vtype, NULL);
    if (error) {
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }*/

    /*Change objective sense to maximization*/
    error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
    if (error) {
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }

    /* update the model - to integrate new variables */

    error = GRBupdatemodel(model);
    if (error) {
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }


    /*/first constraint type - each cell gets a value/*/

    for (i = 0; i < total_size; i++) {
        for (j = 0; j < total_size; j++) {
            for (v = 0; v < total_size; v++) {
                ind[v] = i * total_size * total_size + j * total_size + v;
                val[v] = 1.0;
            }

            error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                /*printf("ERROR %d 1st GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                freeGurobi(env, model);
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
                return -1;
            }
        }
    }

    /*/ second constraint type - each value must appear once in each column /*/

    for (v = 0; v < total_size; v++) {
        for (j = 0; j < total_size; j++) {
            for (i = 0; i < total_size; i++) {
                ind[i] = i * total_size * total_size + j * total_size + v;
                val[i] = 1.0;
            }

            error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                /*printf("ERROR %d 2nd GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                freeGurobi(env, model);
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
                return -1;
            }
        }
    }

    /*/ third constraint type - each value must appear once in each row /*/

    for (v = 0; v < total_size; v++) {
        for (i = 0; i < total_size; i++) {
            for (j = 0; j < total_size; j++) {
                ind[j] = i * total_size * total_size + j * total_size + v;
                val[j] = 1.0;
            }

            error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
            if (error) {
                /*printf("ERROR %d 3rd GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                freeGurobi(env, model);
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
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
                for (i = ig * row; i < (ig + 1) * row; i++) {
                    for (j = jg * column; j < (jg + 1) * column; j++) {
                        ind[count] = i * total_size * total_size + j * total_size + v;
                        val[count] = 1.0;
                        count++;
                    }
                }

                error = GRBaddconstr(model, total_size, ind, val, GRB_EQUAL, 1.0, NULL);
                if (error) {
                    /*printf("ERROR %d 4th GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                    freeGurobi(env, model);
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
                    return -1;
                }
            }
        }
    }

    /*/ Optimize model /*/

    error = GRBoptimize(model);
    if (error) {
        /*printf("ERROR %d GRBoptimize(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }

    /*/ Write model to 'sudoku.lp' /*/

    error = GRBwrite(model, "sudoku.lp");
    if (error) {
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }

    /*/ Capture solution information /*/

    error = GRBgetintattr(model,GRB_INT_ATTR_STATUS, &optimstatus );
    if (error) {
        /*printf("ERROR %d GRBgetintattr(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }


    /*/ get the solution - the assignment to each variable /*/
    /*/ 3-- number of variables, the size of "sol" should match /*/
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, total_size * total_size * total_size, sol);
    if (error) {
        /*printf("ERROR %d GRBgetdblattrarray(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }


    /*/ print results /*/
    printf("\nOptimization complete\n");
    printf("%d\n", optimstatus == GRB_OPTIMAL);
    /*/ solution found /*/
    if (optimstatus == GRB_OPTIMAL) {
        isSolvable = 1;
        printf("Optimal objective: %.4e\n", objval);
        if (command == GUESS_HINT) {
            MallocAndFindPossibleSolArr(&possible_sol_arr, &possible_sol_arr_size, sol, sudoku, x, y, threshold);
            if(possible_sol_arr_size == 0){
                isSolvable = 0;
            }
            else{
                printPossibleSolAar(possible_sol_arr, possible_sol_arr_size, x, y);
            }
            freePossibleSolArr(&possible_sol_arr, sudoku->total_size);
        }
        else if (command == GUESS) {
            arrMove = (Move **) malloc((total_size * total_size) * sizeof(Move *));
            if (arrMove == NULL) {
                printMallocFailedAndExit();
            }
            for (i = 0; i<total_size; i++) {
                for (j = 0; j<total_size; j++) {
                    if (board[i][j]->digit == 0) {
                        MallocAndFindPossibleSolArr(&possible_sol_arr, &possible_sol_arr_size, sol, sudoku, i, j, threshold);
                        if (possible_sol_arr_size > 0) {
                            chosen_val = choose_weighted_rand(possible_sol_arr, possible_sol_arr_size);
                            board[i][j]->digit = chosen_val;
                            (sudoku->cntFilledCell)++;
                            addMoveToArrMoveAndIncrementSize(arrMove, &numOfMoves, i, j, 0, chosen_val, 0, 0);
                        }
                        freePossibleSolArr(&possible_sol_arr, sudoku->total_size);
                        possible_sol_arr_size = 0;
                    }
                }
            }
            addArrMoveToList(sudoku, arrMove, numOfMoves);
        }
    }

        /*/ no solution found /*/
    else if (optimstatus == GRB_INFEASIBLE || optimstatus == GRB_UNBOUNDED || optimstatus == GRB_INF_OR_UNBD) {
        isSolvable = 0;
        printf("Model is infeasible or unbounded\n");
    }
        /*/ error or calculation stopped /*/
    else {
        isSolvable = 0;
        printf("Optimization was stopped early\n");
    }

    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    if (error) {
        /*printf("ERROR %d GRBgettdblattr(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
        return -1;
    }

    printf("\n");
    freeGurobi(env, model);
    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, &obj, 1);
    return isSolvable;
}

int IsBoardErroneous(SudokuCell*** board, int total_size){
    int i,j;
    for (i=0;i<total_size;i++){
        for (j=0;j<total_size;j++){
            if (isCellErroneous(board,i,j)){
                return 1;
            }
        }
    }
    return 0;
}

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
    int isErroneous;

    isErroneous = IsBoardErroneous(board, row*column);
    if (isErroneous){
        /*don't run ILP and exit function*/
    }
    /* if valid, run ILP */

    allocateGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, total_size, NULL, 0);
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
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
                    return 0;
                }
            }
        }
    }

    /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     */

    /*find objective function*/




    /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@2
     * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     */


    /*/ Create environment /*/

    error = GRBloadenv(&env, "sudoku.log");
    if (error) {
        /*printf("ERROR %d GRBloadenv(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    error = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
    if (error) {
        /*printf("ERROR %d GRBsetintattr(): %s\n", error, GRBgeterrormsg(env));*/
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    /*/ Create an empty model named "sudoku" /*/

    error = GRBnewmodel(env, &model, "sudoku", total_size*total_size*total_size, NULL, lb, ub, vtype, NULL);
    if (error) {
        /*printf("ERROR %d GRBnewmodel(): %s\n", error, GRBgeterrormsg(env));*/
        GRBfreeenv(env);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
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
                /*printf("ERROR %d 1st GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                freeGurobi(env, model);
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
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
                /*printf("ERROR %d 2nd GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                freeGurobi(env, model);
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
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
                /*printf("ERROR %d 3rd GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                freeGurobi(env, model);
                freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
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
                    /*printf("ERROR %d 4th GRBaddconstr(): %s\n", error, GRBgeterrormsg(env));*/
                    freeGurobi(env, model);
                    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
                    return -1;
                }
            }
        }
    }

    /*/ Optimize model /*/

    error = GRBoptimize(model);
    if (error) {
        /*printf("ERROR %d GRBoptimize(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    /*/ Write model to 'sudoku.lp' /*/

    error = GRBwrite(model, "sudoku.lp");
    if (error) {
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    /*/ Capture solution information /*/

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
    if (error) {
        /*printf("ERROR %d GRBgetintattr(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }


    /*/ get the solution - the assignment to each variable /*/
    /*/ 3-- number of variables, the size of "sol" should match /*/
    error = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, total_size*total_size*total_size, sol);
    if (error) {
        /*printf("ERROR %d GRBgetdblattrarray(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
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
        else if(command == GENERATE){
            for (i=0; i<total_size; i++){
                for(j=0;j<total_size;j++){
                    for(v=0;v<total_size; v++){
                        if (sol[i*total_size*total_size + j*total_size + v] == 1.0){
                            board[i][j]->digit = v+1;
                        }
                    }
                }
            }
        }
    }

        /*/ no solution found /*/
    else if (optimstatus == GRB_INFEASIBLE || optimstatus == GRB_UNBOUNDED || optimstatus == GRB_INF_OR_UNBD){
        isSolvable = 0;
        printf("Model is infeasible or unbounded\n");
    }
        /*/ error or calculation stopped /*/
    else{
        isSolvable = 0;
        printf("Optimization was stopped early\n");
    }

    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
    if (error) {
        /*printf("ERROR %d GRBgettdblattr(): %s\n", error, GRBgeterrormsg(env));*/
        freeGurobi(env, model);
        freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
        return -1;
    }

    printf("\n");
    freeGurobi(env, model);
    freeGurobiArrays(&ind, &sol, &val, &lb, &ub, &vtype, NULL, 0);
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

void copyBoardValues(SudokuCell*** fromBoard, SudokuCell*** toBoard, int total_size, int copyFixedCellsOnly){
    int i,j, numOfOptionalDigs;
    for (i=0;i<total_size;i++){
        for (j=0;j<total_size;j++){
            toBoard[i][j]->digit=fromBoard[i][j]->digit;
            toBoard[i][j]->is_fixed=fromBoard[i][j]->is_fixed;
            if (copyFixedCellsOnly == 1){
                if (isFixed(fromBoard, i, j)){
                    toBoard[i][j]->cnt_erroneous=0;
                }
                else{
                    toBoard[i][j]->digit=0;
                }
                continue;
            }
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

SudokuCell*** copyBoard(SudokuCell*** board, int total_size, int copyFixedCellsOnly){
    SudokuCell*** newBoard = (SudokuCell***)malloc(total_size* sizeof(SudokuCell**));
    if (newBoard == NULL){
        printMallocFailedAndExit();
    }
    createEmptyBoard(newBoard, total_size);
    copyBoardValues(board, newBoard, total_size,copyFixedCellsOnly);
    return newBoard;
}

void updateCellAndOptionalDigits(StackItem* stackItem,SudokuCell**** tmpItemBoard, int total_size, int i,int j){
    int dig=0;
    (*tmpItemBoard) = copyBoard(stackItem->board, total_size, 0); /* 0 for copying the all board */ /*@@need to malloc tmpItemBoard + need to copy all the fields in each cell*/
    dig = (*tmpItemBoard)[i][j]->optionalDigits[0];
    (*tmpItemBoard)[i][j]->numOfOptionalDigits -= 1;
    deleteDigitFromArr((*tmpItemBoard), i, j, dig); /*delete the number from the array*/
    (*tmpItemBoard)[i][j]->digit = dig; /*update the board with the chosen digit*/
}
/*TODO: ask Yoni to make separate function that allocate the memory*/
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
    stackItem->board = copyBoard(fixedBoard, total_size, 0); /* 0 for copying the all board */
    stackItem->board[i][j]->numOfOptionalDigits=sudoku->total_size;
    findThePossibleArray(stackItem->board,sudoku->row,sudoku->column, i, j);
    updateCurrentEmptyCell(currentEmptyCell,x,y);
    pushToGetToNextCell(sudoku,stack,stackItem,currentEmptyCell,x,y, 1);
    while (canBacktrack(stack,x,y)) { /*while the array of the first empty cell isn't empty*/
        printf("here1\n");
        while (peek(stack)->board[i][j]->numOfOptionalDigits>0){ /*while the array of the current empty cell isn't empty*/
            printf("here2\n");
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
            break;
        }
    }
}
