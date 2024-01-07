// hashkeys.c
#include <stdio.h>
#include "defs.h"

U64 GeneratePosKey(const S_BOARD *pos) {

	int sq = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	
	// Piece hashing
	for(sq = 0; sq < BRD_SQ_NUM; ++sq) {
		piece = pos->pieces[sq];
		// Check if it's a piece
		if(piece!=NO_SQ && piece!=EMPTY && piece != OFFBOARD) {
			// Extra bound check
			ASSERT(piece>=wP && piece<=bK);
			finalKey ^= PieceKeys[piece][sq];
		}		
	}
	
	// Side hashing
	if(pos->side == WHITE) {
		finalKey ^= SideKey;
	}

	// En passant hashing	
	if(pos->enPas != NO_SQ) {
		ASSERT(pos->enPas>=0 && pos->enPas<BRD_SQ_NUM);
		ASSERT(SqOnBoard(pos->enPas));
		ASSERT(RanksBrd[pos->enPas] == RANK_3 || RanksBrd[pos->enPas] == RANK_6);
		finalKey ^= PieceKeys[EMPTY][pos->enPas];
	}
	
	// Castling perm hashing
	ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15);
	finalKey ^= CastleKeys[pos->castlePerm];
	
	return finalKey;
}
