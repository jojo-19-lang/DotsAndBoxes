#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "game.h"
#include "bot.h"

#define PORT     9090
#define BUF_SIZE 256

static GameState       gs;
static pthread_mutex_t game_lock;
static int             client_fd[2];

static void send_msg(int fd, const char *msg)
{
    char buf[BUF_SIZE * 8];
    int  len = snprintf(buf, sizeof(buf), "%s\n", msg);
    send(fd, buf, len, 0);
}

static void serialise(const GameState *g, char *buf, int maxlen)
{
    int pos = 0, r, c;
    pos += snprintf(buf + pos, maxlen - pos,
                    "%d %d %c %d",
                    g->scoreA, g->scoreB,
                    g->currentPlayer, g->claimedBoxes);
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            pos += snprintf(buf + pos, maxlen - pos,
                            " %d", g->edges.top[r][c]);
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            pos += snprintf(buf + pos, maxlen - pos,
                            " %d", g->edges.bottom[r][c]);
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            pos += snprintf(buf + pos, maxlen - pos,
                            " %d", g->edges.left[r][c]);
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            pos += snprintf(buf + pos, maxlen - pos,
                            " %d", g->edges.right[r][c]);
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            pos += snprintf(buf + pos, maxlen - pos, " %c",
                            g->board.owner[r][c]
                            ? g->board.owner[r][c] : '0');
}

static void send_board(int fd)
{
    char serial[BUF_SIZE * 8];
    char msg   [BUF_SIZE * 8 + 8];
    serialise(&gs, serial, sizeof(serial));
    snprintf(msg, sizeof(msg), "BOARD %s", serial);
    send_msg(fd, msg);
}

typedef struct {
    int player_index;
    int fd;
} ThreadArg;

static void *client_thread(void *arg)
{
    ThreadArg *ta  = (ThreadArg *)arg;
    int        idx = ta->player_index;
    char       me  = (idx == 0) ? 'A' : 'B';
    int        fd  = ta->fd;
    free(ta);

    char recvbuf[BUF_SIZE];
    char sndbuf [BUF_SIZE];

    snprintf(sndbuf, sizeof(sndbuf), "WELCOME %c", me);
    send_msg(fd, sndbuf);

    while (1) {

        while (1) {
            pthread_mutex_lock(&game_lock);
            int my_turn   = (gs.currentPlayer == me);
            int game_done = isGameOver(&gs);
            pthread_mutex_unlock(&game_lock);
            if (my_turn || game_done) break;
            send_msg(fd, "WAIT");
            usleep(300000);
        }

        pthread_mutex_lock(&game_lock);

        if (isGameOver(&gs)) {
            send_board(fd);
            char winner = getWinner(&gs);
            snprintf(sndbuf, sizeof(sndbuf),
                     "GAME_OVER %c %d %d",
                     winner, gs.scoreA, gs.scoreB);
            send_msg(fd, sndbuf);
            pthread_mutex_unlock(&game_lock);
            break;
        }

        send_board(fd);
        send_msg(fd, "YOUR_TURN");
        pthread_mutex_unlock(&game_lock);

        int got = recv(fd, recvbuf, sizeof(recvbuf) - 1, 0);
        if (got <= 0) {
            printf("[Server] Player %c disconnected.\n", me);
            break;
        }
        recvbuf[got] = '\0';
        char *nl = strchr(recvbuf, '\n');
        if (nl) *nl = '\0';

        int r1, c1, r2, c2;
        if (sscanf(recvbuf, "MOVE %d %d %d %d",
                   &r1, &c1, &r2, &c2) != 4) {
            send_msg(fd, "INVALID_MOVE");
            continue;
        }

        pthread_mutex_lock(&game_lock);

        int ok = applyMove(&gs, r1, c1, r2, c2);
        if (!ok) {
            send_msg(fd, "INVALID_MOVE");
            pthread_mutex_unlock(&game_lock);
            continue;
        }

        send_msg(fd, "MOVE_OK");

        int boxes = checkBoxes(&gs, me);
        if (boxes > 0) {
            snprintf(sndbuf, sizeof(sndbuf), "BOX_CLAIMED %d", boxes);
            send_msg(fd, sndbuf);
        } else {
            gs.currentPlayer = (me == 'A') ? 'B' : 'A';
            send_msg(fd, "NEXT_TURN");
        }

        pthread_mutex_unlock(&game_lock);
    }

    close(fd);
    return NULL;
}

int main(void)
{
    int server_fd;
    struct sockaddr_in server_addr;
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(server_fd, 5) < 0) {
        perror("listen"); return 1;
    }

    printf("==========================================\n");
    printf("   Dots and Boxes  - Network Server\n");
    printf("   Waiting for 2 players on port %d...\n", PORT);
    printf("==========================================\n");

    initGame(&gs);
    pthread_mutex_init(&game_lock, NULL);

    int i;
    for (i = 0; i < 2; i++) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        client_fd[i] = accept(server_fd,
                              (struct sockaddr *)&caddr, &clen);
        if (client_fd[i] < 0) { perror("accept"); return 1; }
        printf("[Server] Player %c connected from %s\n",
               (i == 0) ? 'A' : 'B',
               inet_ntoa(caddr.sin_addr));
    }

    pthread_t threads[2];
    for (i = 0; i < 2; i++) {
        ThreadArg *ta = malloc(sizeof(ThreadArg));
        ta->player_index = i;
        ta->fd           = client_fd[i];
        pthread_create(&threads[i], NULL, client_thread, ta);
    }

    for (i = 0; i < 2; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&game_lock);
    close(server_fd);

    char winner = getWinner(&gs);
    printf("\n=== GAME OVER ===\n");
    if (winner == 'T')
        printf("TIE! Both scored %d.\n", gs.scoreA);
    else
        printf("Player %c WINS with %d boxes!\n",
               winner, winner == 'A' ? gs.scoreA : gs.scoreB);

    return 0;
}
