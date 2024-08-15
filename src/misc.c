// misc.c

#include "defs.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#ifdef WIN32
#include "windows.h"
// #else
// #include "sys/time.h"
#endif

/*
uint64_t GetTimeMs() {
#ifdef WIN32
    return GetTickCount();
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
#endif
}
*/

// Less accurate due to not having microseconds but should work?
uint64_t GetTimeMs() {
#ifdef WIN32
    return GetTickCount();
#else
    time_t T = time(NULL);
    struct tm curr_time = *localtime(&T);
    return curr_time.tm_hour*3600*1000 + curr_time.tm_min*60000 + curr_time.tm_sec*1000;
#endif
}


inline uint8_t isLightSq(uint8_t sq) {
	  return !( (sq % 2) ^ ( ( sq / 10 ) % 2) );
}

// Determines if the endgame involves opposite-coloured bishops
inline uint8_t isOppColBishops(const S_BOARD *pos) {

    // Board has no bishops for either side
    if ( (pos->pceNum[wB] != 1) || (pos->pceNum[bB] != 1) ) {
        return FALSE;
    }

    uint8_t W_bishop_sq = pos->pList[wB][0];
    uint8_t B_bishop_sq = pos->pList[bB][0];

    if (isLightSq(W_bishop_sq) != isLightSq(B_bishop_sq)) {
        return TRUE;
    } else {
        return FALSE;
    }

}

// Manhattan distance
inline uint8_t dist_between_squares(uint8_t sq_1, uint8_t sq_2) {
    uint8_t file_1 = FilesBrd[sq_1];
    uint8_t rank_1 = RanksBrd[sq_1];
    uint8_t file_2 = FilesBrd[sq_2];
    uint8_t rank_2 = RanksBrd[sq_2];

    return abs(file_1 - file_2) + abs(rank_1 - rank_2);
}

// Manhattan distance alt defintiion
inline int8_t max_between_squares(uint8_t sq_1, uint8_t sq_2) {
    uint8_t file_1 = FilesBrd[sq_1];
    uint8_t rank_1 = RanksBrd[sq_1];
    uint8_t file_2 = FilesBrd[sq_2];
    uint8_t rank_2 = RanksBrd[sq_2];

    return max( abs(file_1 - file_2), abs(rank_1 - rank_2) );
}

