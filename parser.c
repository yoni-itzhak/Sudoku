#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "solver.h"
#include "game.h"
#include "parser.h"
#include "main_aux.h"
#include "SPBufferset.h"

void _freeCase(int *cmd, char *path, int *errorsInParams){
    free(cmd);
    free(path);
    free(errorsInParams);
}

char *stringFromCommand(Command c){
    char *str_commands[] = {"invalid command", "solve", "edit", "mark_errors","print_board",
                            "set", "validate", "guess", "generate", "undo", "redo", "save", "hint",
                            "guess_hint", "num_solutions", "autofill", "reset", "exit"};

    return str_commands[c];
}

char* commandNumParams(Command com){
    if (com==1 || com==3 || com==7 || com==11){
        return "1";
    }
    else if (com==8 || com ==12 || com ==13){
        return "2";
    }
    else if(com==5){
        return "3";
    }
    else if(com==2){
        return "0 or 1";
    }
    else{
        return "0";
    }
}

int isModeAllowingCommand(Command command, Mode mode){
    if (mode == SOLVE){
        if (command == GENERATE) {
            return 0;
        }
        return 1;
    }
    else if(mode == EDIT){
        if(command == MARK_ERRORS || command == GUESS || command == HINT ||
           command == GUESS_HINT || command == AUTOFILL){
            return 0;
        }
        return 1;
    }
    else{ /*(mode == INIT)*/
        if(command == SOLVE_COMMAND || command == EDIT_COMMAND){
            return 1;
        }
        return 0;
    }
}

/*
 * @params - function receives pointer to the main Sudoku and the string input from the user.
 *
 * The function calls to "_parseCommand" function and according to its result, "readCommand" calls and executes the appropriate command.
 *
 * @return - The function returns enum that indicates if after the execution of the command :
 * 1. the board is solved (STATE_SOLVED)
 * 2. the board isn't solved yet (STATE_LOOP)
 * 3. the user chose the "restart" command (STATE_RESET)
 * 4. the user chose the "exit" command (STATE_EXIT)
 */


State readCommand(Sudoku* sudoku, char* input){
    /*TODO: move the mode verification to the last switch*/
    /*TODO: need to send solved/unsolved: set, guess, generate, autofill*/
    int current_cmd, x, y, z, cnt=0, i, numErrors, total_cells;
    char* path = (char*)malloc(256* sizeof(char));
    Mode current_mode = sudoku->mode;
    int* cmd = (int*)malloc(4* sizeof(int)); /* [command, param_x, param_y, param_z] */
    int* errorsInParams = (int*)calloc(3, sizeof(int));
    if(cmd == NULL || path == NULL || errorsInParams == NULL){
        printMallocFailedAndExit();
    }
    total_cells = (sudoku->total_size)*(sudoku->total_size);
    _parseCommand(input, cmd, path, current_mode, &cnt, sudoku, errorsInParams);
    current_cmd = cmd[0];
    x = cmd[1]; y = cmd[2]; z = cmd[3];
    switch(current_cmd) {
        case 1: /* solve */
            if(_isEnoughParams(cmd, &cnt, 1, current_mode)) {
                solve(sudoku, path);
            }
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 2: /* edit */
            if (cnt == 1){
                path=NULL;
            }
            edit(sudoku, path);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 3: /* mark_errors */
            if(_isEnoughParams(cmd, &cnt, 1, current_mode)){
                if(errorsInParams[0]){
                    handleInputError(MARK_ERRORS, INVALID_PARAM_X, current_mode, sudoku->total_size, total_cells);
                }
                else{
                    mark_errors(sudoku, x);
                }
            }
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 4: /* print_board */
            print_board(sudoku);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 5: /* set */
            if(_isEnoughParams(cmd, &cnt, 3, current_mode)){
                numErrors=0;
                for(i=0; i<3; ++i){
                    if(errorsInParams[i]){
                        handleInputError(SET, 4+i, current_mode, sudoku->total_size, total_cells);
                        numErrors++;
                    }
                }
                if(!numErrors) {
                    set(sudoku, --x, --y, z);
                }
            }
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 6: /* validate */
            validate(sudoku);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 7:/* guess */
            /* TODO: write guess @@@@ */
            if(_isEnoughParams(cmd, &cnt, 1, current_mode)){
                guess(sudoku, x);
            }
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 8: /* generate */
            if(_isEnoughParams(cmd, &cnt, 2, current_mode)){
                numErrors=0;
                for(i=0; i<2; ++i){
                    if(errorsInParams[i]){
                        handleInputError(GENERATE, 4+i, current_mode, sudoku->total_size, total_cells);
                        numErrors++;
                    }
                }
                if(!numErrors){
                    generate(sudoku, x, y);
                }
            }
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 9: /* undo */
            undo(sudoku);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 10: /* redo */
            redo(sudoku);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 11: /* save */
            save(sudoku, path);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 12: /* hint */
            if(_isEnoughParams(cmd, &cnt, 2, current_mode)){
                numErrors=0;
                for(i=0; i<2; ++i){
                    if(errorsInParams[i]){
                        handleInputError(HINT, 4+i, current_mode, sudoku->total_size, total_cells);
                        numErrors++;
                    }
                }
                if(!numErrors){
                    hint(sudoku, --x, --y);
                }
            }
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 13: /* guess_hint */
            if(_isEnoughParams(cmd, &cnt, 2, current_mode)){
                numErrors=0;
                for(i=0; i<2; ++i){
                    if(errorsInParams[i]){
                        handleInputError(GUESS_HINT, 4+i, current_mode, sudoku->total_size, total_cells);
                        numErrors++;
                    }
                }
                if(!numErrors){
                    guess_hint(sudoku, --x, --y);
                }
            }
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 14: /* num_solutions */
            num_solutions(sudoku);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 15: /* autofill */
            autofill(sudoku);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 16: /* TODO: Figure out what to return in reset */
            reset(sudoku);
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 17: /* TODO: Figure out what to return in exit */
            exitProgram();
            _freeCase(cmd, path, errorsInParams);
            return STATE_EXIT;
        default: /*invalid command*/
            _freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
    }
}


