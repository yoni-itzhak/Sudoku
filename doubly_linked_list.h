#ifndef HW4_DOUBLY_LINKED_LIST_H
#define HW4_DOUBLY_LINKED_LIST_H

#include "structs.h"

/**
 * doubly_linked_list summary
 *
 * A container that takes care of the doubly linked list data structure
 *
 */

int isDummyNode(List* list);
void addDummyNode(List* list);
List* getNewList();
struct Node* getNewNode(Move** arrMove, int arrSize);
Move* getNewMove(int x, int y, int beforeValue, int afterValue, int beforeErroneous, int afterErroneous);
int isListEmpty(List* list);
void insertAtTail(List* list, Move** arrMove, int arrSize);
void moveToNext(List* list);
void moveToPrev(List* list);
int hasNext(List* list);
void deleteFromCurrent(List* list);
void deleteLast(List* list);
struct Node* getCurrentMove(List* list);

#endif
