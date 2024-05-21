CC=gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.c util.c files.c

trsh: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f *.o
