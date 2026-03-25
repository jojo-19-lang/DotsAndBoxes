#include <stdio.h>
#include "game.h"

int getMove(int *r1, int *c1, int *r2, int *c2, char player)
{
    if (scanf("%d %d %d %d", r1, c1, r2, c2) != 4) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        return 0;
    }
    return 1;
}
