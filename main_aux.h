
#ifndef HW4_MAIN_AUX_H
#define HW4_MAIN_AUX_H

#include "solver.h"
#include "parser.h"

#include "structs.h"




/**
 * main_aux summary
 *
 * A container that prints messages to the screen.
 * The container supports the following functions:
 *
 * printSolved          - prints that the puzzle is solved successfully
 * printFixed           - prints error when the cell is fixed
 * printInvalidValue    - prints error when the value given is invalid
 * printSolvable        - prints the board is solvable if the validation is true
 * printUnsolvable      - prints the board is unsolvable if the validation is false
 * printHint            - prints a hint to the user
 * printBoard           - prints the current board
 * printSeparatorRow    - prints the separator row for the board printing format
 * printExiting         - prints that the program is exiting, and exit
 * printInvalidCommand  - prints that the command is invalid
 * numberOfCells        - reads from the user the number of fixed cells
 * checkNoInput         - checks for EOF. if so, calling "printExiting"
 * printMallocFailed    - gets called when memory allocation from malloc was failed and prints appropriate message
 *
 */

void printSameValueCell();
void printCannotGenerateBoardWithErrors();
void printNotEnoughEmptyCells(int, int);
void printChangeInBoard(Cell* cell, int from, int to);
void printNumOfSolution(int numOfSolution);
void printSolutionIsErroneous();
void printExitMessage();
void printEnterCommand();
void printCommandTooLong();
void handleInputError(Command, Error, Mode, int, int);
int stringToInt(char*);
void printOpenFileFailed(char* path);
void printCloseFileFailed(char* path);
void printWriteToFileFailed(char* path);

void printLoadedFileLengthNotValid(char* path);
void printLoadedFileEOF(char* path);
void printLoadedFileFirstLineNotValid(char* path);
void printLoadedFileCellNotValid(char* path);
void printLoadedFileNotSolvable(char* path);
void printErroneousBoard();
void printErrorInPuzzleGenerator();

void printNoMovesToUndo();
void printNoMovesToRedo();

void printSolved();
void printFixed();
/*void printInvalidValue();*/
void printSolvableBoard();
void printUnsolvableBoard();
void printHint(int x, int y, int value);
void printSudoku(Sudoku *sudoku);
void printSeparatorRow(int N, int m);
/*void printExiting();*/
/*void printInvalidCommand();*/
/*void checkNoInput();*/
void printMallocFailedAndExit();
void printGurobiFailed();

#endif

