CC ?= gcc
SRCS = $(wildcard *.c)
EXE ?= Dragonrose_027_Dev
OPT_FLAGS = -O3 -march=native

all:
	$(CC) $(SRCS) -o $(EXE) $(OPT_FLAGS) -lm
# -lm is added to fix errors when compiling on Linux (Ubuntu)