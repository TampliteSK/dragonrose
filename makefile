CC ?= gcc
SRCS = $(wildcard src/*.c)
EXE ?= Dragonrose
WARN_FLAGS = -Wall -Wextra
OPT_FLAGS = -O3 -march=native

all:
	$(CC) $(SRCS) -o $(EXE) $(WARN_FLAGS) $(OPT_FLAGS) -lm
# -lm is added to fix errors when compiling on Linux (Ubuntu)