#ifndef HW4_PARSER_H
#define HW4_PARSER_H


#include "structs.h"

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

char *stringFromCommand(Command c);
char* stringFromMode(Mode mode);
char* commandNumParams(Command com);
int isModeAllowingCommand(Command command, Mode mode);

int _commandName(int*, char*, int*, Mode);
int _isTooManyParams(int*, int*, int, Mode);
int _isEnoughParams(int *cmd, int *cnt, int validNumOfParams, Mode mode);
int _commandWithPath(int*, char*, int*, char*, Mode);
int _commandMarkErrors(int*, char*, int*, Mode, int*);
int _commandSet(int*, char*, int*, Mode, int, int*);
int _commandGuess(int*, char*, int*, float*, Mode, int*);
int _commandGenerate(int*, char*, int*, Mode, int, int*);
int _commandHint(int*, char*, int*, Mode, int, int*);
void _freeCase(int *, char* , int *);
void _parseCommand(char*, int*, float*, char*, Mode, int*, Sudoku*, int*);

State readCommand(Sudoku*, char*);

#endif
