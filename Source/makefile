CC ?= gcc
SRCS = $(wildcard *.c)
EXE ?= Dragonrose_027_20240712
OPT_FLAGS = -O3 -march=native -mavx2

all:
	$(CC) $(SRCS) -o $(EXE) $(OPT_FLAGS)