int _commandName(int* cmd, char* token, int* cnt, Mode mode){
    if (strcmp(token, "solve") == 0) {
        cmd[0] = 1;
    } else if (strcmp(token, "edit") == 0) {
        cmd[0] = 2;
    } else if (strcmp(token, "mark_errors") == 0) {
        cmd[0] = 3;
    } else if (strcmp(token, "print_board") == 0) {
        cmd[0] = 4;
    } else if (strcmp(token, "set") == 0) {
        cmd[0] = 5;
    } else if (strcmp(token, "validate") == 0) {
        cmd[0] = 6;
    }else if (strcmp(token, "guess") == 0) {
        cmd[0] = 7;
    }else if (strcmp(token, "generate") == 0) {
        cmd[0] = 8;
    }else if (strcmp(token, "undo") == 0) {
        cmd[0] = 9;
    }else if (strcmp(token, "redo") == 0) {
        cmd[0] = 10;
    }else if (strcmp(token, "save") == 0) {
        cmd[0] = 11;
    }else if (strcmp(token, "hint") == 0) {
        cmd[0] = 12;
    }else if (strcmp(token, "guess_hint") == 0) {
        cmd[0] = 13;
    }else if (strcmp(token, "num_solutions") == 0) {
        cmd[0] = 14;
    }else if (strcmp(token, "autofill") == 0) {
        cmd[0] = 15;
    }else if (strcmp(token, "reset") == 0) {
        cmd[0] = 16;
    }else if (strcmp(token, "exit") == 0) {
        cmd[0] = 17;
    }
    else{/* name is incorrect */
        handleInputError(0, INVALID_NAME, mode, -1, -1);
        cmd[0] = 0;
        return 0;
    }
    *cnt = *cnt+1;
    return 1;
}

int _isTooManyParams(int* cmd, int* cnt, int validNumOfParams, Mode mode){
    if(*cnt > validNumOfParams){
        handleInputError(cmd[0], TOO_MANY_PARAMS, mode, -1, -1);
        cmd[0] = 0;
        return 1;
    }
    return 0;
}

int _isEnoughParams(int *cmd, int *cnt, int validNumOfParams, Mode mode){
    if((*cnt)-1 < validNumOfParams){
        handleInputError(cmd[0], NOT_ENOUGH_PARAMS, mode, -1, -1);
        return 0;
    }
    return 1;
}


