#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "solver.h"
#include "game.h"
#include "parser.h"
#include "main_aux.h"
#include "SPBufferset.h"

freeCase(int* cmd, char* path, int* errorsInParams){
    free(cmd);
    free(path);
    free(errorsInParams);
}

int _isModeAllowingCommand(Command command, Mode mode){
    if (!isModeAllowingCommand(command, mode)) {
        handleInputError(command, INVALID_MODE, mode);
        return 0;
    }
    return 1;
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
    int current_cmd, x, y, z, cnt=0, i, numErrors;
    char* path = (char*)malloc(256* sizeof(char));
    SET_STATUS setStatus = UNSOLVED;
    State gameState = STATE_LOOP;
    Mode current_mode = sudoku->mode;
    int* cmd = (int*)malloc(4* sizeof(int)); /* [command, param_x, param_y, param_z] */
    int* errorsInParams = (int*)calloc(3, sizeof(int));
    if(cmd == NULL || path == NULL || errorsInParams == NULL){
        printMallocFailedAndExit();
    }
    _parseCommand(input, cmd, &path, current_mode, &cnt, sudoku, errorsInParams);
    current_cmd = cmd[0];
    x = cmd[1]; y = cmd[2]; z = cmd[3];
    path=NULL;
    switch(current_cmd) {
        case 1: /* solve */
            if(_isEnoughParams(cmd, &cnt, 1, current_mode)){
                if(_isModeAllowingCommand(SOLVE_COMMAND, current_mode)) {
                    solve(sudoku, path);
                }
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 2: /* edit */
            if(_isModeAllowingCommand(EDIT_COMMAND, current_mode)) {
                edit(sudoku, path);
            }
            freeCase(cmd, path,errorsInParams);
            return STATE_LOOP;
        case 3: /* mark_errors */
            if(_isEnoughParams(cmd, &cnt, 1, current_mode)){
                if(errorsInParams[0]){
                    handleInputError(MARK_ERRORS, INVALID_PARAM_X, current_mode);
                }
                else if(_isModeAllowingCommand(MARK_ERRORS, current_mode)){
                    mark_errors(sudoku, x);
                }
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 4: /* print_board */
            if(_isModeAllowingCommand(PRINT_BOARD, current_mode)) {
                print_board(sudoku);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 5: /* set */
            if(_isEnoughParams(cmd, &cnt, 3, current_mode)){
                numErrors=0;
                for(i=0; i<3; ++i){
                    if(errorsInParams[i]){
                        handleInputError(SET, 4+i, current_mode);
                        numErrors++;
                    }
                }
                if(!numErrors){
                    if(_isModeAllowingCommand(SET, current_mode)){
                        setStatus = set(sudoku, --x, --y, z);
                    }
                }
            }
            freeCase(cmd, path, errorsInParams);
            if(setStatus == SOLVED){
                return STATE_SOLVED;
            }
            return STATE_LOOP;
        case 6: /* validate */
            if(isModeAllowingCommand(VALIDATE, current_mode)) {
                validate(sudoku);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 7:/* guess */
            /* TODO: write guess @@@@ */
            if(_isEnoughParams(cmd, &cnt, 1, current_mode)){
                guess(sudoku, x);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 8: /* generate */
            if(_isEnoughParams(cmd, &cnt, 2, current_mode)){
                numErrors=0;
                for(i=0; i<2; ++i){
                    if(errorsInParams[i]){
                        handleInputError(GENERATE, 4+i, current_mode);
                        numErrors++;
                    }
                }
                if(!numErrors){
                    if(_isModeAllowingCommand(GENERATE, current_mode)){
                        gameState = generate(sudoku, x, y);
                    }
                }
            }
            freeCase(cmd, path, errorsInParams);
            return gameState;
        case 9: /* undo */
            if(_isModeAllowingCommand(UNDO, current_mode)) {
                undo(sudoku);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 10: /* redo */
            if(_isModeAllowingCommand(REDO, current_mode)) {
                redo(sudoku);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 11: /* save */
            if(_isEnoughParams(cmd, &cnt, 1, current_mode)){
                save(sudoku, path);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 12: /* hint */
            if(_isEnoughParams(cmd, &cnt, 2, current_mode)){
                numErrors=0;
                for(i=0; i<2; ++i){
                    if(errorsInParams[i]){
                        handleInputError(HINT, 4+i, current_mode);
                        numErrors++;
                    }
                }
                if(!numErrors){
                    if(_isModeAllowingCommand(HINT, current_mode)){
                        hint(sudoku, --x, --y);
                    }
                }
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 13: /* guess_hint */
            if(_isEnoughParams(cmd, &cnt, 2, current_mode)){
                numErrors=0;
                for(i=0; i<2; ++i){
                    if(errorsInParams[i]){
                        handleInputError(HINT, 4+i, current_mode);
                        numErrors++;
                    }
                }
                if(!numErrors){
                    if(_isModeAllowingCommand(HINT, current_mode)){
                        hint(sudoku, --x, --y);
                    }
                }
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 14: /* num_solutions */
            if(_isModeAllowingCommand(NUM_SOLUTIONS, current_mode)) {
                num_solutions(sudoku);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 15: /* autofill */
            if(_isModeAllowingCommand(AUTOFILL, current_mode)) {
                autofill(sudoku);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 16: /* TODO: Figure out what to return in reset */
            if(_isModeAllowingCommand(AUTOFILL, current_mode)) {
                reset(sudoku);
            }
            freeCase(cmd, path, errorsInParams);
            return STATE_LOOP;
        case 17: /* TODO: Figure out what to return in exit */
            exitProgram();
            freeCase(cmd, path, errorsInParams);
            return STATE_EXIT;
        default: /*invalid command*/
            freeCase(cmd, path, errorsInParams);
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
        handleInputError(0, INVALID_NAME, mode);
        cmd[0] = 0;
        return 0;
    }
    *cnt = *cnt+1;
    return 1;
}

int _isTooManyParams(int* cmd, int* cnt, int validNumOfParams, Mode mode){
    if(*cnt > validNumOfParams){
        handleInputError(cmd[0], TOO_MANY_PARAMS, mode);
        cmd[0] = 0;
        return 1;
    }
    return 0;
}

int _isEnoughParams(int *cmd, int *cnt, int validNumOfParams, Mode mode){
    if(*cnt < validNumOfParams){
        handleInputError(cmd[0], NOT_ENOUGH_PARAMS, mode);
        return 0;
    }
    return 1;
}


int _commandWithPath(int* cmd, char* token, int* cnt, char** path, Mode mode){
    if (_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    *path=token;
    *cnt = (*cnt)+1;
    return 0;
}

int _commandMarkErrors(int* cmd, char* token, int *cnt, Mode mode, int* errorsArr){
    if(_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    if(strlen(token)>1 || *token != 48 || *token != 49){
        errorsArr[0]=1;
    }
    else {
        cmd[1] = stringToInt(token);
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
    if (val<=1 || val>= total_size){
        errArr[(*cnt)-1] = 1;
    }
    else{
        cmd[*cnt] = val;
    }
    (*cnt) = (*cnt)+1;
    return 1;
}

int _commandGuess(int* cmd, char* token, int* cnt, Mode mode, int total_size){/* TODO: write gues !! */
    int val = stringToInt(token);
    if (_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    if (val<=1 || val>= total_size){
        handleInputError(GUESS, INVALID_PARAM_X, mode);
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

void _parseCommand(char* input, int* cmd, char** path, Mode mode, int* cnt, Sudoku* sudoku, int* errorsInParams) {
    char s[] = " \t\r\n";
    char* token;
    int total_cells;
    token = strtok(input, s);
    path = NULL;
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
                handleInputError(cmd[0], TOO_MANY_PARAMS, mode);
                return;
            }
        }
        token = strtok(NULL, s);
    }
}
