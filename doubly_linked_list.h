
#ifndef HW4_DOUBLY_LINKED_LIST_H
#define HW4_DOUBLY_LINKED_LIST_H

#include "solver.h"

typedef struct {
    int before;
    int after;
    Cell* cell;
}Move;

struct Node {
    Move* move;
    struct Node *next;
    struct Node *prev;
};

typedef struct{
    struct Node *head;
    struct Node *last;
    struct Node *current;
}List;

List* getNewList(); /*Creates a new List and returns pointer to it.*/
struct Node* getNewNode(Move* move); /*Creates a new Node and returns pointer to it.*/
Move* getNewMove(int x, int y, int value, int z); /*Creates a new Move and returns pointer to it.*/
int isListEmpty(List* list);
int length(List* list);
void insertAtTail(List* list, Move* move); /*Inserts a Node at tail of doubly linked list*/
void moveToNext(List* list);
void moveToPrev(List* list);
int hasNext(List* list);
int hasPrev(List* list);
void moveToStart(List* list);
void freeNode(struct Node* node);
void deleteLast(List* list); /*delete the node at the last location*/
void freeList(List *list);
Move* getCurrentMove(List* list);

#endif