int _commandWithPath(int* cmd, char* token, int* cnt, char* path, Mode mode){
    if (_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    strcpy(path, token);
    *cnt = (*cnt)+1;
    return 0;
}

int _commandMarkErrors(int* cmd, char* token, int *cnt, Mode mode, int* errorsArr){
    if(_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    if((strlen(token)==1) & (*token == 48 || *token == 49)){
        cmd[1] = stringToInt(token);
    }
    else {
        errorsArr[0]=1;
    }
    *cnt = (*cnt)+1;
    return 1;
}

int _commandSet(int* cmd, char* token, int* cnt, Mode mode, int total_size, int* errArr){
    int val;
    if (_isTooManyParams(cmd, cnt, 3, mode)){
        return 0;
    }
    val = stringToInt(token);
    if (val<0 || val> total_size){
        errArr[(*cnt)-1] = 1;
    }
    else{
        cmd[*cnt] = val;
    }
    (*cnt) = (*cnt)+1;
    return 1;
}

int _commandGuess(int* cmd, char* token, int* cnt, Mode mode, int total_size){/* TODO: write guess !! */
    int val = stringToInt(token);
    if (_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    if (val<=1 || val>= total_size){
        handleInputError(GUESS, INVALID_PARAM_X, mode, total_size, -1);
        cmd[0] = 0;
        return 0;
    }
    *cnt = *cnt+1;
    return 1;
}

int _commandGenerate(int* cmd, char* token, int* cnt, Mode mode, int total_cells, int* errArr){
    int val, isValidParam=1;
    unsigned long i;
    if (_isTooManyParams(cmd, cnt, 2, mode)){
        return 0;
    }
    val = stringToInt(token);
    if(val==0){
        for(i=0; i<strlen(token); i++){
            if(token[i] != 48){
                errArr[(*cnt)-1]=1;
                isValidParam=0;
                break;
            }
        }
    }
    else if (val<0 || val> total_cells){
        errArr[(*cnt)-1]=1;
        isValidParam=0;
    }
    if(isValidParam){
        cmd[*cnt]=val;
    }
    *cnt = *cnt+1;
    return 1;
}

int _commandHint(int* cmd, char* token, int* cnt, Mode mode, int total_size, int* errArr){/* hint and guess_hint */
    int val = stringToInt(token);
    if (_isTooManyParams(cmd, cnt, 2, mode)){
        return 0;
    }
    if (val<=1 || val>= total_size){
        errArr[(*cnt)-1]=1;
    }
    else{
        cmd[*cnt]=val;
    }
    (*cnt) = (*cnt)+1;
    return 1;
}

void _parseCommand(char* input, int* cmd, char* path, Mode mode, int* cnt, Sudoku* sudoku, int* errorsInParams) {
    char s[] = " \t\r\n";
    char* token;
    int total_cells;
    token = strtok(input, s);
    if (token==NULL){
        cmd[0]=18;
        return;
    }
    total_cells = (sudoku->total_size)*(sudoku->total_size);
    while(token != NULL) {
        if(*cnt==0){
            if (!_commandName(cmd, token, cnt, mode)){/* invalid command */
                return;
            }
            if(!isModeAllowingCommand(cmd[0], mode)){
                handleInputError(cmd[0], INVALID_MODE, mode, -1, -1);
                cmd[0]=0;
                return;
            }
        }
        else {
            if (cmd[0] == 1 || cmd[0] == 2 || cmd[0] == 11) {/* solve or edit or save */
                if(!_commandWithPath(cmd, token, cnt, path, mode)){
                    return;
                }
            }
            else if (cmd[0] == 3){/* mark_errors */
                if(!_commandMarkErrors(cmd, token, cnt, mode, errorsInParams)){
                    return;
                }
            }
            else if (cmd[0] == 5){/* set */
                if(!_commandSet(cmd, token, cnt, mode, sudoku->total_size, errorsInParams)){
                    return;
                }
            }
            else if (cmd[0] == 7){/* guess */
                if(!_commandGuess(cmd, token, cnt, mode, sudoku->total_size)){
                    return;
                }
            }
            else if (cmd[0] == 8){/* generate */

                if(!_commandGenerate(cmd, token, cnt, mode, total_cells, errorsInParams)){
                    return;
                }
            }
            else if(cmd[0] == 12 || cmd[0] == 13){/* hint or guss_hint */
                if(!_commandHint(cmd, token, cnt, mode, sudoku->total_size, errorsInParams)){
                    return;
                }
            }
            else{/* all other commands */
                cmd[0] = 0;
                handleInputError(cmd[0], TOO_MANY_PARAMS, mode, -1, -1);
                return;
            }
        }
        token = strtok(NULL, s);
    }
}
