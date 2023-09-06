CC = gcc
COMMON_FLAGS = -Wall -Wextra -Wpedantic
CFLAGS = $(COMMON_FLAGS) -O3
DB_FLAGS = $(COMMON_FLAGS) -ggdb3
LK_FLAGS = -Iinclude -Llib -lcurses

SRC_DIR = src
SRC_FILES = ./$(SRC_DIR)/latlearn.c

EXE_DIR = bin
EXE_FILE = ./$(EXE_DIR)/main

main:
	$(CC) -o $(EXE_FILE) $(SRC_FILES) $(CFLAGS) $(LK_FLAGS)
	$(EXE_FILE)

db:
	$(CC) -o $(EXE_FILE) $(SRC_FILES) $(DB_FLAGS) $(LK_FLAGS)
	gdb $(EXE_FILE)