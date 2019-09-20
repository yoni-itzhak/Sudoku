#ifndef HW4_STACK_H
#define HW4_STACK_H

#include "solver.h"
#include "structs.h"

/**
 * stack summary
 *
 * A container that takes care of the stack data structure
 *
 */

Stack* newStack(int capacity); /*initialize stack*/
int size(Stack *stack); /*return the size of the stack*/
Stack* newStack(int capacity);
int size(Stack *stack);
void push(Stack *stack, StackItem* stackItem); /*add an element x in the stack*/
StackItem* peek(Stack *stack); /*return top element in a stack*/
void pop(Stack *stack, int total_size); /*pop top element from the stack*/

#endif
