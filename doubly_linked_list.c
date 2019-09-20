#include "doubly_linked_list.h"
#include "main_aux.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * doubly_linked_list summary
 *
 * A container that takes care of the doubly linked list data structure
 *
 */

/*checks if current node is dummy node*/
int isDummyNode(List* list){
    if (list->current->arrSize == -1){
        return 1;
    }
    return 0;
}

/*adds a dummy node to the list*/
void addDummyNode(List* list){
    insertAtTail(list, NULL, -1);
}

/*Creates a new List and returns pointer to it.*/
List* getNewList(){
    List* newList = (List*)malloc(sizeof(List));
    if (newList==NULL){
        printMallocFailedAndExit();
    }
    newList->head = NULL;
    newList->last = NULL;
    newList->current = NULL;
    addDummyNode(newList);
    return newList;
}

/*Creates a new Node and returns pointer to it.*/
struct Node* getNewNode(Move** arrMove, int arrSize) {
    struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));
    if (newNode==NULL){
        printMallocFailedAndExit();
    }
    newNode->arrMove = arrMove;
    newNode->arrSize=arrSize;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}
/*Creates a new Move and returns pointer to it.*/
Move* getNewMove(int x, int y, int beforeValue, int afterValue, int beforeErroneous, int afterErroneous){
    Cell* cell = (Cell*)malloc(sizeof(Cell));
    Move* newMove = (Move*)malloc(sizeof(Move));
    if (newMove==NULL || cell==NULL){
        printMallocFailedAndExit();
    }
    cell->x = x;
    cell->y = y;
    newMove->cell = cell;
    newMove->beforeValue = beforeValue;
    newMove->afterValue = afterValue;
    newMove->beforeErroneous = beforeErroneous;
    newMove->afterErroneous = afterErroneous;
    return newMove;
}

/*checks if the list is empty*/
int isListEmpty(List* list) {
    if(list->head == NULL){
        return 1;
    }
    return 0;
}

/*Inserts a Node at tail of doubly linked list*/
void insertAtTail(List* list, Move** arrMove, int arrSize) {
    struct Node *newNode = getNewNode(arrMove, arrSize); /*create new node*/
    if(isListEmpty(list)) {
        list->head = newNode;
    }
    else {
        list->last->next = newNode;  /*make newNode the new last node*/
        newNode->prev = list->last;  /*mark old last node as prev of newNode*/
    }
    list->last = newNode; /*point last to new last node - make it the last node*/
    list->current = newNode; /*point current to new last node*/
}

/*moves 'current' pointer to the next node*/
void moveToNext(List* list){
    /*assumes there is next node*/
    list->current=list->current->next;
}

/*moves 'current' pointer to the previous node*/
void moveToPrev(List* list){
    /*assumes there is previous node*/
    list->current=list->current->prev;
}

/*checks if the list has another node after 'current'*/
int hasNext(List* list){
    if (list->current==list->last){
        return 0;
    }
    return 1;
}

/*deletes all the nodes from 'current' to the end off the list*/
void deleteFromCurrent(List* list){
    while (list->last != list->current){
        deleteLast(list);
    }
}

/*deletes the node at the last location*/
void deleteLast(List* list) {
    struct Node *tmp = list->last; /*save reference to last link*/
    if(list->head->next == NULL) { /*if only one node*/
        list->head = NULL;
    }
    else {
        list->last->prev->next = NULL;
    }
    list->last = list->last->prev;
    freeNode(tmp); /*frees the allocated memory of the deleted node*/
}

/*gets the current move */
struct Node* getCurrentMove(List* list){
    return list->current;
}


