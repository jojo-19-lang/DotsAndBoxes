#include <stdlib.h>
#include <time.h>
#include "bot.h"

static int countEdges(const GameState *gs, int r, int c)
{
    int count = 0;
    if (gs->edges.top   [r][c]) count++;
    if (gs->edges.bottom[r][c]) count++;
    if (gs->edges.left  [r][c]) count++;
    if (gs->edges.right [r][c]) count++;
    return count;
}

static int getAllMoves(const GameState *gs, Move *moves, int maxMoves)
{
    int total = 0;
    int r, c;

    for (r = 0; r <= ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (total >= maxMoves) break;
            int drawn = 0;
            if (r < ROWS && gs->edges.top[r][c])      drawn = 1;
            if (r > 0    && gs->edges.bottom[r-1][c]) drawn = 1;
            if (!drawn) {
                moves[total].r1 = r;
                moves[total].c1 = c;
                moves[total].r2 = r;
                moves[total].c2 = c + 1;
                total++;
            }
        }
    }

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS; c++) {
            if (total >= maxMoves) break;
            int drawn = 0;
            if (c < COLS && gs->edges.left[r][c])      drawn = 1;
            if (c > 0    && gs->edges.right[r][c-1])   drawn = 1;
            if (!drawn) {
                moves[total].r1 = r;
                moves[total].c1 = c;
                moves[total].r2 = r + 1;
                moves[total].c2 = c;
                total++;
            }
        }
    }

    return total;
}

static int moveCompletesBox(const GameState *gs, Move m)
{
    int r1 = m.r1, c1 = m.c1, r2 = m.r2;

    if (r1 == r2) {
        int row = r1;
        int col = c1;
        if (row < ROWS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 3) return 1;
        if (row > 0 && gs->board.owner[row-1][col] == 0 &&
            countEdges(gs, row-1, col) == 3) return 1;
    } else {
        int row = r1;
        int col = c1;
        if (col < COLS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 3) return 1;
        if (col > 0 && gs->board.owner[row][col-1] == 0 &&
            countEdges(gs, row, col-1) == 3) return 1;
    }
    return 0;
}

static int givesOpponentBox(const GameState *gs, Move m)
{
    int r1 = m.r1, c1 = m.c1, r2 = m.r2;

    if (r1 == r2) {
        int row = r1;
        int col = c1;
        if (row < ROWS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 2) return 1;
        if (row > 0 && gs->board.owner[row-1][col] == 0 &&
            countEdges(gs, row-1, col) == 2) return 1;
    } else {
        int row = r1;
        int col = c1;
        if (col < COLS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 2) return 1;
        if (col > 0 && gs->board.owner[row][col-1] == 0 &&
            countEdges(gs, row, col-1) == 2) return 1;
    }
    return 0;
}

BotStrategy botMove(const GameState *gs, int *r1, int *c1, int *r2, int *c2)
{
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }

    Move moves[MAX_MOVES];
    Move safe[MAX_MOVES];
    int total    = getAllMoves(gs, moves, MAX_MOVES);
    int safeCount = 0;
    int i;

    for (i = 0; i < total; i++) {
        if (moveCompletesBox(gs, moves[i])) {
            *r1 = moves[i].r1;
            *c1 = moves[i].c1;
            *r2 = moves[i].r2;
            *c2 = moves[i].c2;
            return BOT_COMPLETES_BOX;
        }
    }

    for (i = 0; i < total; i++) {
        if (!givesOpponentBox(gs, moves[i])) {
            safe[safeCount++] = moves[i];
        }
    }
    if (safeCount > 0) {
        int pick = rand() % safeCount;
        *r1 = safe[pick].r1;
        *c1 = safe[pick].c1;
        *r2 = safe[pick].r2;
        *c2 = safe[pick].c2;
        return BOT_SAFE_MOVE;
    }

    int pick = rand() % total;
    *r1 = moves[pick].r1;
    *c1 = moves[pick].c1;
    *r2 = moves[pick].r2;
    *c2 = moves[pick].c2;
    return BOT_FORCED_MOVE;
}
