CC=gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.c util.c files.c globals.h db.c
HOME_DIR := $(shell echo $$HOME)
PREFIX = $(HOME_DIR)/.local

trsh: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

install : trsh
	cp -f trsh ${DESTDIR}${PREFIX}/bin

clean:
	rm -f *.o

.PHONY: install clean
