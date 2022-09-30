CFLAGS=-O2 -Wall -Wextra
LDFLAGS=-lSDL2
SRC_DIR=src

.PHONY: all
all: munch
munch: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f munch
