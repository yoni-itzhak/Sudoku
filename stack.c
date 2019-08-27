
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
    }
    stack->items[++stack->top] = stackItem; /*add an element and increments the top index*/
}

StackItem* peek(Stack *stack){
    if (!isStackEmpty(stack)) /*check for empty stack*/
        return stack->items[stack->top];
    else{
        printf("empty stack\nProgram Terminated\n");
        return NULL;
    }
}

void pop(Stack *stack, int total_size){
    if (isStackEmpty(stack)){ /*check for stack underflow*/
        printf("UnderFlow\nProgram Terminated\n");
    }
    freeStackItem(stack->items[stack->top], total_size);
    stack->top--;
    /* decrement stack size by 1 and (optionally) return the popped element*/
}

