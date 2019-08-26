#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "main_aux.h"
#include "parser.h"

int main() {
    State current_state;
    Sudoku* sudoku;
    char input[257];
    srand(time(0)); /* TODO: make sure the seed is fine */
    sudoku = (Sudoku*)malloc(sizeof(Sudoku));
    if(sudoku == NULL){
        printMallocFailedAndExit();
    }
    sudoku->mode = INIT; /* TODO: verify the sudoku board creation */
    sudoku->justStarted = 1; /* starting game */
    printEnterCommand();
    while(fgets(input,257, stdin)!=NULL){/*TODO: verify the length is right */
        if (strlen(input)>256){
            printCommandTooLong();
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
