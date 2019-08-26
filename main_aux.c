#include <stdio.h>
#include <stdlib.h>

#include "main_aux.h"


void printNoMovesToUndo(){
    printf("Error: No moves to undo\n");
}

void printNoMovesToRedo(){
    printf("Error: No moves to redo\n");
}

void printErrorInPuzzleGenerator(){
    printf("Error: An error occurred in the puzzle generator\n");
}

void printErroneousBoard(){
    printf("Error: The board is erroneous\n");
}

void printNotEnoughEmptyCells(int x, int numOfEmptyCells){
    printf("Error: The number of empty cells is smaller than %d, please choose a smaller number than "
           "%d\n", x, numOfEmptyCells);
}

void printCannotGenerateBoardWithErrors(){ /*TODO: change to a dynamic function */
    printf("Error: The function 'generate' cannot receive a board with erroneous cells\n");
}

void _printAllowedCommands(Mode mode){
    int i, is_first=1;
    for(i=1; i<18;++i){
        if (isModeAllowingCommand(i, mode)){
            if(is_first){
                printf("%s", stringFromCommand(i));
                is_first=0;
            }
            else {
                printf(", %s", stringFromCommand(i));
            }
        }
    }
    printf("\n");
}

void handleInputError(Command command, Error err, Mode mode, int total_size, int total_cells){
    if(err == INVALID_NAME){
        printf("Error: The command does not exist, please choose from: ");
        _printAllowedCommands(mode);
    }
    else if(err == INVALID_MODE){
        printf("Error: The command %s is not available in the current mode, "
               "Please choose from: ", stringFromCommand(command));
        _printAllowedCommands(mode);
    }
    else if(err == TOO_MANY_PARAMS){
        printf("Error: Too many parameters were entered, "
               "the command '%s' gets exactly '%s' arguments\n",
                stringFromCommand(command), commandNumParams(command));
    }
    else if(err == NOT_ENOUGH_PARAMS){
        printf("Error: Not enough parameters were entered, "
               "the command '%s' gets exactly '%s' arguments\n",
               stringFromCommand(command), commandNumParams(command));
    }
    else if(err == INVALID_PARAM_X){
        if(command==SOLVE_COMMAND || command==EDIT_COMMAND || command==SAVE){
            printf("Error: The command '%s' got an invalid path\n", stringFromCommand(command));
        }
        else if(command==MARK_ERRORS){
            printf("Error: The command 'mark_errors' can only get 0 or 1 as arguments\n");
        }
        else if(command==SET){/*TODO: figure out the error for HINT and GUESS_HINT */
            printf("Error: The first parameter for the command '%s' should be in the range 0-%d\n",
                    stringFromCommand(command), total_size);
        }
        else if(command==GUESS){
            printf("Error: The command 'guess' gets a parameter in the range 0-%d\n",
                    total_size);
        }
        else if(command==GENERATE){
            printf("Error: The first parameter for the command 'generate' should be in the range 0-%d\n",
                    total_cells);
        }
    }
    else if(err == INVALID_PARAM_Y){
        if(command==SET || command==HINT || command==GUESS_HINT){
            printf("Error: The second parameter for the command '%s' should be in the range 0-%d\n",
                   stringFromCommand(command), total_size);
        }
        else if(command==GENERATE){
            printf("Error: The second parameter for the command 'generate' should be in the range 0-%d\n",
                    total_cells);
        }
    }
    else if(err==INVALID_PARAM_Z){
        printf("Error: The third parameter for the command 'set' should be in the range 0-%d\n",
               total_size);
    }
}

void printEnterCommand(){
    printf("Please enter a command: \n");
}

void printCommandTooLong(){
    printf("The command entered is too long, please enter maximum 256 characters\n");
}

void printNumOfSolution(int numOfSolution){
    printf("Your puzzle has %d solution", numOfSolution);
}

void printGurobiFailed(){
    printf("Error: Gurobi optimization has failed\n");
}

void printChangeInBoard(Cell* cell, int from, int to){
    printf("The value of cell <%d,%d> has changed from %d to %d\n", (cell->x)+1, (cell->y)+1, from, to);
}

void printSolutionIsErroneous(){
    printf("Your solution for the board is erroneous\n");
}
void printExitMessage(){
    printf("Exiting from program...\n");
    exit(0);
}
/*
 * The function prints that the board is solved.
 */
