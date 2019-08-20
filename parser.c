#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "solver.h"
#include "game.h"
#include "parser.h"
#include "main_aux.h"
#include "SPBufferset.h"

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
State readCommand(Sudoku *sudoku, char *input){
    int current_cmd, x, y, z, cnt=0, *ptr_cnt;
    char* path[256];
    Mode current_mode = sudoku->mode;
    int* cmd = (int*)malloc(4* sizeof(int)); /* [command, param_x, param_y, param_z] */
    if(cmd == NULL){
        printMallocFailedAndExit();
    }
    ptr_cnt = &cnt;
    _parseCommand(input, cmd, path, current_mode, ptr_cnt);
    current_cmd = cmd[0];
    x = cmd[1]; y = cmd[2]; z = cmd[3];

    switch(current_cmd) { /* TODO: write handle error for too few params */
        case 0: /* invalid command */
            free(cmd);
            return STATE_LOOP;
        case 1: /* TODO: write solve */
            if(_isEnoughParams(cmd, ptr_cnt, 1, current_mode)){
                solve();
                free(cmd);
            }
            free(cmd);
            return STATE_LOOP;
        case 2: /* TODO:write edit */
            edit();
            free(cmd);
        case 3: /* mark_errors */
            if(_isEnoughParams(cmd, ptr_cnt, 1, current_mode)){
                mark_errors(sudoku, x);
            }
            free(cmd);
            return STATE_LOOP;
        case 4: /* print_board */
            print_board(sudoku);
            free(cmd);
            return STATE_LOOP;
        case 5: /* set */
            if(_isEnoughParams(cmd, ptr_cnt, 3, current_mode)){
                set(sudoku, --x, --y, z);
                free(cmd);
                /* TODO: Figure out what to return */
            }
            free(cmd);
            return STATE_LOOP;
        case 6: /* validate */
            validate(sudoku);
            free(cmd);
            return STATE_LOOP;
        case 7:/* guess */
            if(_isEnoughParams(cmd, ptr_cnt, 1, current_mode)){
                guess(sudoku, x);
            }
            free(cmd);
            return STATE_LOOP;
        case 8: /* generate */
            if(_isEnoughParams(cmd, ptr_cnt, 2, current_mode)){
                generate(sudoku, x, y);
            }
            free(cmd);
            return STATE_LOOP;
        case 9: /* undo */
            undo(sudoku);
            free(cmd);
            return STATE_LOOP;
        case 10: /* redo */
            redo(sudoku);
            free(cmd);
            return STATE_LOOP;
        case 11: /* TODO:write edit */
            if(_isEnoughParams(cmd, ptr_cnt, 1, current_mode)){
                save();
            }
            free(cmd);
            return STATE_LOOP;
        case 12: /* hint */
            if(_isEnoughParams(cmd, ptr_cnt, 2, current_mode)){
                hint(sudoku, --x, --y);
            }
            free(cmd);
            return STATE_LOOP;
        case 13: /* guess_hint */
            if(_isEnoughParams(cmd, ptr_cnt, 2, current_mode)){
                guess_hint(sudoku, --x, --y);
            }
            free(cmd);
            return STATE_LOOP;
        case 14: /* num_solutions */
            num_solutions(sudoku);
            free(cmd);
            return STATE_LOOP;
        case 15: /* autofill */
            autofill(sudoku);
            free(cmd);
            return STATE_LOOP;
        case 16: /* TODO: Figure out what to return in reset */
            reset(sudoku);
            free(cmd);
            return STATE_LOOP;
        case 17: /* TODO: Figure out what to return in exit */
            exitProgram(sudoku);
            free(cmd);
            return STATE_LOOP;
        default:
            free(cmd);
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
    /*TODO: Take care of the file path */
    *cnt = *cnt+1;
    return 0;
}

int _commandMarkErrors(int* cmd, char* token, int *cnt, Mode mode){
    if(_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    if(strlen(token)>1 || *token != 48 || *token != 49){
        handleInputError(MARK_ERRORS, INVALID_PARAM_X, mode);
        return 0;
    }
    cmd[1] = stringToInt(token);
    *cnt = *cnt+1;
    return 1;
}

int _commandSet(int* cmd, char* token, int* cnt, Mode mode){
    Error err;
    int val = stringToInt(token);
    if (_isTooManyParams(cmd, cnt, 3, mode)){
        return 0;
    }
    if (val<=1 || val>= MAX_CELL_RANGE){
        err = *cnt+3;
        handleInputError(SET, err, mode);
        cmd[0] = 0;
        return 0;
    }
    *cnt = *cnt+1;
    return 1;
}

int _commandGuess(int* cmd, char* token, int* cnt, Mode mode){
    int val = stringToInt(token);
    if (_isTooManyParams(cmd, cnt, 1, mode)){
        return 0;
    }
    if (val<=1 || val>= MAX_CELL_RANGE){
        handleInputError(GUESS, INVALID_PARAM_X, mode);
        cmd[0] = 0;
        return 0;
    }
    *cnt = *cnt+1;
    return 1;
}

int _commandGenerate(int* cmd, char* token, int* cnt, Mode mode){
    Error err;
    int val = stringToInt(token);
    if (_isTooManyParams(cmd, cnt, 2, mode)){
        return 0;
    }
    if (val<0 || val> BOARD_SIZE){/*TODO: make sure y is in range */
        err = *cnt+3;
        handleInputError(GENERATE, err, mode);
        cmd[0] = 0;
        return 0;
    }
    *cnt = *cnt+1;
    return 1;
}

int _commandHint(int* cmd, char* token, int* cnt, Mode mode){/* hint and guess_hint */
    Error err;
    int val = stringToInt(token);
    if (_isTooManyParams(cmd, cnt, 2, mode)){
        return 0;
    }
    if (val<=1 || val>= MAX_CELL_RANGE){
        err = *cnt+3;
        handleInputError(cmd[0], err, mode);
        cmd[0] = 0;
        return 0;
    }
    *cnt = *cnt+1;
    return 1;
}

void _parseCommand(char* input, int* cmd, char** path, Mode mode, int* cnt) {
    char s[] = " \t\r\n";
    char* token;
    token = strtok(input, s);
    *path = NULL;
    if (token==NULL){
        cmd[0]=18;
        return;
    }
    while(token != NULL) {
        if(*cnt==0){
            if (!_commandName(cmd, token, cnt, mode)){/* invalid command */
                return;
            }
            if(!isModeAllowingCommand(cmd[0], mode)){
                handleInputError(cnt[0], INVALID_MODE, mode);
                cmd[0]=0;
            }
        }
        else {
            if (cmd[0] == 1 || cmd[0] == 2 || cmd[0] == 11) {/* solve or edit or save */
                if(!_commandWithPath(cmd, token, cnt, path, mode)){
                    return;
                }
            }
            else if (cmd[0] == 3){/* mark_errors */
                if(!_commandMarkErrors(cmd, token, cnt, mode)){
                    return;
                }
            }
            else if (cmd[0] == 5){/* set */
                if(!_commandSet(cmd, token, cnt, mode)){
                    return;
                }
            }
            else if (cmd[0] == 7){/* guess */
                if(!_commandGuess(cmd, token, cnt, mode)){
                    return;
                }
            }
            else if (cmd[0] == 8){/* generate */
                if(!_commandGenerate(cmd, token, cnt, mode)){
                    return;
                }
            }
            else if(cmd[0] == 12 || cmd[0] == 13){/* hint or guss_hint */
                if(!_commandHint(cmd, token, cnt, mode)){
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


