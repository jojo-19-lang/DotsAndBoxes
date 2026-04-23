CC     = gcc
CFLAGS = -Wall -Werror -g
LFLAGS = -pthread

COMMON_OBJ = logic.o display.o input.o

all: dots_and_boxes server client

dots_and_boxes: main.o bot.o $(COMMON_OBJ)
	$(CC) $(LFLAGS) -o $@ $^

server: server.o bot.o $(COMMON_OBJ)
	$(CC) $(LFLAGS) -o $@ $^

client: client.o $(COMMON_OBJ)
	$(CC) $(LFLAGS) -o $@ $^

%.o: %.c game.h
	$(CC) $(CFLAGS) -c $< -o $@

run: dots_and_boxes
	./dots_and_boxes

valgrind: dots_and_boxes
	valgrind --leak-check=full --track-origins=yes ./dots_and_boxes

clean:
	rm -f *.o dots_and_boxes server client

.PHONY: all run valgrind clean