void printSolved(){
    printf("Puzzle solved successfully\n");
}
/*
 * The function prints that the cell is fixed.
 */
void printFixed(){
    printf("Error: cell is fixed\n");
}

/*
 * The function prints the hint for the required cell.
 */
void printHint(int x, int y, int value){
    printf("Hint: set cell <%d,%d> to %d\n",x,y,value);
}
/*
 * The function prints that the board is solvable.
 */
void printSolvableBoard(){
    printf("Validation passed: board is solvable\n");
}
/*
 * The function prints that the board is unsolvable.
 */
void printUnsolvableBoard(){
    printf("Validation failed: board is unsolvable\n");
}
/*
 * The function prints that the program is exiting, and exit.
 */
/*void printExiting(){
    printf("Exiting...\n");
    exit(0);
}*/
/*
 * The function prints the separator row for the board printing format.
 */

void printSeparatorRow(int N, int m){
    int i,dash=4*N+m+1;
    for (i=0;i<dash;i++){
        printf("%c",'-');
    }
    printf("\n");
}
/*
 * The function prints the board printing format.
 */
void printSudoku(Sudoku *sudoku) {
    int i, j, cell;
    int total_size = sudoku->total_size;
    int row = sudoku->row;
    int column = sudoku->column;
    for (i = 0; i < total_size; i++) {
        if (i % row == 0) {
            printSeparatorRow(total_size, column);
        }
        for (j = 0; j < total_size; j++) {
            if (j % column == 0) {
                printf("|");
            }
            printf(" ");
            cell = sudoku->currentState[i][j]->digit;
            if (cell == 0) { /*cell is empty*/
                printf("   ");
            }
            else { /*cell isn't empty*/
                if (sudoku->currentState[i][j]->is_fixed==1){ /*cell is fixed*/
                    printf("%2d.", cell);
                }
                else if (sudoku->currentState[i][j]->cnt_erroneous>0 && (sudoku->markErrors==1 || sudoku->mode==EDIT)){ /*cell is erroneous*/
                    printf("%2d*", cell);
                }
                else{
                    printf("%2d ", cell);
                }
            }
            if (j == total_size-1) {
                printf("|\n");
            }
        }
    }
    printSeparatorRow(total_size, column);
}


/*
 * The function checks for EOF. if so, calling "printExiting".
 */
/*void checkNoInput(){
    if(feof(stdin)){
        printExiting();
    }
}*/

/*
 * The function gets called when memory allocation from malloc was failed and prints appropriate message.
 */
void printMallocFailedAndExit(){
    printf("Error: 'malloc' has failed\n");
    exit(0);
}

/*
 * The function gets called when opening file with 'fopen' was failed and prints appropriate message.
 */
void printOpenFileFailed(char* path){
    printf("Error: 'fopen' has failed to open %s\n", path);
}

/*
 * The function gets called when opening file with 'fclose' was failed and prints appropriate message.
 */
void printCloseFileFailed(char* path){
    printf("Error: 'fclose' has failed to close %s\n", path);
}

/*
 * The function gets called when write to file with 'fprintf' was failed and prints appropriate message.
 */
void printWriteToFileFailed(char* path){
    printf("Error: 'fprintf' has failed to write to %s\n", path);
}

void printLoadedFileLengthNotValid(char* path){
    printf("Error: file %s is not in valid length\n", path);
}
void printLoadedFileEOF(char* path){
    printf("Error: file %s has reached EOF\n", path);
}
void printLoadedFileFirstLineNotValid(char* path){
    printf("Error: the first line (row & column) in file %s is not valid\n", path);
}
void printLoadedFileCellNotValid(char* path){
    printf("Error: one of the cells in file %s is not valid\n", path);
}
void printLoadedFileNotSolvable(char* path){
    printf("Error: The board in file %s is not solvable. When loading a file in edit mode the board loaded must be solvable.\n", path);
}



/*
 * The function prints that the command is invalid.
 */
/*void printInvalidCommand(){
    printf("Error: invalid command\n");
}*/

void printSameValueCell(){
    printf("Notice: your chosen cell has the same value that you chose to set\n");
}


int stringToInt(char* str){
    char* ptr;
    int val = strtol(str, &ptr, 10);
    return val;
}

