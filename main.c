#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "main_aux.h"
#include "solver.h"
#include "parser.h"

#define ROW 3
#define COLUMN 3
int main() {
    /*
    char* ptr;
    int seed = strtol(argv[1], &ptr, 10);
    int num_of_cells;

    if(argc<=1) {
        return 0;
    }
    srand(seed);
    */
    State current_state;
    Sudoku* sudoku;
    char input[257];
    while (1) {
        current_state = STATE_LOOP;
        sudoku = (Sudoku*)malloc(sizeof(Sudoku));
        if(sudoku == NULL){
            printMallocFailed();
        }
        /*
        createSudoku(sudoku,ROW,COLUMN,num_of_cells);
        generateBoard(sudoku);
        randomDeletingCells(sudoku, num_of_cells);
        printSudoku(sudoku);*/
        sudoku->mode = INIT; /* TODO: verify the sudoku board creation */
        printEnterCommand();
        while(fgets(input,257, stdin)!=NULL){
            if (strlen(input)>256){
                printCommandTooLong();
                continue;
            }
            current_state=readCommand(sudoku, input);
            if(current_state==STATE_SOLVED || current_state ==STATE_EXIT ||current_state==STATE_RESET){
                break;
            }
            printEnterCommand();
        }
        if (feof(stdin) && current_state==STATE_LOOP){
            current_state = STATE_EXIT;
        }
        freeBoard (sudoku->currentState, sudoku->total_size);
        freeBoard (sudoku->solution, sudoku->total_size);
        free(sudoku);
        if(current_state==STATE_SOLVED){ /*In case the board is solved we will read the input from the user until he inserts exit or restart */
            while(current_state == STATE_SOLVED && fgets(input, 1024, stdin)!=NULL){
                current_state = exitRestartCommand(input);
            }
            if(feof(stdin)&&current_state==STATE_SOLVED){
                current_state = STATE_EXIT;
            }
        }
        if(current_state==STATE_EXIT){
            printExiting();
            break;
        }
        else if(current_state==STATE_RESET){
            continue;
        }
    }
    return 0;
}
