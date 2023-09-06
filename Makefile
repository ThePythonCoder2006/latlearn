CC = gcc
COMMON_FLAGS = -Wall -Wextra -Wpedantic
CFLAGS = $(COMMON_FLAGS) -O3
DB_FLAGS = $(COMMON_FLAGS) -ggdb3

SRC_FILES = latlearn.c
EXE_FILE = main

main:
	$(CC) -o $(EXE_FILE) $(SRC_FILES) $(CFLAGS)
	./$(EXE_FILE)

db:
	$(CC) -o $(EXE_FILE) $(SRC_FILES) $(DB_FLAGS)
	gdb $(EXE_FILE)