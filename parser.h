

#ifndef HW4_PARSER_H
#define HW4_PARSER_H


#include "solver.h"

/**
 * parser summary
 *
 * A container that takes care of the input from the user.
 * The container supports the following functions:
 *
 * State -              Enum that specifies the game current state: STATE_SOLVED - if the board is solved,
 *                      STATE_RESTART - if the user inserts "restart", STATE_EXIT - if the user inserts exit,
 * STATE_LOOP -         any other case
 * readCommand -        Calls and executes the appropriate command using _whichCommand
 * _whichCommand -      The function interprets the user's input into the required command and values
 *                      (or indicates if the command is invalid)
 * numberOfCells -      reads from the user the number of fixed cells that he wants on the board
 * exitRestartCommand - Called after the board is solved, allows the user to type only "exit" or "restart"
 *
 */

typedef enum{
    STATE_SOLVED,
    STATE_RESET,
    STATE_EXIT,
    STATE_LOOP
}State;

typedef enum{
    INVALID_COMMAND,
    SOLVE_COMMAND,
    EDIT_COMMAND,
    MARK_ERRORS,
    PRINT_BOARD,
    SET,
    VALIDATE,
    GUESS,
    GENERATE,
    UNDO,
    REDO,
    SAVE,
    HINT,
    GUESS_HINT,
    NUM_SOLUTIONS,
    AUTOFILL,
    RESET,
    EXIT
}Command;

typedef enum{
    INVALID_NAME,
    INVALID_MODE,
    TOO_MANY_PARAMS,
    NOT_ENOUGH_PARAMS,
    INVALID_PARAM_X,
    INVALID_PARAM_Y,
    INVALID_PARAM_Z
}Error;

char *stringFromCommand(Command c){
    char *str_commands[] = {"invalid command", "solve", "edit", "mark_errors","print_board",
                            "set", "validate", "generate", "undo", "redo", "save", "hint",
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
    if(mode == EDIT){
        if(command == MARK_ERRORS || command == GUESS || command == HINT ||
           command == GUESS_HINT || command == AUTOFILL){
            return 0;
        }
        return 1;
    }
    if (mode == INIT){
        if(command == SOLVE_COMMAND || command == EDIT_COMMAND){
            return 1;
        }
        return 0;
    }
}

int _commandName(int*, char*, int*, Mode);
int _isTooManyParams(int*, int*, int, Mode);
int _isEnoughParams(int *cmd, int *cnt, int validNumOfParams, Mode mode);
int _commandWithPath(int*, char*, int*, char**, Mode);
int _commandMarkErrors(int*, char*, int*, Mode);
int _commandSet(int*, char*, int*, Mode, int);
int _commandGuess(int*, char*, int*, Mode, int);
int _commandGenerate(int*, char*, int*, Mode, int);
int _commandHint(int*, char*, int*, Mode, int);
int _modeIsAllowingCommand(Command, Mode mode);
State readCommand(Sudoku*, char*);
void _parseCommand(char*, int*, char**, Mode, int*, Sudoku*);
State exitRestartCommand(char*);

#endif
