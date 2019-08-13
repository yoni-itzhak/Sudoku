

#ifndef HW4_STACK_H
#define HW4_STACK_H

#include "solver.h"

/*Data structure for stack : */

typedef struct{
    SudokuCell*** board;
    Cell* currentEmptyCell;
}StackItem;

typedef struct{
    int maxsize; /* define max capacity of Stack*/
    int top;
    StackItem** items; /*array of stackItem's pointers*/
}Stack;

void freeStackItem(StackItem* stackItem, int total_size); /*free the allocated memory of StackItem*/
void freeStack(Stack* stack, int total_size); /*free the allocated memory of Stack*/
Stack* newStack(int capacity); /*initialize stack*/
int size(Stack *stack); /*return the size of the stack*/
int isStackEmpty(Stack *stack); /*check if the stack is empty or not*/
int isFull(Stack *stack); /*check if the stack is full or not*/
void push(Stack *stack, StackItem* stackItem); /*add an element x in the stack*/
StackItem* peek(Stack *stack); /*return top element in a stack*/
StackItem* pop(Stack *stack); /*pop top element from the stack*/

#endif
