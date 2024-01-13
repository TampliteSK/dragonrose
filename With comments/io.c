// io.c

#include "stdio.h"
#include "defs.h"

// Print square
char *PrSq(const int sq) {

	static char SqStr[3];

	int file = FilesBrd[sq];
	int rank = RanksBrd[sq];

	// Sets the string as a formatted output
	sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank));

	return SqStr;

}

// Print move in UCI notation
char *PrMove(const int move) {
	static char MvStr[6];

	// file_from, rank_from, file_to, rank_to
	int ff = FilesBrd[FROMSQ(move)];
	int rf = RanksBrd[FROMSQ(move)];
	int ft = FilesBrd[TOSQ(move)];
	int rt = RanksBrd[TOSQ(move)];

	// Check if isPromotion
	int promoted = PROMOTED(move);
	if(promoted) {
		// Prints queen by default
		char pchar = 'q';
		if(IsKn(promoted)) {
			pchar = 'n';
		} else if(IsRQ(promoted) && !IsBQ(promoted))  {
			pchar = 'r';
		} else if(!IsRQ(promoted) && IsBQ(promoted))  {
			pchar = 'b';
		}
		// print with promotion piece
		sprintf(MvStr, "%c%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt), pchar);
	} else {
		// print without promotion piece
		sprintf(MvStr, "%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt));
	}

	return MvStr;
}

int ParseMove(char *ptrChar, S_BOARD *pos) {

	ASSERT(CheckBoard(pos));

	if(ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
    if(ptrChar[3] > '8' || ptrChar[3] < '1') return NOMOVE;
    if(ptrChar[0] > 'h' || ptrChar[0] < 'a') return NOMOVE;
    if(ptrChar[2] > 'h' || ptrChar[2] < 'a') return NOMOVE;

    int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
    int to = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');

	ASSERT(SqOnBoard(from) && SqOnBoard(to));

	S_MOVELIST list[1];
    GenerateAllMoves(pos,list);
    int MoveNum = 0;
	int Move = 0;
	int PromPce = EMPTY;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		Move = list->moves[MoveNum].move;
		if(FROMSQ(Move)==from && TOSQ(Move)==to) {
			PromPce = PROMOTED(Move);
			if(PromPce!=EMPTY) {
				if(IsRQ(PromPce) && !IsBQ(PromPce) && ptrChar[4]=='r') {
					return Move;
				} else if(!IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4]=='b') {
					return Move;
				} else if(IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4]=='q') {
					return Move;
				} else if(IsKn(PromPce)&& ptrChar[4]=='n') {
					return Move;
				}
				continue;
			}
			return Move;
		}
    }

    return NOMOVE;
}

void PrintMoveList(const S_MOVELIST *list) {
	int index = 0;
	int score = 0;
	int move = 0;
	printf("MoveList:\n");

	for(index = 0; index < list->count; ++index) {

		move = list->moves[index].move;
		score = list->moves[index].score;

		printf("Move:%d > %s (score:%d)\n",index+1,PrMove(move),score);
	}
	printf("MoveList Total %d Moves:\n\n",list->count);
}














