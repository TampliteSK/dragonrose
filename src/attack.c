// attack.c

#include <stdio.h>
#include "defs.h"
#include "attack.h"

U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];
U64 bishop_masks[64];
U64 rook_masks[64];

/*
	Attack checkers
*/

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
		return FALSE;
	}

	return FALSE;
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

// Special version of SqAttacked() which excludes kings
uint8_t SqAttackedS(const int sq, const int side, const S_BOARD *pos) {

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
				if (IsBQ(pce) && PieceCol[pce] == side) {
					return TRUE;
				}
				break;
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
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

U64 set_occupancy(int index, int bits_in_mask, uint64_t attack_mask) {

	U64 occupancy = 0ULL;

	for (int count = 0; count < bits_in_mask; count++) {
		int sq = PopBit(&attack_mask);

		// make sure occupancy is on board
		if (index & (1 << count)) {
			occupancy |= (1ULL << sq);
		}
	}

	// return occupancy map
	return occupancy;
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

// Relevant occupancy squares for bishops
U64 mask_bishop_attacks(uint8_t sq64) {

	U64 attacks = 0ULL;
	int r, f;

	// Target ranks & files
	int tr = RanksBrd[SQ120(sq64)];
	int tf = FilesBrd[SQ120(sq64)];

	for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
		attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
		attacks |= (1ULL << (r * 8 + f));
	for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
		attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
		attacks |= (1ULL << (r * 8 + f));

	return attacks;
}

// Relevant occupancy squares for rooks
U64 mask_rook_attacks(uint8_t sq64) {

	U64 attacks = 0ULL;
	int r, f;

	// Target rank & files
	int tr = RanksBrd[SQ120(sq64)];
	int tf = FilesBrd[SQ120(sq64)];

	for (r = tr + 1; r <= 6; r++)
		// Up
		attacks |= (1ULL << (r * 8 + tf));
	for (r = tr - 1; r >= 1; r--)
		// Down
		attacks |= (1ULL << (r * 8 + tf));
	for (f = tf + 1; f <= 6; f++)
		// Right
		attacks |= (1ULL << (tr * 8 + f));
	for (f = tf - 1; f >= 1; f--)
		// Left
		attacks |= (1ULL << (tr * 8 + f));

	return attacks;
}

// generate bishop attacks on the fly
U64 bishop_attacks_on_the_fly(uint8_t sq, uint64_t block) {

	U64 attacks = 0ULL;
	int r, f;

	// Target rank & files
	int tr = RanksBrd[sq];
	int tf = FilesBrd[sq];

	for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & block)
		break;
	}

	for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & block)
		break;
	}

	for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & block)
		break;
	}

	for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & block)
		break;
	}

	return attacks;
}

// generate rook attacks on the fly
U64 rook_attacks_on_the_fly(uint8_t sq, uint64_t block) {

	U64 attacks = 0ULL;
	int r, f;

	// Target rank & files
	int tr = RanksBrd[sq];
	int tf = FilesBrd[sq];

	for (r = tr + 1; r <= 7; r++) {
		attacks |= (1ULL << (r * 8 + tf));
		if ((1ULL << (r * 8 + tf)) & block)
		break;
	}

	for (r = tr - 1; r >= 0; r--) {
		attacks |= (1ULL << (r * 8 + tf));
		if ((1ULL << (r * 8 + tf)) & block)
		break;
	}

	for (f = tf + 1; f <= 7; f++) {
		attacks |= (1ULL << (tr * 8 + f));
		if ((1ULL << (tr * 8 + f)) & block)
		break;
	}

	for (f = tf - 1; f >= 0; f--) {
		attacks |= (1ULL << (tr * 8 + f));
		if ((1ULL << (tr * 8 + f)) & block)
		break;
	}

	return attacks;
}

U64 get_bishop_attacks(uint8_t sq, U64 occupancy) {
	
	uint8_t sq64 = SQ64(sq);
	occupancy ^= 1ULL << sq64;
	occupancy &= bishop_masks[sq64];
	occupancy *= bishop_magic_numbers[sq64];
	occupancy >>= 64 - bishop_relevant_bits[sq64];

	return bishop_attacks[sq64][occupancy];
}

U64 get_rook_attacks(uint8_t sq, U64 occupancy) {

	uint8_t sq64 = SQ64(sq);
	occupancy ^= 1ULL << sq64;
	occupancy &= rook_masks[sq64];
	occupancy *= rook_magic_numbers[sq64];
	occupancy >>= 64 - rook_relevant_bits[sq64];

	return rook_attacks[sq64][occupancy];
}

// get queen attacks
U64 get_queen_attacks(uint8_t sq, U64 occupancy) {

	U64 queen_attacks = 0ULL;

	uint8_t sq64 = SQ64(sq);
	occupancy ^= 1ULL << sq64;
	U64 bishop_occupancy = occupancy;
	U64 rook_occupancy = occupancy;

	bishop_occupancy &= bishop_masks[sq64];
	bishop_occupancy *= bishop_magic_numbers[sq64];
	bishop_occupancy >>= 64 - bishop_relevant_bits[sq64];

	rook_occupancy &= rook_masks[sq64];
	rook_occupancy *= rook_magic_numbers[sq64];
	rook_occupancy >>= 64 - rook_relevant_bits[sq64];

	queen_attacks = bishop_attacks[sq64][bishop_occupancy] | rook_attacks[sq64][rook_occupancy];
	return queen_attacks;
}

void init_slider_attacks() {

	for (int rank = RANK_1; rank <= RANK_8; ++rank) {
		for (int file = FILE_A; file <= FILE_H; ++file) {

			uint8_t sq = SQ64(FR2SQ(file, rank));

			bishop_masks[sq] = mask_bishop_attacks(sq);
			rook_masks[sq] = mask_rook_attacks(sq);

			uint8_t bishop_relevant_bits_count = CountBits(bishop_masks[sq]);
			uint8_t rook_relevant_bits_count = CountBits(rook_masks[sq]);

			int bishop_occupancy_indicies = (1 << bishop_relevant_bits_count);
			int rook_occupancy_indicies = (1 << rook_relevant_bits_count);

			// Initialise bishop attacks
			for (int index = 0; index < bishop_occupancy_indicies; index++) {

				U64 occupancy = set_occupancy(index, bishop_relevant_bits_count, bishop_masks[sq]);

				int magic_index = (occupancy * bishop_magic_numbers[sq]) >>
									(64 - bishop_relevant_bits[sq]);

				bishop_attacks[sq][magic_index] = bishop_attacks_on_the_fly(sq, occupancy);
			}

			// Initialise rook attacks
			for (int index = 0; index < rook_occupancy_indicies; index++) {

				U64 occupancy = set_occupancy(index, rook_relevant_bits_count, rook_masks[sq]);

				int magic_index = (occupancy * rook_magic_numbers[sq]) >>
									(64 - rook_relevant_bits[sq]);

				rook_attacks[sq][magic_index] =	rook_attacks_on_the_fly(sq, occupancy);
			}
		}
	}
	
}

void init_attack_tables() {
	init_slider_attacks();
}