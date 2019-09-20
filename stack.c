#include "stack.h"
#include "solver.h"
#include "main_aux.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * stack summary
 *
 * A container that takes care of the stack data structure
 *
 */

/* creates new Stack*/
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

/*returns the size of the Stack*/
int size(Stack *stack){
    return stack->top + 1;
}

/* adds a StackItem at the top of the Stack*/
void push(Stack *stack, StackItem* stackItem){
    stack->items[++stack->top] = stackItem; /*add an element and increments the top index*/
}

/* gets a StackItem at the top of the Stack*/
StackItem* peek(Stack *stack){
    return stack->items[stack->top];
}

/*pop the top element from the stack*/
void pop(Stack *stack, int total_size){
    freeStackItem(stack->items[stack->top], total_size);
    stack->top--;
    /* decrement stack size by 1 and (optionally) return the popped element*/
}

