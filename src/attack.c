// attack.c

#include <stdio.h>
#include "defs.h"

const int KnDir[8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10,	1, 10 }; // at one square
const int BiDir[4] = { -9, -11, 11, 9 }; // at one square
const int KiDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 };

// Determine if a move is check
uint8_t IsCheck(const S_BOARD *pos, int move) {
	uint8_t moving_pce = pos->pieces[FROMSQ(move)];
    uint8_t target_sq = TOSQ(move);
	uint8_t colour = pos->side;
	uint8_t opp_king_sq = pos->KingSq[!colour];
	
	int8_t dist = opp_king_sq - target_sq;

	if (IsPawn(moving_pce)) {
		if ( (colour == WHITE) && ( (dist == -9) || (dist == -11) ) ) {
			return TRUE;
		} else if ( (colour == BLACK) && ( (dist == 9) || (dist == 11) ) ) {
			return TRUE;
		}
	} else if (IsKn(moving_pce)) {
		for (int i = 0; i < 8; ++i) {
			if (dist == KnDir[i]) {
				return TRUE;
			}
		}
	} else if (IsBishop(moving_pce)) {

	}
	// TODO: Do the same for sliders and king
}

// Used for check detection / determining castling privileges
// Returns 1 if a given square is attacked

uint8_t SqAttacked(const int sq, const int side, const S_BOARD *pos) {

	int pce,t_sq,dir;
	
	ASSERT(SqOnBoard(sq));
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));
	
	// pawns
	if(side == WHITE) {
		if(pos->pieces[sq-11] == wP || pos->pieces[sq-9] == wP) {
			return TRUE;
		}
	} else {
		if(pos->pieces[sq+11] == bP || pos->pieces[sq+9] == bP) {
			return TRUE;
		}	
	}
	
	// knights
	for(int index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KnDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKn(pce) && PieceCol[pce]==side) {
			return TRUE;
		}
	}
	
	// rooks, queens
	for(int index = 0; index < 4; ++index) {		
		dir = RkDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsRQ(pce) && PieceCol[pce] == side) {
					return TRUE;
				}
				break;
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	
	// bishops, queens
	for(int index = 0; index < 4; ++index) {		
		dir = BiDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsBQ(pce) && PieceCol[pce] == side) {
					return TRUE;
				}
				break;
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	
	// kings
	for(int index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KiDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKi(pce) && PieceCol[pce]==side) {
			return TRUE;
		}
	}
	
	return FALSE;
	
}

// Sort of inverse of SqAttacked(). Considering the perspective of the piece rather than the square
uint8_t IsAttack(const int pce, const int sq, const S_BOARD *pos) {
	// sq = Landing square of the move

	int attacked_sq, dir;
	
	ASSERT(SqOnBoard(sq));
	ASSERT(CheckBoard(pos));

	uint8_t side = PieceCol[pce]; // moving side
	#define get_piece_col(Sq) PieceCol[ pos->pieces[Sq] ]
	
	// pawns
	if (pce == wP) {
		if((get_piece_col(sq-11) != side) || (get_piece_col(sq-9) != side)) {
			return TRUE;
		}
	}
	if (pce == bP) {
		if((get_piece_col(sq+11) != side) || (get_piece_col(sq+9) != side)) {
			return TRUE;
		}	
	}
	
	// knights
	if (PieceKnight[pce]) {
		for(int index = 0; index < 8; ++index) {	
			attacked_sq = sq + KnDir[index];
			if( (attacked_sq >= 0) && (attacked_sq < BRD_SQ_NUM) && (get_piece_col(attacked_sq) != side)) {
				return TRUE;
			}
		}
	}
	
	
	// rooks, queens
	if (PieceRookQueen[pce]) {
		for(int index = 0; index < 4; ++index) {		
			dir = RkDir[index];
			attacked_sq = sq + dir;
			ASSERT(SqIs120(attacked_sq));
			while((attacked_sq >= 0) && (attacked_sq < BRD_SQ_NUM)) {
				if(get_piece_col(attacked_sq) != side) {
					return TRUE;
				}
				attacked_sq += dir;
			}
		}
	}
	
	
	// bishops, queens
	if (PieceBishopQueen[pce]) {
		for(int index = 0; index < 4; ++index) {		
			dir = BiDir[index];
			attacked_sq = sq + dir;
			ASSERT(SqIs120(attacked_sq));
			while (attacked_sq >= 0 && attacked_sq < BRD_SQ_NUM) {
				if(get_piece_col(attacked_sq) != side) {
					return TRUE;
				}
				attacked_sq += dir;
			}
		}	
	}
	
	
	// kings cannot attack

	return FALSE;
	
}

// Outputs a weight of the attack based on what enemy pieces are attacking that square
uint16_t SqAttackedByWho(const int sq, const int side, const S_BOARD *pos) {

	int pce,t_sq,dir;
	
	ASSERT(SqOnBoard(sq));
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));
	
	// pawns
	if(side == WHITE) {
		if(pos->pieces[sq-11] == wP || pos->pieces[sq-9] == wP) {
			return 0;
		}
	} else {
		if(pos->pieces[sq+11] == bP || pos->pieces[sq+9] == bP) {
			return 0;
		}	
	}
	
	// knights
	for(int index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KnDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKn(pce) && PieceCol[pce]==side) {
			return 20;
		}
	}
	
	// rooks, queens
	for(int index = 0; index < 4; ++index) {		
		dir = RkDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsRQ(pce) && PieceCol[pce] == side) {
					if ((pce == wR) || (pce == bR)) {
						return 80;
					} else {
						return 40;
					}
				}
				break;
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	
	// bishops, queens
	for(int index = 0; index < 4; ++index) {		
		dir = BiDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsBQ(pce) && PieceCol[pce] == side) {
					if ((pce == wB) || (pce == bB)) {
						return 20;
					} else {
						return 80;
					}
				}
				break;
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	
	// kings
	for(int index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KiDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKi(pce) && PieceCol[pce]==side) {
			return 0;
		}
	}
	
	return 0;
	
}

/*
	Attack Table Generation
*/

U64 prepare_occupancy(U64 occupancy, uint8_t sq) {
	return occupancy ^ (1ULL << SQ64(sq));
}

U64 mask_pawn_attacks(uint8_t sq, uint8_t col) {
	
	U64 mask = 0ULL;

	if (col == WHITE) {
		if (SQ64(sq - 11) != 65) {
			mask |= (1ULL << SQ64(sq - 11));
		}
		if (SQ64(sq - 9) != 65) {
			mask |= (1ULL << SQ64(sq - 9));
		}
	} else {
		if (SQ64(sq + 11) != 65) {
			mask |= (1ULL << SQ64(sq + 11));
		}
		if (SQ64(sq + 9) != 65) {
			mask |= (1ULL << SQ64(sq + 9));
		}
	}

	return mask;
}

void init_attack_tables() {

}