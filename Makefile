CFLAGS=-O2 -Wall -Wextra
LDFLAGS=-lSDL2
SRC_DIR=src
INSTALL_PREFIX=/usr

.PHONY: all
all: munch
munch: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f munch

.PHONY: install
install:
	mkdir -p $(INSTALL_PREFIX)/bin
	mkdir -p $(INSTALL_PREFIX)/share/man/man6
	cp munch $(INSTALL_PREFIX)/bin/munch
	cp man/munch.6 $(INSTALL_PREFIX)/share/man/man6/munch.6

.PHONY: uninstall
uninstall:
	rm $(INSTALL_PREFIX)/bin/munch
	rm $(INSTALL_PREFIX)/share/man/man6/munch.6
