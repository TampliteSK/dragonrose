# Compiler
# Run "make CC=<compiler>" and replace <compiler> with either gcc or clang
# If left unspecified, gcc is used as default
CC ?= gcc 
SRCS = $(wildcard src/*.c)

# Attaching "EXE=<name>" to the above command allows you to rename the executable to <name>. By default it is "Dragonrose.exe"
EXE ?= Dragonrose


#
# COMPILER FLAGS
#


# Default flags
WARN_FLAGS = -Wall -Werror -Wextra -Wno-error=vla -Wpedantic -Wno-unused-command-line-argument
OPT_FLAGS = -Ofast -march=native -funroll-loops

# Detect Clang
ifeq ($(CC), clang)
	OPT_FLAGS = -O3 -ffast-math -flto -fuse-ld=lld -march=native -funroll-loops
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