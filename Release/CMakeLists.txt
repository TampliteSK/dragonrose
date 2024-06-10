cmake_minimum_required(VERSION 3.16)
project(Dragonrose VERSION 0.25 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
# -Werror flags Sq64ToSq120[(sq64)] in defs.h as "Iteration 64 invokes undefined behaviour"
add_compile_options(-Wall -Wextra -Wpedantic -Wno-error=vla)
# Optimizations
add_compile_options(-march=native -Ofast -fomit-frame-pointer -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse -DUSE_AVX2 -mavx2)
#Not default optimizations. Enable for slight boost on supported systems. Aprox 64ms. From 2828 to 2764 on average.
#add_compile_options( -DUSE_AVX512 -mavx512f)

add_executable(Dragonrose "")

#[[
target_link_libraries(Dragonrose
    PRIVATE
        nnue
        m
) ]]

set(DRAGONROSE_SOURCES
    "../Source/attack.c"
    "../Source/bitboards.c"
    "../Source/board.c"
    "../Source/data.c"
    "../Source/evaluate.c"
    "../Source/hashkeys.c"
    "../Source/init.c"
    "../Source/io.c"
    "../Source/main.c"
    "../Source/makemove.c"
    "../Source/misc.c"
    "../Source/movegen.c"
    "../Source/perft.c"
    "../Source/polybook.c"
    "../Source/polykeys.c"
    "../Source/pvtable.c"
    "../Source/search.c"
    "../Source/uci.c"
    "../Source/validate.c"
)

set(DRAGONROSE_HEADERS
    "../Source/defs.h"
    "../Source/polykeys.h"
)

set(DRAGONROSE_BOOK
    "../Source/VICEbook.bin"
)

target_sources(Dragonrose
    PRIVATE
        ${DRAGONROSE_SOURCES}
    PUBLIC
        ${DRAGONROSE_HEADERS}
        ${DRAGONROSE_BOOK}
)