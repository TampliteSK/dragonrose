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
    
    for(int ArgNum = 0; ArgNum < argc; ++ArgNum) {
    	if (strncmp(argv[ArgNum], "bench", 5) == 0) {
			clock_t start, end;
			double time;
			unsigned long total_nodes = 0;
			
			start = clock();
			for (int index = 0; index < 50; ++index) {
				printf("\n=== Benching position %d/%d ===\n", index, 49);
				printf("Position: %s\n", bench_positions[index]);

				// Allocate a long string that can contain "position " and also the FEN
				size_t buffer_size = strlen("position fen ") + strlen(bench_positions[index]) + 1;
    			char *position_str = malloc(buffer_size);
				strcpy(position_str, "position fen ");
				strcat(position_str, bench_positions[index]);
				// printf("position_str = %s\n", position_str);

				ParsePosition(position_str, pos);
				free(position_str);
				ParseGo("go depth 7", info, pos, HashTable);
				total_nodes += info->nodes;
				// printf("Nodes: %lu\n", total_nodes);
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

	Uci_Loop(pos, info);

	free(HashTable->pTable);
	CleanPolyBook();
	return 0;
}








