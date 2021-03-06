#ifndef HW4_PARSER_H
#define HW4_PARSER_H


#include "structs.h"

/**
 * parser summary
 *
 * A container that takes care of the user input.
 * flow:
 * 1. The user input reaches the function readCommand.
 * 2. The input is moved to _parseCommand where the input is separated to its arguments.
 * 3. The arguments are validated for the relevant command.
 * 4. If the number of arguments is right and there no errors, they are passed to the relevant function in game.c
 *
 **/

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


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*auxiliary functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
char* stringFromCommand(Command);
char* stringFromMode(Mode);
char* commandNumParams(Command);
int isModeAllowingCommand(Command, Mode);
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
State _finish_and_return_loop(int*, char*, int*);
void _parseCommand(char*, int*, float*, char*, Mode, int*, Sudoku*, int*);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*read the user command and parse it for game*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
State readCommand(Sudoku*, char*);

#endif
