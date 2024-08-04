# Compiler. Default is gcc
CC ?= gcc 

SRCS = $(wildcard src/*.c)
EXE ?= Dragonrose

# Compiler flags
WARN_FLAGS = -Wall -Wextra
OPT_FLAGS = -Ofast -march=native

# -lm is added to fix errors when compiling on Linux (Ubuntu)
LIBS = -lm

# Default target
all:
	$(CC) $(SRCS) -o $(EXE) $(OPT_FLAGS) $(LIBS)

# Clean target to remove the executable
clean:
	rm -f $(EXE)