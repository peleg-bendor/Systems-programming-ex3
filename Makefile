# Makefile for mini_bash program

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
TARGET = mini_bash

# Default target: build the executable
all: $(TARGET)

# Build rule: compile mini_bash.c into executable
$(TARGET): mini_bash.c
	$(CC) $(CFLAGS) mini_bash.c -o $(TARGET)

# Clean rule: remove the executable
clean:
	rm -f $(TARGET)

# Phony targets (not actual files)
.PHONY: all clean