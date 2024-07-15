// uci.c

#include <stdio.h>
#include "defs.h"
#include <string.h>
#include <math.h>
#include <time.h>
#include "uci.h"

#define INPUTBUFFER 400 * 6

// go depth 6 wtime 180000 btime 100000 binc 1000 winc 1000 movetime 1000 movestogo 40
void ParseGo(char* line, S_SEARCHINFO *info, S_BOARD *pos, S_HASHTABLE *table) {

	int depth = -1, movestogo = 30,movetime = -1;
	int time = -1, inc = 0;
    char *ptr = NULL;
	info->timeset = FALSE;

	if ((ptr = strstr(line,"infinite"))) {
		;
	}

	if ((ptr = strstr(line,"binc")) && pos->side == BLACK) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"winc")) && pos->side == WHITE) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"wtime")) && pos->side == WHITE) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line,"btime")) && pos->side == BLACK) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line,"movestogo"))) {
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line,"movetime"))) {
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line,"depth"))) {
		depth = atoi(ptr + 6);
	}

	if(movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	info->starttime = GetTimeMs();
	info->depth = depth;
	
	/**********************
	*** Time Management ***
	**********************/
	
	if(time != -1) {

		info->timeset = TRUE;
		int phaseMoves = 0;

		// Rose was having issues managing time when there was >=5s increment
		// time += inc / 2; // include increment in the allocation

		// Time trouble check
		if (time < 30000 /* 30s */) {
			time /= 80;
		} else {
			// Opening phase
			if (pos->hisPly <= 30) {
				// Allocate 10% of total time
				time *= 0.1;
				phaseMoves = round((30 - pos->hisPly + (pos->side ? 0 : 1)) / 2.0); // perspective adjustment to prevent crash
				time /= phaseMoves;
			} else {
				// Early-middlegame phase
				if (pos->hisPly <= 50) {
					// Allocate 30% of total time
					time *= 0.3;
					phaseMoves = round((50 - pos->hisPly + (pos->side ? 0 : 1)) / 2.0);
					time /= phaseMoves;
				} else {
					// Late-middlegame - endgame phase
					// Allocate remaining time evenly
					time /= 35;
				}
			}
		}
		

		// time /= movestogo;
		time -= 50; // overhead
		info->stoptime = info->starttime + time + inc/2;
	}

	if(depth == -1) {
		info->depth = MAXDEPTH;
	}

	printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
		time,info->starttime,info->stoptime,info->depth,info->timeset);
	SearchPosition(pos, table, info);
}

// position fen fenstr
// position startpos
// ... moves e2e4 e7e5 b7b8q
void ParsePosition(char* lineIn, S_BOARD *pos) {

	lineIn += 9;
    char *ptrChar = lineIn;

    if(strncmp(lineIn, "startpos", 8) == 0){
        ParseFen(START_FEN, pos);
    } else {
        ptrChar = strstr(lineIn, "fen");
        if(ptrChar == NULL) {
            ParseFen(START_FEN, pos);
        } else {
            ptrChar += 4;
            ParseFen(ptrChar, pos);
        }
    }

	ptrChar = strstr(lineIn, "moves");
	int move;

	if(ptrChar != NULL) {
        ptrChar += 6;
        while(*ptrChar) {
              move = ParseMove(ptrChar,pos);
			  if(move == NOMOVE) break;
			  MakeMove(pos, move);
              pos->ply=0;
              while(*ptrChar && *ptrChar!= ' ') ptrChar++;
              ptrChar++;
        }
    }
	// PrintBoard(pos);
}

void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info) {

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	char line[INPUTBUFFER];
    printf("id name %s\n",NAME);
    printf("id author Tamplite Siphron Kents\n");
	printf("option name Hash type spin default 128 min 4 max %d\n", MAX_HASH);
	if (OPENBENCH_MODE) {
		printf("option name Threads type spin default 1 min 1 max 1\n"); // No multithreading at the moment
		printf("option name Book type check default false\n"); // The book will be provided by the test environment
	} else {
		printf("option name Book type check default true\n");
	}
    printf("uciok\n");
	
	int MB = 128; // default hash 128 MB

	while (TRUE) {
		memset(&line[0], 0, sizeof(line));
        fflush(stdout);
        if (!fgets(line, INPUTBUFFER, stdin))
        continue;

        if (line[0] == '\n')
        continue;

        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        } else if (!strncmp(line, "position", 8)) {
            ParsePosition(line, pos);
        } else if (!strncmp(line, "ucinewgame", 10)) {
			ClearHashTable(HashTable);
            ParsePosition("position startpos\n", pos);
        } else if (!strncmp(line, "go", 2)) {
            printf("Seen Go..\n");
            ParseGo(line, info, pos, HashTable);
		} else if (!strncmp(line, "run", 3)) {
            ParseFen(START_FEN, pos);
            ParseGo("go infinite", info, pos, HashTable);
        } else if (!strncmp(line, "quit", 4)) {
            info->quit = TRUE;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n", NAME);
            printf("id author Tamplite Siphron Kents\n");
            printf("uciok\n");
        } else if (!strncmp(line, "bench", 5)) {
			clock_t start, end;
			double time;
			unsigned long total_nodes = 0;
			
			start = clock();
			for (int index = 0; index < 50; ++index) {
				printf("=== Benching position %d/%d ===\n", index, 49);
				printf("Position: %s\n", bench_positions[index]);

				// Allocate a long string that can contain "position " and also the FEN
				size_t buffer_size = strlen("position fen ") + strlen(bench_positions[index]) + 1;
    			char *position_str = malloc(buffer_size);
				strcpy(position_str, "position fen ");
				strcat(position_str, bench_positions[index]);
				// printf("position_str = %s\n", position_str);

				ParsePosition(position_str, pos);
				ParseGo("go depth 7", info, pos, HashTable);
				total_nodes += info->nodes;
				// printf("Nodes: %lu\n", total_nodes);
			}
			end = clock();

			time = ( (double)(end - start) ) / CLOCKS_PER_SEC;
			printf("Benchmark results:\n");
			printf("%lu nodes %d nps\n", total_nodes, (int)( total_nodes / time ));
		} else if (!strncmp(line, "debug", 4)) {
            DebugAnalysisTest(pos, HashTable, info);
            break;
        } else if (!strncmp(line, "setoption name Hash value ", 26)) {			
			sscanf(line,"%*s %*s %*s %*s %d",&MB);
			if(MB < 4) MB = 4;
			if(MB > MAX_HASH) MB = MAX_HASH;
			printf("Set Hash to %d MB\n",MB);
			InitHashTable(HashTable, MB);
		} else if (!strncmp(line, "setoption name Book value ", 26)) {			
			char *ptrTrue = NULL;
			ptrTrue = strstr(line, "true");
			if(ptrTrue != NULL) {
				EngineOptions->UseBook = TRUE;
			} else {
				EngineOptions->UseBook = FALSE;
			}
		}
		if(info->quit) break;
    }
}













