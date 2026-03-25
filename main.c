#include <stdio.h>
#include "game.h"

int main(void)
{
    GameState gs;
    initGame(&gs);

    printf("==========================================\n");
    printf("        DOTS AND BOXES  (4 x 5)           \n");
    printf("  Players: A and B                         \n");
    printf("  Moves:  row1 col1 row2 col2              \n");
    printf("  Dots:   rows 0-%d, cols 0-%d             \n", ROWS, COLS);
    printf("==========================================\n");

    while (!isGameOver(&gs)) {
        printBoard(&gs);
        printScores(&gs);

        printf("\nPlayer %c's turn. Enter the row and column of the first dot (e.g., A0 -> 0 0) and second dot:\n",
               gs.currentPlayer);
        fflush(stdout);

        int r1, c1, r2, c2;
        int gotInput = getMove(&r1, &c1, &r2, &c2, gs.currentPlayer);

        if (!gotInput) {
            printf("Bad input. Try again.\n");
            continue;
        }

        if (r1 < 0 || r1 > ROWS || c1 < 0 || c1 > COLS ||
            r2 < 0 || r2 > ROWS || c2 < 0 || c2 > COLS) {
            printf("Out of range! Rows 0-%d, Cols 0-%d. Try again.\n",
                   ROWS, COLS);
            continue;
        }

        int ok = applyMove(&gs, r1, c1, r2, c2);
        if (!ok) {
            printf("Invalid move (already drawn or dots not adjacent). Try again.\n");
            continue;
        }

        int newBoxes = checkBoxes(&gs, gs.currentPlayer);

        if (newBoxes > 0) {
            printf("Player %c claimed %d box%s! Play again.\n",
                   gs.currentPlayer, newBoxes, newBoxes == 1 ? "" : "es");
        } else {
            gs.currentPlayer = (gs.currentPlayer == 'A') ? 'B' : 'A';
        }
    }

    printBoard(&gs);
    printScores(&gs);

    char winner = getWinner(&gs);
    printf("\n=== GAME OVER ===\n");

    if (winner == 'T')
        printf("It's a TIE! Both scored %d.\n", gs.scoreA);
    else
        printf("Player %c WINS with %d boxes!\n",
               winner, winner == 'A' ? gs.scoreA : gs.scoreB);

    return 0;
}
