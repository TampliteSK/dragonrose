// pvtable.c

#include "stdio.h"
#include "defs.h"

S_HASHTABLE HashTable[1];

int GetPvLine(const int depth, S_BOARD *pos, const S_HASHTABLE *table) {

	ASSERT(depth < MAX_DEPTH && depth >= 1);

	int move = ProbePvMove(pos, table);
	int count = 0;
	
	while(move != NOMOVE && count < depth) {
	
		ASSERT(count < MAX_DEPTH);
	
		if( MoveExists(pos, move) ) {
			MakeMove(pos, move);
			pos->PvArray[count++] = move;
		} else {
			break;
		}		
		move = ProbePvMove(pos, table);	
	}
	
	while(pos->ply > 0) {
		TakeMove(pos);
	}
	
	return count;
	
}

void ClearHashTable(S_HASHTABLE *table) {

  	S_HASHENTRY *tableEntry;
  
	for (tableEntry = table->pTable; tableEntry < table->pTable + table->maxEntries; tableEntry++) {
		tableEntry->posKey = 0ULL;
		tableEntry->move = NOMOVE;
		tableEntry->depth = 0;
		tableEntry->score = 0;
		tableEntry->flags = 0;
		tableEntry->age = 0;
	}
	table->newWrite = 0;
	table->currentAge = 0;
}

void InitHashTable(S_HASHTABLE *table, const int MB) {  
	
	int HashSize = 0x100000 * MB;
	table->maxEntries = 0;
    table->maxEntries = HashSize / sizeof(S_HASHENTRY);
    table->maxEntries -= 2;
	
	if(table->pTable!=NULL) {
		free(table->pTable);
	}
		
    table->pTable = (S_HASHENTRY *) malloc(table->maxEntries * sizeof(S_HASHENTRY));
	if(table->pTable == NULL) {
		// printf("Hash Allocation Failed, trying %dMB...\n",MB/2);
		InitHashTable(table, MB/2);
	} else {
		ClearHashTable(table);
		// printf("HashTable init complete with %d entries\n",table->maxEntries);
	}
	
}

int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE *table, int *move, int *score, int alpha, int beta, int depth) {

	int index = pos->posKey % table->maxEntries;
	
	ASSERT(index >= 0 && index <= table->maxEntries - 1);
    ASSERT(depth >= 1 && depth < MAX_DEPTH);
    ASSERT(alpha < beta);
    ASSERT(alpha >= -INF_BOUND && alpha <= INF_BOUND);
    ASSERT(beta >= -INF_BOUND && beta <= INF_BOUND);
    ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);
	
	if( table->pTable[index].posKey == pos->posKey ) {
		*move = table->pTable[index].move;
		if(table->pTable[index].depth >= depth){
			table->hit++;
			
			ASSERT(table->pTable[index].depth >= 1 && table->pTable[index].depth < MAX_DEPTH);
            ASSERT(table->pTable[index].flags >= HFALPHA && table->pTable[index].flags <= HFEXACT);
			
			*score = table->pTable[index].score;
			if(*score > ISMATE) *score -= pos->ply;
            else if(*score < -ISMATE) *score += pos->ply;
			
			switch(table->pTable[index].flags) {
                case HFALPHA: if(*score<=alpha) {
                    *score=alpha;
                    return TRUE;
                    }
                    break;
                case HFBETA: if(*score>=beta) {
                    *score=beta;
                    return TRUE;
                    }
                    break;
                case HFEXACT:
                    return TRUE;
                    break;
                default: ASSERT(FALSE); break;
            }
		}
	}
	
	return FALSE;
}

void StoreHashEntry(S_BOARD *pos, S_HASHTABLE *table, const int move, int score, const int flags, const int depth) {

	int index = pos->posKey % table->maxEntries;
	
	ASSERT(index >= 0 && index <= table->maxEntries - 1);
	ASSERT(depth >= 1 && depth < MAX_DEPTH);
    ASSERT(flags >= HFALPHA && flags <= HFEXACT);
    ASSERT(score >= -INF_BOUND && score <= INF_BOUND);
    ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);
	
	uint8_t replace = FALSE;

	// Check if there is an existing entry at a certain index
	if( table->pTable[index].posKey == 0) {
		table->newWrite++;
		table->numEntries++;
		replace = TRUE;
	} else {
		int entryAge = table->pTable[index].age; 
		// Existing entry is old, or equal age but shallower depth
		if (entryAge < table->currentAge) {
			// Existing entry is old, so we replace it
			replace = TRUE;
			table->overWrite++;
		} else if (entryAge == table->currentAge && table->pTable[index].depth < depth) {
			// Existing entry is equal age, but at a shallower depth, we still replace it
			replace = TRUE;
			table->overWrite++;
		}
	}

	if (!replace) return; // No need to overwrite the entry
	
	int written_score = score;
	if (score > ISMATE) {
		written_score += pos->ply;
	} else if (score < -ISMATE) {
		written_score -= pos->ply;
	}
	
	table->pTable[index].move = move;
    table->pTable[index].posKey = pos->posKey;
	table->pTable[index].flags = flags;
	table->pTable[index].score = written_score;
	table->pTable[index].depth = depth;
	table->pTable[index].age = table->currentAge;
}

int ProbePvMove(const S_BOARD *pos, const S_HASHTABLE *table) {

	int index = pos->posKey % table->maxEntries;
	ASSERT(index >= 0 && index <= table->maxEntries - 1);
	
	if( table->pTable[index].posKey == pos->posKey ) {
		return table->pTable[index].move;
	}
	
	return NOMOVE;
}
















