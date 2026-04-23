#ifndef GAME_H
#define GAME_H

#define ROWS 4
#define COLS 5

#define MAX_MOVES ((ROWS+1)*COLS + ROWS*(COLS+1))


typedef enum {
    MODE_TWO_PLAYER = 0,
    MODE_BOT        =1,
    MODE_NETWORK    = 2
} GameMode;

typedef struct {
    int top   [ROWS][COLS];
    int bottom[ROWS][COLS];
    int left  [ROWS][COLS];
    int right [ROWS][COLS];
} Edges;

typedef struct {
    char owner[ROWS][COLS];
} Board;

typedef struct {
    Edges edges;
    Board board;
    int   scoreA;
    int   scoreB;
    char  currentPlayer;
    int   totalBoxes;
    int   claimedBoxes;
} GameState;

void printBoard(const GameState *gs);
void printScores(const GameState *gs);
int  getMove(int *r1, int *c1, int *r2, int *c2, char player);
void initGame(GameState *gs);
int  applyMove(GameState *gs, int r1, int c1, int r2, int c2);
int  checkBoxes(GameState *gs, char player);
int  isGameOver(const GameState *gs);
char getWinner(const GameState *gs);

#endif
