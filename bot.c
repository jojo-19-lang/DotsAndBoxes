#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "bot.h"

#define MAX_DEPTH       4
#define CHAIN_PENALTY   3
#define MAX_EXTRA_TURNS 10
#define SAFE_BONUS      2

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
                moves[total].r1 = r; moves[total].c1 = c;
                moves[total].r2 = r; moves[total].c2 = c + 1;
                total++;
            }
        }
    }

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS; c++) {
            if (total >= maxMoves) break;
            int drawn = 0;
            if (c < COLS && gs->edges.left[r][c])    drawn = 1;
            if (c > 0    && gs->edges.right[r][c-1]) drawn = 1;
            if (!drawn) {
                moves[total].r1 = r; moves[total].c1 = c;
                moves[total].r2 = r + 1; moves[total].c2 = c;
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
        int row = r1, col = c1;
        if (row < ROWS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 3) return 1;
        if (row > 0    && gs->board.owner[row-1][col] == 0 &&
            countEdges(gs, row-1, col) == 3) return 1;
    } else {
        int row = r1, col = c1;
        if (col < COLS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 3) return 1;
        if (col > 0    && gs->board.owner[row][col-1] == 0 &&
            countEdges(gs, row, col-1) == 3) return 1;
    }
    return 0;
}

static int givesOpponentBox(const GameState *gs, Move m)
{
    int r1 = m.r1, c1 = m.c1, r2 = m.r2;

    if (r1 == r2) {
        int row = r1, col = c1;
        if (row < ROWS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 2) return 1;
        if (row > 0    && gs->board.owner[row-1][col] == 0 &&
            countEdges(gs, row-1, col) == 2) return 1;
    } else {
        int row = r1, col = c1;
        if (col < COLS && gs->board.owner[row][col] == 0 &&
            countEdges(gs, row, col) == 2) return 1;
        if (col > 0    && gs->board.owner[row][col-1] == 0 &&
            countEdges(gs, row, col-1) == 2) return 1;
    }
    return 0;
}

static int applyMoveLocal(GameState *gs, Move m)
{
    int r1 = m.r1, c1 = m.c1, r2 = m.r2, c2 = m.c2;

    if (r1 > r2 || (r1 == r2 && c1 > c2)) {
        int t;
        t = r1; r1 = r2; r2 = t;
        t = c1; c1 = c2; c2 = t;
    }

    if (r1 == r2 && c2 == c1 + 1) {
        int drawn = 0;
        if (r1 < ROWS && !gs->edges.top[r1][c1])
            { gs->edges.top[r1][c1] = 1; drawn = 1; }
        if (r1 > 0    && !gs->edges.bottom[r1-1][c1])
            { gs->edges.bottom[r1-1][c1] = 1; drawn = 1; }
        return drawn;
    }

    if (c1 == c2 && r2 == r1 + 1) {
        int drawn = 0;
        if (c1 < COLS && !gs->edges.left[r1][c1])
            { gs->edges.left[r1][c1] = 1; drawn = 1; }
        if (c1 > 0    && !gs->edges.right[r1][c1-1])
            { gs->edges.right[r1][c1-1] = 1; drawn = 1; }
        return drawn;
    }

    return 0;
}

static int claimBoxesLocal(GameState *gs, char player)
{
    int claimed = 0;
    int r, c;
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (gs->board.owner[r][c] == 0 &&
                gs->edges.top   [r][c] &&
                gs->edges.bottom[r][c] &&
                gs->edges.left  [r][c] &&
                gs->edges.right [r][c])
            {
                gs->board.owner[r][c] = player;
                gs->claimedBoxes++;
                claimed++;
                if (player == 'B') gs->scoreB++;
                else               gs->scoreA++;
            }
        }
    }
    return claimed;
}

static int dfsChain(const GameState *gs,
                    int visited[ROWS][COLS],
                    int r, int c)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return 0;
    if (visited[r][c])                              return 0;
    if (gs->board.owner[r][c] != 0)                return 0;
    if (countEdges(gs, r, c) < 2)                  return 0;

    visited[r][c] = 1;

    int size = 1;

    if (!gs->edges.top   [r][c]) size += dfsChain(gs, visited, r - 1, c);
    if (!gs->edges.bottom[r][c]) size += dfsChain(gs, visited, r + 1, c);
    if (!gs->edges.left  [r][c]) size += dfsChain(gs, visited, r, c - 1);
    if (!gs->edges.right [r][c]) size += dfsChain(gs, visited, r, c + 1);

    return size;
}

static int totalChainValue(const GameState *gs)
{
    int visited[ROWS][COLS];
    int total = 0;
    int r, c;

    memset(visited, 0, sizeof(visited));

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (!visited[r][c] &&
                gs->board.owner[r][c] == 0 &&
                countEdges(gs, r, c) >= 2)
            {
                int size = dfsChain(gs, visited, r, c);
                if (size >= 2)
                    total += size * size;
            }
        }
    }

    return total;
}

