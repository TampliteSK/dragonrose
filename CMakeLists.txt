cmake_minimum_required(VERSION 3.16)
project(Dragonrose VERSION 0.27 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
# -Werror flags Sq64ToSq120[(sq64)] in defs.h as "Iteration 64 invokes undefined behaviour"
add_compile_options(-Wall -Wextra -Wpedantic -Wno-error=vla)
# Optimizations
add_compile_options(-march=native -O3)
# Not default optimizations. Enable for slight boost on supported systems.
#add_compile_options( -DUSE_AVX512 -mavx512f)

add_executable(Dragonrose "")

set(DRAGONROSE_SOURCES
    "src/attack.c"
    "src/bitboards.c"
    "src/board.c"
    "src/data.c"
    "src/evaluate.c"
    "src/hashkeys.c"
    "src/init.c"
    "src/io.c"
    "src/main.c"
    "src/makemove.c"
    "src/misc.c"
    "src/movegen.c"
    "src/perft.c"
    "src/polybook.c"
    "src/polykeys.c"
    "src/pvtable.c"
    "src/search.c"
    "src/uci.c"
    "src/validate.c"
)

set(DRAGONROSE_HEADERS
    "src/defs.h"
    "src/main.h"
    "src/polykeys.h"
)

set(DRAGONROSE_BOOK
    "src/VICEbook.bin"
)

target_sources(Dragonrose
    PRIVATE
        ${DRAGONROSE_SOURCES}
    PUBLIC
        ${DRAGONROSE_HEADERS}
        ${DRAGONROSE_BOOK}
)