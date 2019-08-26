
#include "structs.h"
#include "stack.h"
#include "doubly_linked_list.h"
#include <stdlib.h>


void freeSudokuMemory(Sudoku *sudoku){
    if (sudoku->justStarted != 1){
        freeBoard (sudoku->currentState, sudoku->total_size);
        /*freeBoard (sudoku->solution, sudoku->total_size);*/
        freeList(sudoku->list);
    }
    free(sudoku);
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

void freeCellsArray(Cell **arr, int arr_size){
    int i=0;
    for(;i<arr_size;++i){
        free(arr[i]);
    }
    free(arr);
}

void freeStackItem(StackItem* stackItem, int total_size){
    freeBoard(stackItem->board, total_size);
    free(stackItem->currentEmptyCell);
    free(stackItem);
}

void freeStack(Stack* stack, int total_size){
    while(size(stack)>0){
        freeStackItem(pop(stack), total_size);
    }
    free(stack->items);
    free(stack);
}

void freeArrMove(struct Node* node){
    int i;
    for (i=0; i< node->arrSize; i++){
        free(node->arrMove[i]->cell);
        free(node->arrMove[i]);
    }
    free(node->arrMove);
}

void freeNode(struct Node* node){
    freeArrMove(node);
    free(node);
}

void freeList(List *list){
    while (!isListEmpty(list)){
        deleteLast(list);
    }
    free(list);
}
