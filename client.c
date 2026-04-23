#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "game.h"

#define PORT     9090
#define BUF_SIZE 256

static int recv_line(int fd, char *buf, int maxlen)
{
    int  i = 0;
    char c;

    while (i < maxlen - 1) {
        int n = recv(fd, &c, 1, 0);
        if (n <= 0) return -1;
        if (c == '\n') break;
        buf[i++] = c;
    }

    buf[i] = '\0';
    return i;
}

static void deserialise(GameState *g, const char *data)
{
    int r, c;
    int sA, sB, claimed;
    char cur;

    sscanf(data, "%d %d %c %d", &sA, &sB, &cur, &claimed);

    g->scoreA        = sA;
    g->scoreB        = sB;
    g->currentPlayer = cur;
    g->claimedBoxes  = claimed;

    const char *p = data;

    int skip = 0;
    while (skip < 4) {
        while (*p && *p != ' ') {
            p++;
        }
        if (*p) {
            p++;
        }
        skip++;
    }

    #define READ_INT(target) do {          \
        (target) = atoi(p);                 \
        while (*p && *p != ' ') { p++; }    \
        if (*p) { p++; }                    \
    } while (0)

    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            READ_INT(g->edges.top[r][c]);

    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            READ_INT(g->edges.bottom[r][c]);

    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            READ_INT(g->edges.left[r][c]);

    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            READ_INT(g->edges.right[r][c]);

    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++) {
            char ch = *p;
            g->board.owner[r][c] = (ch == '0') ? 0 : ch;

            while (*p && *p != ' ') {
                p++;
            }
            if (*p) {
                p++;
            }
        }

    #undef READ_INT
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server-ip>\n", argv[0]);
        return 1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));

    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &saddr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP: %s\n", argv[1]);
        return 1;
    }

    if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("==========================================\n");
    printf("   Dots and Boxes  - Network Client\n");
    printf("==========================================\n");

    GameState gs;
    initGame(&gs);

    char my_player = '?';
    char line[BUF_SIZE * 8];

    while (1) {

        if (recv_line(fd, line, sizeof(line)) < 0) {
            printf("Connection lost.\n");
            break;
        }

        if (strncmp(line, "WELCOME", 7) == 0) {
            my_player = line[8];
            printf("You are Player %c.\n", my_player);
            printf("Moves format: r1 c1 r2 c2 (0-%d)\n", ROWS);
        }

        else if (strncmp(line, "BOARD", 5) == 0) {
            deserialise(&gs, line + 6);
            printBoard(&gs);
            printScores(&gs);
        }

        else if (strcmp(line, "YOUR_TURN") == 0) {
            int r1, c1, r2, c2;

            printf("\nPlayer %c - enter your move: ", my_player);
            fflush(stdout);

            while (1) {
                if (!getMove(&r1, &c1, &r2, &c2, my_player)) {
                    printf("Bad input, try again: ");
                    fflush(stdout);
                    continue;
                }

                if (r1 < 0 || r1 > ROWS || c1 < 0 || c1 > COLS ||
                    r2 < 0 || r2 > ROWS || c2 < 0 || c2 > COLS) {
                    printf("Out of range. Try again: ");
                    fflush(stdout);
                    continue;
                }
                break;
            }

            char movebuf[BUF_SIZE];
            snprintf(movebuf, sizeof(movebuf),
                     "MOVE %d %d %d %d\n", r1, c1, r2, c2);

            send(fd, movebuf, strlen(movebuf), 0);
        }

        else if (strcmp(line, "WAIT") == 0) {
            printf("Waiting for the other player...\n");
        }

        else if (strcmp(line, "MOVE_OK") == 0) {
        }

        else if (strncmp(line, "BOX_CLAIMED", 11) == 0) {
            int n = atoi(line + 12);
            printf("You claimed %d box%s! Play again.\n",
                   n, n == 1 ? "" : "es");
        }

        else if (strcmp(line, "NEXT_TURN") == 0) {
            printf("Turn passed to other player.\n");
        }

        else if (strcmp(line, "INVALID_MOVE") == 0) {
            printf("Invalid move. Try again.\n");
        }

        else if (strncmp(line, "GAME_OVER", 9) == 0) {
            char winner;
            int  sA, sB;

            sscanf(line + 10, "%c %d %d", &winner, &sA, &sB);

            printf("\n=== GAME OVER ===\n");
            printf("Player A: %d   Player B: %d\n", sA, sB);

            if (winner == 'T')
                printf("It's a TIE!\n");
            else
                printf("Player %c WINS!\n", winner);

            break;
        }
    }

    close(fd);
    return 0;
}
