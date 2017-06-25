CC = gcc
ARGS = -Wall -Wpedantic -Wextra -Wstrict-prototypes
BINARY = rename
LOCAL_BIN_PATH = ~/bin/

all: rename

rename: main.c
	@echo "generating $(BINARY) ..."
	@$(CC) $< $(ARGS) -o $(BINARY)

.PHONY: clean install uninstall

clean:	
	@echo "cleaning up ..."
	@rm $(BINARY)

install:
	@cp $(BINARY) $(LOCAL_BIN_PATH)

uninstall:
	rm $(LOCAL_BIN_PATH)$(BINARY)
