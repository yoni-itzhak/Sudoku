#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "structs.h"
#include "main_aux.h"
#include "parser.h"

int main() {
    State current_state;
    Sudoku* sudoku;
    char input[257];
    int c;
    srand(time(0)); /* TODO: make sure the seed is fine */
    sudoku = (Sudoku*)malloc(sizeof(Sudoku));
    if(sudoku == NULL){
        printMallocFailedAndExit();
    }
    sudoku->mode = INIT; /* TODO: verify the sudoku board creation */
    sudoku->justStarted = 1; /* starting game */
    sudoku->markErrors = 1;
    printf("Welcome to Ron&Yoni's Sudoku!\n");
    printEnterCommand();
    while(fgets(input, 257, stdin)!=NULL){
        if (strchr(input, '\n') == NULL){
            printCommandTooLong();
            for (c; (c = getchar()) != EOF && c != '\n';); /* cleaning the buffer */
            printEnterCommand();
            continue;
        }
        current_state=readCommand(sudoku, input);
        if(current_state == STATE_EXIT){
            break;
        }
        printEnterCommand();
    }
    freeSudokuMemory(sudoku);
    return 0;
}
