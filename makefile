# Compiler
# Run "make CC=xxx" and replace xxx with your choice of compiler to use another compiler (e.g. clang)
CC ?= gcc 
SRCS = $(wildcard src/*.c)

# Run "make EXE=yyy" to name the executable yyy
EXE ?= Dragonrose


#
# COMPILER FLAGS
#


# Default flags
WARN_FLAGS = -Wall -Werror -Wextra -Wno-error=vla -Wpedantic -Wno-unused-command-line-argument
OPT_FLAGS = -Ofast -march=native -funroll-loops

# Detect Clang
ifeq ($(CC), clang)
	OPT_FLAGS = -O3 -ffast-math -march=native -funroll-loops
endif

# -lm is added to fix errors when compiling on Linux (Ubuntu)
LIBS = -lm


#
# TARGETS
#


# Default target
all:
	$(CC) $(SRCS) -o $(EXE) $(OPT_FLAGS) $(LIBS)

# Clean target to remove the executable
clean:
	rm -f $(EXE)