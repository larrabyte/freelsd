# ----------------------------------------------------------------
# Makefile for FreeLSD's initrdgen, made by the larrabyte himself.
# ----------------------------------------------------------------
.PHONY: clean build

CC		 := gcc
WARNINGS := -Wno-stringop-truncation -Wno-format-truncation
CFLAGS   := $(WARNINGS) -Wall -Wextra -Wpedantic -O3
EXECNAME := initrdgen

$(EXECNAME): src/main.o src/utils.o
	@$(CC) $(CFLAGS) src/main.o src/utils.o -o $(EXECNAME)

src/main.o: src/main.c src/utils.h
	@$(CC) $(CFLAGS) -c src/main.c -o src/main.o
	@printf "[gcc] src/main.c compiled.\n"

src/utils.o: src/utils.c src/utils.h
	@$(CC) $(CFLAGS) -c src/utils.c -o src/utils.o
	@printf "[gcc] src/utils.c compiled.\n"

clean:
	@rm -f $(EXECNAME)
	@printf "[wipe] Deleted $(EXECNAME) executable.\n"
	@rm -f src/*.o
	@printf "[wipe] Deleted object files.\n"
