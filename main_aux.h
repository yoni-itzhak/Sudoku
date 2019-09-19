
#ifndef HW4_MAIN_AUX_H
#define HW4_MAIN_AUX_H


#include "structs.h"
#include "parser.h"



/**
 * main_aux summary
 *
 * A container that prints messages to the screen.
 * The container supports the following functions: *
 */

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*auxiliary functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
int stringToInt(char*);
int isNumInArr(int num, int *arr, int arr_size);
float stringToFloat(char* str);
int _isOnlyDigits(char* str);

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*error messages*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void printNotEnoughEmptyCells(int, int);
void handleInputError(Command, Error, Mode, int, int);
void printOpenFileFailed(char* path);
void printCloseFileFailed(char* path);
void printWriteToFileFailed(char* path);
void printLoadedFileLengthNotValid(char* path);
void printErrorInBoardSize();
void printLoadedFileFirstLineNotValid(char* path);
void printLoadedFileCellNotValid(char* path);
void printLoadedFileNotSolvable(char* path);
void printErroneousBoard(Command);
void printErrorInPuzzleGenerator();
void printNoMovesToUndo();
void printNoMovesToRedo();
void printMallocFailedAndExit();
void printGurobiFailed();
void printSaveErronousBoard();

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*game flow messages*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void printEmptyAutofill();
void printNoAutoFilledCells();
void printSameValueCell();
void printChangeInBoard(Cell* cell, int from, int to);
void printNumOfSolution(int numOfSolution);
void printSolutionIsErroneous();
void printExitMessage();
void printEnterCommand();
void printCommandTooLong();
void printLoadedFileFilledAndSolved();
void printSolved();
void printFixed();
void printContainsValue();
void printSolvableBoard();
void printUnsolvableBoard(Command);
void printValidationFailed();
void printHint(int x, int y, int value);
void printSudoku(Sudoku *sudoku);
void printSeparatorRow(int N, int m);
void printPossibleSolAar(WeightedCell** possible_sol_arr, int possible_sol_arr_size, int x, int y);


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
/*auxiliary printing functions*/
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
void _printAllowedCommands(Mode);
void _printModesAllowingCommand(Command);


#endif

