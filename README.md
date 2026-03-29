# Dots and Boxes – CMPS 241 Sprint 1

This is a two-player console game written in C.

## Build
Run `make` to compile the program.

## Run
Execute `./dots_and_boxes` to start the game.

## How to play
Players enter moves as: row1 col1 row2 col2
Example: `0 0 0 1` draws the top edge of the top-left box.

If you complete a box, it’s yours and you get to play again. 
The player with the most boxes at the end wins.

## Files
- main.c     — game loop
- logic.c    — rules and move validation
- display.c  — board rendering
- input.c    — player input
- game.h     — shared structs and declarations
- Makefile   — build system
