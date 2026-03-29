CC = gcc
CFLAGS = -Wall -Werror -g -c
LFLAGS = -o

SOURCES = main.c logic.c display.c input.c
OBJECTS = $(SOURCES:.c=.o)
OUTPUT = dots_and_boxes

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) $(LFLAGS) $@ $^

%.o: %.c game.h
	$(CC) $(CFLAGS) $<
run: $(OUTPUT)
	./$(OUTPUT)

valgrind: $(OUTPUT)
	valgrind --leak-check=full --track-origins=yes ./$(OUTPUT)

clean:
	rm -f $(OBJECTS) $(OUTPUT)

.PHONY: all clean valgrind run
