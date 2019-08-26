

#ifndef HW4_STACK_H
#define HW4_STACK_H

#include "solver.h"
#include "structs.h"

/*Data structure for stack : */




Stack* newStack(int capacity); /*initialize stack*/
int size(Stack *stack); /*return the size of the stack*/
int isStackEmpty(Stack *stack); /*check if the stack is empty or not*/
int isFull(Stack *stack); /*check if the stack is full or not*/
void push(Stack *stack, StackItem* stackItem); /*add an element x in the stack*/
StackItem* peek(Stack *stack); /*return top element in a stack*/
StackItem* pop(Stack *stack); /*pop top element from the stack*/

#endif
