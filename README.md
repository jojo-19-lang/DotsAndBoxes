# Dots and Boxes – CMPS 241 Project (Sprint 1 & 2 & 3)

This is a console-based implementation of the classic Dots and Boxes game written in C.
It supports:
- Two-player mode (local gameplay)
- Player vs Bot mode (AI opponent)
- Networked gameplay (client/server – Sprint 3)
## Build
Run `make` to compile the program.

## Run
Execute `./dots_and_boxes` to start the game.

## Game Modes
1.Two-player mode (Sprint 1): Player A vs Player B
2.Bot mode (Sprint 2): Player vs Hard-level Bot
3.Network Mode (Sprint 3):
a.Client/server multiplayer over TCP sockets
b.Allows two players on separate machines
## How to play
Players enter moves as: row1 col1 row2 col2
Example: `0 0 0 1` draws the top edge of the top-left box.

If you complete a box, it’s yours and you get to play again. 
The player with the most boxes at the end wins.

## Bot Strategy (Sprint 2)
This bot implements a strategically informed AI for Dots and Boxes using a depth-limited Minimax algorithm with Alpha-Beta pruning, enhanced by domain-specific heuristics to capture the underlying structure of the game. It prioritizes moves through intelligent ordering,favoring immediate box completions, then safe moves that avoid conceding opportunities, and finally risky moves,thereby improving pruning efficiency and effective search depth. The evaluation function combines score differential with a structural analysis of the board, notably incorporating chain detection via DFS to identify and penalize large connected regions of vulnerable boxes, reflecting the strategic importance of controlling chains rather than opening them prematurely. Additionally, the bot explicitly models extra turns resulting from box completions, enabling it to reason about multi-step capture sequences and maintain sustained tactical pressure. Overall, the approach balances short-term gains with long-term positional safety, producing a competitive and strategically coherent agent.
##Sprint 3 – Networking
The game was extended with:

TCP client/server communication
Separate client.c and server.c modules
Turn synchronization over network
Mutex-protected shared logic for thread safety

This allows two players to play remotely in real time.
## Files
main.c — game loop and mode selection
logic.c — rules and move validation
display.c — board rendering
input.c — player input
bot.c — bot logic and decision making
server.c — server-side networking (Sprint 3)
client.c — client-side networking (Sprint 3)
game.h — shared structures and declarations
bot.h — bot-related declarations
Makefile — build system
