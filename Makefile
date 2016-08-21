CC = gcc

rename: main.c
	@echo "generating rename ..."
	@$(CC) $< -o rename

.PHONY: clean

clean:	
	@echo "cleaning up ..."
	@rm rename
