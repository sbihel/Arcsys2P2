.PHONY: doc clean doc_clean

OBJECTS=7colors.o main.o game.o ai.o server.o
EXECUTABLE=7colors
CFLAGS=-Wall -Wextra -std=gnu99

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	gcc $^ -o $@ -lm

%.o: %.c
	gcc $(CFLAGS) $(@:.o=.c) -c -g

clean:
	rm -f *.o *.dSYM
	rm -f $(EXECUTABLE)
	
doc:
	doxygen

doc_clean:
	rm -rf doc
	
