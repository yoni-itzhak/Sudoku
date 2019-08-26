#ifndef SUDOKU_STRUCTS_H
#define SUDOKU_STRUCTS_H


typedef struct{
    int x;
    int y;
}Cell;

typedef enum{
    INIT,
    EDIT,
    SOLVE
}Mode;

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
    int cnt_erroneous; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    int* optionalDigits;
    int numOfOptionalDigits; /*initialized to total_size and will save the length of the "optionalDigits" array*/
    /*int hasSingleLegalValue; only for "autofill" command. initialized to 0. will be 1 if the cell has only single legal value*/
}SudokuCell;


typedef struct{
    SudokuCell*** currentState;
    SudokuCell*** solution;
    int row;
    int column;
    int total_size;
    int cntFilledCell;
    int cntErroneousCells; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    List* list; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    int markErrors; /*remember to take care of this field @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    Mode mode; /*remember to take care of this field. maybe should be a separate parameter @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
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



void freeBoard(SudokuCell*** board, int total_size);
void freeSudokuMemory(Sudoku *sudoku);
void freeCellsArray(Cell **arr, int arr_size);

void freeStackItem(StackItem* stackItem, int total_size); /*free the allocated memory of StackItem*/
void freeStack(Stack* stack, int total_size); /*free the allocated memory of Stack*/
void freeNode(struct Node* node);
void freeList(List *list);

#endif

