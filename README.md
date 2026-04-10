# Dots and Boxes – CMPS 241 Project (Sprint 1 & 2)

This is a console-based implementation of the classic Dots and Boxes game written in C.

## Build
Run `make` to compile the program.

## Run
Execute `./dots_and_boxes` to start the game.

## Game Modes
Two-player mode (Sprint 1): Player A vs Player B
Bot mode (Sprint 2): Player vs Hard-level Bot

## How to play
Players enter moves as: row1 col1 row2 col2
Example: `0 0 0 1` draws the top edge of the top-left box.

If you complete a box, it’s yours and you get to play again. 
The player with the most boxes at the end wins.

## Bot Strategy (Sprint 2)
The bot uses a simple rule-based greedy strategy. It checks all possible moves and first tries to find a move that completes a box. If no such move exists, it avoids moves that would allow the opponent to complete a box on their next turn and randomly chooses from the remaining safe moves. If no safe moves are available, it selects any valid move at random. 

## Files
main.c — game loop and mode selection
logic.c — rules and move validation
display.c — board rendering
input.c — player input
bot.c — bot logic and decision making
game.h — shared structures and declarations
bot.h — bot-related declarations
Makefile — build system