static int evaluate(const GameState *gs)
{
    int score = 10 * (gs->scoreB - gs->scoreA);

    int chainVal = totalChainValue(gs);
    score -= CHAIN_PENALTY * chainVal;

    Move moves[MAX_MOVES];
    int total = getAllMoves(gs, moves, MAX_MOVES);
    int safeMoves = 0;
    int i;
    for (i = 0; i < total; i++) {
        if (!givesOpponentBox(gs, moves[i]))
            safeMoves++;
    }
    score += SAFE_BONUS * safeMoves;

    return score;
}

static int orderMoves(const GameState *gs,
                      Move *moves, int total,
                      Move *ordered)
{
    Move good[MAX_MOVES], safe[MAX_MOVES], bad[MAX_MOVES];
    int  g = 0, s = 0, b = 0, i;

    for (i = 0; i < total; i++) {
        if      (moveCompletesBox(gs, moves[i]))  good[g++] = moves[i];
        else if (!givesOpponentBox(gs, moves[i])) safe[s++] = moves[i];
        else                                       bad [b++] = moves[i];
    }

    int idx = 0;
    for (i = 0; i < g; i++) ordered[idx++] = good[i];
    for (i = 0; i < s; i++) ordered[idx++] = safe[i];
    for (i = 0; i < b; i++) ordered[idx++] = bad [i];

    return idx;
}

static int minimax(GameState gs,
                   int depth,
                   int alpha,
                   int beta,
                   int isMaximising,
                   int extraTurns)
{
    if (gs.claimedBoxes == gs.totalBoxes)
        return 10 * (gs.scoreB - gs.scoreA);

    if (depth == 0)
        return evaluate(&gs);

    Move moves[MAX_MOVES];
    int total = getAllMoves(&gs, moves, MAX_MOVES);
    if (total == 0)
        return evaluate(&gs);

    Move ordered[MAX_MOVES];
    orderMoves(&gs, moves, total, ordered);

    char player = isMaximising ? 'B' : 'A';
    int i;

    if (isMaximising) {
        int best = INT_MIN;
        for (i = 0; i < total; i++) {
            GameState next = gs;
            applyMoveLocal(&next, ordered[i]);
            int claimed = claimBoxesLocal(&next, player);

            int val;
            if (claimed > 0 && extraTurns < MAX_EXTRA_TURNS)
                val = minimax(next, depth, alpha, beta, 1, extraTurns + 1);
            else
                val = minimax(next, depth - 1, alpha, beta, 0, 0);

            if (val > best)  best  = val;
            if (val > alpha) alpha = val;
            if (beta <= alpha) break;
        }
        return best;

    } else {
        int best = INT_MAX;
        for (i = 0; i < total; i++) {
            GameState next = gs;
            applyMoveLocal(&next, ordered[i]);
            int claimed = claimBoxesLocal(&next, player);

            int val;
            if (claimed > 0 && extraTurns < MAX_EXTRA_TURNS)
                val = minimax(next, depth, alpha, beta, 0, extraTurns + 1);
            else
                val = minimax(next, depth - 1, alpha, beta, 1, 0);

            if (val < best)  best = val;
            if (val < beta)  beta = val;
            if (beta <= alpha) break;
        }
        return best;
    }
}

BotStrategy botMove(const GameState *gs, int *r1, int *c1, int *r2, int *c2)
{
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }

    Move moves[MAX_MOVES];
    int total = getAllMoves(gs, moves, MAX_MOVES);

    int i;
    for (i = 0; i < total; i++) {
        if (moveCompletesBox(gs, moves[i])) {
            *r1 = moves[i].r1; *c1 = moves[i].c1;
            *r2 = moves[i].r2; *c2 = moves[i].c2;
            return BOT_COMPLETES_BOX;
        }
    }

    Move ordered[MAX_MOVES];
    orderMoves(gs, moves, total, ordered);

    int bestScore = INT_MIN;
    int bestIdx   = -1;

    for (i = 0; i < total; i++) {
        GameState next = *gs;
        applyMoveLocal(&next, ordered[i]);
        int claimed = claimBoxesLocal(&next, 'B');

        int val;
        if (claimed > 0)
            val = minimax(next, MAX_DEPTH, INT_MIN, INT_MAX, 1, 1);
        else
            val = minimax(next, MAX_DEPTH - 1, INT_MIN, INT_MAX, 0, 0);

        if (val > bestScore) {
            bestScore = val;
            bestIdx   = i;
        }
    }

    if (bestIdx == -1) bestIdx = rand() % total;

    *r1 = ordered[bestIdx].r1; *c1 = ordered[bestIdx].c1;
    *r2 = ordered[bestIdx].r2; *c2 = ordered[bestIdx].c2;

    return givesOpponentBox(gs, ordered[bestIdx])
           ? BOT_FORCED_MOVE
           : BOT_SAFE_MOVE;
}
