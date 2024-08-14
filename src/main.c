// main.c

#include <stdio.h>
#include "defs.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "main.h"

#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"
#define PERFT "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

int main(int argc, char *argv[]) {

	AllInit();

	S_BOARD pos[1];
    S_SEARCHINFO info[1];
    info->quit = FALSE;
	HashTable->pTable = NULL;
    InitHashTable(HashTable, 64);

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
	// CLI Arguments
    for(int ArgNum = 0; ArgNum < argc; ++ArgNum) {
    	if (strncmp(argv[ArgNum], "bench", 5) == 0) {
			clock_t start, end;
			double time;
			unsigned long total_nodes = 0;
			
			start = clock();
			for (int index = 0; index < 50; ++index) {
				printf("\n=== Benching position %d/%d ===\n", index, 49);
				printf("Position: %s\n", bench_positions[index]);
				ParseFen(bench_positions[index], pos);
				ParseGo("go depth 7", info, pos, HashTable);
				total_nodes += info->nodes;
			}
			end = clock();

			time = ( (double)(end - start) ) / CLOCKS_PER_SEC;
			printf("\n-#-#- Benchmark results -#-#-\n");
			printf("%lu nodes %d nps\n", total_nodes, (int)( total_nodes / time ));

			// Quit after benching has finished
			info->quit = TRUE; 
            return 0;
		}
    }

	char line[256];
	while (TRUE) {
		memset(&line[0], 0, sizeof(line));

		fflush(stdout);
		if (!fgets(line, 256, stdin))
			continue;
		if (line[0] == '\n')
			continue;
		if (!strncmp(line, "uci", 3)) {
			Uci_Loop(pos, info);
			if(info->quit == TRUE) break;
			continue;
		} else if(!strncmp(line, "perft", 4))	{
			ParseFen(START_FEN, pos);
			PerftTest(10, pos);
		} else if(!strncmp(line, "quit", 4))	{
			break;
		} else if(!strncmp(line, "test", 4))	{
			// Place for debugging the engine
			// ParseFen("r1bqr1k1/pp3pbp/6p1/3nn3/8/2P2NQ1/PPBN2PP/R1B2RK1 b - - 1 15", pos);
			// PrintBitBoard(prepare_occupancy(pos->occupancy[WHITE], A2));
			// PrintBitBoard(prepare_occupancy(pos->occupancy[BLACK], A2));
		} 
	}

	free(HashTable->pTable);
	CleanPolyBook(); // Free book entries
	return 0;
}








