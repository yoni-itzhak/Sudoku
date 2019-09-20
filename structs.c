#include "structs.h"
#include "stack.h"
#include "doubly_linked_list.h"
#include "main_aux.h"
#include <stdlib.h>

/**
 * structs summary
 *
 * A container that takes care of the different commonly structs and enums in the program
 *
 */

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

/*frees the Sudoku allocated memory*/
void freeSudokuMemory(Sudoku *sudoku){
    if (sudoku->justStarted != 1){
        freeBoard (sudoku->currentState, sudoku->total_size);
        freeList(sudoku->list);
    }
    free(sudoku);
}

/*
 * @params - function receives SudokuCell*** board and the Sudoku size.
 *
 * The function frees all board's allocated memory.
 *
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

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Cell function*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/*frees the Cells array allocated memory*/
void freeCellsArray(Cell **arr, int arr_size){
    int i;
    for(i=0;i<arr_size;++i){
        free(arr[i]);
    }
    free(arr);
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Stack functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/*frees the StackItem allocated memory*/
void freeStackItem(StackItem* stackItem, int total_size){
    freeBoard(stackItem->board, total_size);
    free(stackItem->currentEmptyCell);
    free(stackItem);
}

/*frees the Stack allocated memory*/
void freeStack(Stack* stack, int total_size){
    while(size(stack)>0){
        pop(stack, total_size);
    }
    free(stack->items);
    free(stack);
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Linked list functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

/*frees the array of Moves allocated memory*/
void freeArrMove(struct Node* node){
    int i;
    for (i=0; i< node->arrSize; i++){
        free(node->arrMove[i]->cell);
        free(node->arrMove[i]);
    }
    free(node->arrMove);
}

/*frees the Node allocated memory*/
void freeNode(struct Node* node){
    freeArrMove(node);
    free(node);
}

/*frees the List allocated memory*/
void freeList(List *list){
    while (!isListEmpty(list)){
        deleteLast(list);
    }
    free(list);
}
