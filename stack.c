
#include "stack.h"
#include "solver.h"
#include "main_aux.h"
#include <stdio.h>
#include <stdlib.h>

Stack* newStack(int capacity){
    Stack *stack = (Stack*)malloc(sizeof(Stack));
    if (stack == NULL){
        printMallocFailedAndExit();
    }
    stack->maxsize = capacity;
    stack->top = -1;
    stack->items = (StackItem**)malloc(sizeof(StackItem*) * capacity);
    if (stack->items == NULL){
        printMallocFailedAndExit();
    }
    return stack;
}

int size(Stack *stack){
    return stack->top + 1;
}

int isStackEmpty(Stack *stack){
    return stack->top == -1;	/*or return size(pt) == 0;*/
}

int isFull(Stack *stack){
    return stack->top == stack->maxsize - 1;	/* or return size(pt) == pt->maxsize;*/
}

void push(Stack *stack, StackItem* stackItem){
    if (isFull(stack)){ /*check if stack is already full. Then inserting an element would lead to stack overflow*/
        printf("OverFlow\nProgram Terminated\n");
        exit(EXIT_FAILURE);
    }
    stack->items[++stack->top] = stackItem; /*add an element and increments the top index*/
}

StackItem* peek(Stack *stack){
    if (!isStackEmpty(stack)) /*check for empty stack*/
        return stack->items[stack->top];
    else
        exit(EXIT_FAILURE);
}

StackItem* pop(Stack *stack){
    if (isStackEmpty(stack)){ /*check for stack underflow*/
        printf("UnderFlow\nProgram Terminated\n");
        exit(EXIT_FAILURE);
    }
    return stack->items[stack->top--]; /* decrement stack size by 1 and (optionally) return the popped element*/
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