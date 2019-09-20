#ifndef SUDOKU_STRUCTS_H
#define SUDOKU_STRUCTS_H

/**
 * structs summary
 *
 * A container that takes care of the different commonly structs and enums in the program
 *
 */

typedef enum{
    INIT,
    EDIT,
    SOLVE
}Mode;

typedef struct{
    int x;
    int y;
}Cell;

typedef struct {
    int beforeValue;
    int afterValue;
    int beforeErroneous;
    int afterErroneous;
    Cell* cell;
}Move;

struct Node {
    Move** arrMove;
    int arrSize;
    struct Node *next;
    struct Node *prev;
};

typedef struct{
    struct Node *head;
    struct Node *last;
    struct Node *current;
}List;

typedef struct{
    int digit;
    int is_fixed;
    int cnt_erroneous;
    int* optionalDigits;
    int numOfOptionalDigits; /*initialized to total_size and will save the length of the "optionalDigits" array*/
}SudokuCell;


typedef struct{
    SudokuCell*** currentState;
    int row;
    int column;
    int total_size;
    int cntFilledCell;
    int cntErroneousCells;
    List* list;
    int markErrors;
    Mode mode;
    int justStarted; /* 1 if this the first game*/
}Sudoku;


typedef struct{
    SudokuCell*** board;
    Cell* currentEmptyCell;
}StackItem;

typedef struct{
    int maxsize; /* define max capacity of Stack*/
    int top;
    StackItem** items; /*array of stackItem's pointers*/
}Stack;


typedef struct{
    int val;
    float probability;
}WeightedCell;

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Sudoku and SudokuCell functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void createEmptyBoard(SudokuCell*** board, int total_size);
void freeSudokuMemory(Sudoku *sudoku);
void freeBoard(SudokuCell*** board, int total_size);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Cell function*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void freeCellsArray(Cell **arr, int arr_size);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Stack functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void freeStackItem(StackItem* stackItem, int total_size); /*free the allocated memory of StackItem*/
void freeStack(Stack* stack, int total_size); /*free the allocated memory of Stack*/

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/* Linked list functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void freeArrMove(struct Node* node);
void freeNode(struct Node* node);
void freeList(List *list);

#endif

