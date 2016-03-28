OBJECTS=7colors.o main.o game.o ai.o
EXECUTABLE=7colors
CFLAGS=-Wall -Werror -Wextra -O3

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $^ -o $@ -lm

%.o: %.c
	gcc $(CFLAGS) $(@:.o=.c) -c -g

clean:
	rm -f *.o *.dSYM
	rm -f $(EXECUTABLE)
