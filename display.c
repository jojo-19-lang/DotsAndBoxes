#include <stdio.h>
#include "game.h"

void printBoard(const GameState *gs)
{
    int r, c;

    printf("\n");
    printf("  ");
    for (c = 0; c <= COLS; c++) printf("%d ", c);
    printf("\n");

    for (r = 0; r <= ROWS; r++) {
        /* Dot row */
        printf("%d ", r);
        for (c = 0; c <= COLS; c++) {
            printf(".");
            if (c < COLS) {
                int drawn = 0;
                if (r < ROWS && gs->edges.top   [r][c]) drawn = 1;
                if (r > 0    && gs->edges.bottom [r-1][c]) drawn = 1;
                printf("%c", drawn ? '-' : ' ');
            }
        }
        printf("\n");

        /* Interior row */
        if (r < ROWS) {
            printf("  ");
            for (c = 0; c <= COLS; c++) {
                int vDrawn = 0;
                if (c < COLS && gs->edges.left [r][c]) vDrawn = 1;
                if (c > 0    && gs->edges.right[r][c-1]) vDrawn = 1;
                printf("%c", vDrawn ? '|' : ' ');
                if (c < COLS) {
                    char o = gs->board.owner[r][c];
                    printf("%c", o ? o : ' ');
                }
            }
            printf("\n");
        }
    }
    printf("\n");
}

void printScores(const GameState *gs)
{
    printf("******************************************\n");
    printf("Player A score: %d\n", gs->scoreA);
    printf("Player B score: %d\n", gs->scoreB);
    printf("******************************************\n");
}
