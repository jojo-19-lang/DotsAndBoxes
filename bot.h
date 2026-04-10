#ifndef BOT_H
#define BOT_H

#include "game.h"

typedef struct {
    int r1, c1, r2, c2;
} Move;

typedef enum {
    BOT_COMPLETES_BOX = 0,
    BOT_SAFE_MOVE     = 1,
    BOT_FORCED_MOVE   = 2
} BotStrategy;

BotStrategy botMove(const GameState *gs, int *r1, int *c1, int *r2, int *c2);
#endif
