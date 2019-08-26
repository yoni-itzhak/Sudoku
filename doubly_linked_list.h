
#ifndef HW4_DOUBLY_LINKED_LIST_H
#define HW4_DOUBLY_LINKED_LIST_H


#include "structs.h"

List* getNewList(); /*Creates a new List and returns pointer to it.*/
struct Node* getNewNode(Move** arrMove, int arrSize); /*Creates a new Node and returns pointer to it.*/
Move* getNewMove(int x, int y, int beforeValue, int afterValue, int beforeErroneous, int afterErroneous); /*Creates a new Move and returns pointer to it.*/
int isListEmpty(List* list);
/*int length(List* list);*/
void insertAtTail(List* list, Move** arrMove, int arrSize); /*Inserts a Node at tail of doubly linked list*/
void moveToNext(List* list);
void moveToPrev(List* list);
int hasNext(List* list);
int hasPrev(List* list);
/*void moveToStart(List* list);*/

void deleteLast(List* list); /*delete the node at the last location*/

struct Node* getCurrentMove(List* list);

#endif
