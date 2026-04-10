#include <stdio.h>
#include "game.h"
#include "bot.h"

int main(void)
{
    GameState gs;
    int mode;

    printf("==========================================\n");
    printf("        DOTS AND BOXES  (4 x 5)          \n");
    printf("==========================================\n");
    printf("  1. Two Player\n");
    printf("  2. Player vs Bot\n");
    printf("Choose mode: ");
    fflush(stdout);

    if (scanf("%d", &mode) != 1 || (mode != 1 && mode != 2)) {
        printf("Invalid choice. Defaulting to Two Player.\n");
        mode = MODE_TWO_PLAYER;
    } else {
        mode = (mode == 2) ? MODE_BOT : MODE_TWO_PLAYER;
    }

    initGame(&gs);

    printf("\n  Moves: row1 col1 row2 col2\n");
    printf("  Dots:  rows 0-%d, cols 0-%d\n", ROWS, COLS);
    printf("==========================================\n");

    while (!isGameOver(&gs)) {
        printBoard(&gs);
        printScores(&gs);

        int r1, c1, r2, c2;

        if (mode == MODE_BOT && gs.currentPlayer == 'B') {
            printf("\nBot (Player B) is thinking...\n");
            BotStrategy strat = botMove(&gs, &r1, &c1, &r2, &c2);

            if      (strat == BOT_COMPLETES_BOX)
                printf("Bot completes a box! Move: %d %d %d %d\n", r1, c1, r2, c2);
            else if (strat == BOT_SAFE_MOVE)
                printf("Bot plays safe. Move: %d %d %d %d\n", r1, c1, r2, c2);
            else
                printf("Bot makes a forced move. Move: %d %d %d %d\n", r1, c1, r2, c2);

        } else {
            printf("\nPlayer %c's turn. Enter the row and column of the first dot (e.g., A0 -> 0 0) and second dot:\n",
                   gs.currentPlayer);
            fflush(stdout);

            int gotInput = getMove(&r1, &c1, &r2, &c2, gs.currentPlayer);
            if (!gotInput) {
                printf("Bad input. Try again.\n");
                continue;
            }

            if (r1 < 0 || r1 > ROWS || c1 < 0 || c1 > COLS ||
                r2 < 0 || r2 > ROWS || c2 < 0 || c2 > COLS) {
                printf("Out of range! Rows 0-%d, Cols 0-%d. Try again.\n", ROWS, COLS);
                continue;
            }
        }

        int ok = applyMove(&gs, r1, c1, r2, c2);
        if (!ok) {
            if (mode == MODE_BOT && gs.currentPlayer == 'B')
                printf("Bot error: invalid move generated.\n");
            else
                printf("Invalid move (already drawn or not adjacent). Try again.\n");
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
