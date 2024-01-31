cmake_minimum_required(VERSION 3.16)
project(Dragonrose VERSION 0.21 LANGUAGES C)

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
    "../Source/Engine/attack.c"
    "../Source/Engine/bitboards.c"
    "../Source/Engine/board.c"
    "../Source/Engine/data.c"
    "../Source/Engine/evaluate.c"
    "../Source/Engine/hashkeys.c"
    "../Source/Engine/init.c"
    "../Source/Engine/io.c"
    "../Source/Engine/main.c"
    "../Source/Engine/makemove.c"
    "../Source/Engine/misc.c"
    "../Source/Engine/movegen.c"
    "../Source/Engine/perft.c"
    "../Source/Engine/polybook.c"
    "../Source/Engine/polykeys.c"
    "../Source/Engine/pvtable.c"
    "../Source/Engine/search.c"
    "../Source/Engine/uci.c"
    "../Source/Engine/validate.c"
    "../Source/Engine/xboard.c"
)

set(DRAGONROSE_HEADERS
    "../Source/Engine/defs.h"
    "../Source/Engine/polykeys.h"
)

set(DRAGONROSE_TEXTFILES
    "../Source/Engine/performance.bin"
)

target_sources(Dragonrose
    PRIVATE
        ${DRAGONROSE_SOURCES}
    PUBLIC
        ${DRAGONROSE_HEADERS}
        ${DRAGONROSE_TEXTFILES}
)
