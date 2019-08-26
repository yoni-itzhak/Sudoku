
#include "doubly_linked_list.h"
#include "main_aux.h"
#include <stdio.h>
#include <stdlib.h>


void printMove(Move* move){
    if (move==NULL){
        return;
    }
    return;
    /*printf("(x: %d, y: %d, b_value: %d, a_value: %d, b_err: %d, a_err: %d) ", move->cell->x, move->cell->y, move->beforeValue,move->afterValue, move->beforeErroneous, move->afterErroneous);*/
}

void displayForward(List* list) {
    if (list==NULL){
        return;
    }
    return;
/*
    int i, cnt=0;
    struct Node *ptr = list->head;



    while (ptr != NULL) {
        if (cnt == 0){
            printf("DUMMY NODE\n");
        }
        for (i=0; i<ptr->arrSize; i++){
            printMove(ptr->arrMove[i]);
        }
        printf("\n--------------------------------\n");
        ptr = ptr->next;
        cnt++;
    }

*/
}

void addDummyNode(List* list){
    insertAtTail(list, NULL, -1);
}
/*Creates a new List and returns pointer to it.*/
List* getNewList(){
    List* newList = (List*)malloc(sizeof(List)); /*TODO: check what should be the size of the allocated memory*/
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

/*is list empty*/
int isListEmpty(List* list) {
    if(list->head == NULL){
        return 1;
    }
    return 0;
}

/*int length(List* list) {
    int length = 0;
    struct Node* tmp;

    for(tmp = list->head; tmp != NULL; tmp = tmp->next){
        length++;
    }
    return length;
}*/

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

/*assume there is next node*/
void moveToNext(List* list){
    list->current=list->current->next;
}

/*assume there is previous node*/
void moveToPrev(List* list){
    list->current=list->current->prev;
}

int hasNext(List* list){
    if (list->current==list->last){
        return 0;
    }
    return 1;
}

int hasPrev(List* list){
    if (list->current==list->head){
        return 0;
    }
    return 1;
}

/*void moveToStart(List* list){
    while (hasPrev(list)){
        list->current=list->current->prev;
    }
}*/



/*delete the node at the last location*/
void deleteLast(List* list) {
    struct Node *tmp = list->last; /*save reference to last link*/
    if(list->head->next == NULL) { /*if only one node*/
        list->head = NULL;
    }
    else {
        list->last->prev->next = NULL;
    }
    list->last = list->last->prev;
    freeNode(tmp); /*free the allocated memory of the deleted node*/
}



struct Node* getCurrentMove(List* list){
    return list->current;
}


