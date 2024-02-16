// attack.c
// Major credits to Alexandria / BBC for most of the following code

#include <stdio.h>
#include <stdint.h>
#include "defs.h"
#include "attacks.h"

/*************************
* Attack Masks (Leapers) *
*************************/

// Generate pawn attacks
U64 MaskPawnAttacks(int side, int square) {
    U64 output = 0ULL;
    U64 pawnBoard = 0ULL;
    SETBIT(&pawnBoard, SQ64(square));

    if (!side) {
		// White pawns
        if ((pawnBoard >> 7) & NOT_A_FILE)
            output |= (pawnBoard >> 7);
        if ((pawnBoard >> 9) & NOT_H_FILE)
            output |= (pawnBoard >> 9);
    }
    else {
		// Black pawns
        if ((pawnBoard << 7) & NOT_H_FILE)
            output |= (pawnBoard << 7);
        if ((knightBoard << 9) & NOT_A_FILE)
            output |= (pawnBoard << 9);
    }

    return output;
}

// Generate king attacks
U64 MaskKingAttacks(int sq120) {
  	U64 output = 0ULL;
  
  	for (int index = 0; index < 8; ++index) {
    	int sq = sq120 + KiDir[index];
    	if (SQ64(sq) != 65) {
      		SETBIT(&output, SQ64(sq));
    	}
  	}

  	return output;
}

// Generate knight attacks
U64 MaskKnightAttacks(int sq120) {
  	U64 output = 0ULL;
  
  	for (int index = 0; index < 8; ++index) {
    	int sq = sq120 + KnDir[index];
    	if (SQ64(sq) != 65) {
      		SETBIT(&output, SQ64(sq));
    	}
  	}

  	return output;
}

/*************************
* Attack Masks (Sliders) *
*************************/

// Mask bishop occupany bits for a given square
U64 MaskBishopOccupancies(int sq120) {
    U64 output = 0ULL;

    // Target ranks and files
	int square = SQ64(sq120);
    int tr = square / 8;
    int tf = square % 8;

    for (int r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
        output |= (1ULL << (r * 8 + f));
    for (int r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
        output |= (1ULL << (r * 8 + f));
    for (int r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
        output |= (1ULL << (r * 8 + f));
    for (int r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
        output |= (1ULL << (r * 8 + f));

    return output;
}

// Mask rook occupancy bits for a given square
U64 MaskRookOccupancies(int square) {
    U64 output = 0ULL;

    // Target ranks and files
    int tr = square / 8;
    int tf = square % 8;

    for (int r = tr + 1; r <= 6; r++)
        output |= (1ULL << (r * 8 + tf));
    for (int r = tr - 1; r >= 1; r--)
        output |= (1ULL << (r * 8 + tf));
    for (int f = tf + 1; f <= 6; f++)
        output |= (1ULL << (tr * 8 + f));
    for (int f = tf - 1; f >= 1; f--)
        output |= (1ULL << (tr * 8 + f));

    return output;
}

// Generate bishop attacks on the fly
// Will be used to generate lookup tables for search
U64 MaskBishopAttacks(int square, U64 blockers) {
	// Blockers: Bitboard for pieces that block the bishop
	// Block = 0ULL on an open board
    U64 output = 0ULL;

    // Target rank & files
    int tr = square / 8;
    int tf = square % 8;

    for (int r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        output |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers)
            break;
    }

    for (int r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        output |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers)
            break;
    }

    for (int r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        output |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers)
            break;
    }

    for (int r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        output |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & blockers)
            break;
    }

    return output;
}

// Generate rook attacks on the fly
// Will be used to generate lookup tables for search
U64 MaskRookAttacks(int square, Bitboard blockers) {
    U64 output = 0ULL;

    // Target rank & files
    int tr = square / 8;
    int tf = square % 8;


    for (int r = tr + 1; r <= 7; r++) {
        output |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & blockers)
            break;
    }

    for (int r = tr - 1; r >= 0; r--) {
        output |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & blockers)
            break;
    }

    for (int f = tf + 1; f <= 7; f++) {
        output |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & blockers)
            break;
    }

    for (int f = tf - 1; f >= 0; f--) {
        output |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & blockers)
            break;
    }

    return output;
}

// Sets up blockers map
U64 SetBlockers(int index, int mask_bit_count, U64 occupancy_mask) {
    U64 output = 0ULL;

    for (int count = 0; count < mask_bit_count; count++) {
        int square = PopBit(&occupancy_mask);
        // Make sure occupancy is on board
        if (index & (1 << count))
            output |= (1ULL << square); // Populate occupancy map
    }

    return output;
}

// Get bishop attacks from table
U64 GetBishopAttacks(const int square, U64 occupancy) {

    // Apply the same calculation as before
    uint64_t magicIndex = (occupancy & bishop_masks[square]) * bishop_magic_numbers[square] >> 
			 			(64 - bishop_relevant_bits[square]);

    return bishop_attacks[square][magicIndex];

}

// Get rook attacks from table
U64 GetRookAttacks(const int square, Bitboard occupancy) {

	// Apply the same calculation as before
	uint64_t magicIndex = (occupancy & rook_masks[square]) * rook_magic_numbers[square] >>
    					(64 - rook_relevant_bits[square]);

    return rook_attacks[square][magicIndex];
}

// Get queen attacks from table-ish
U64 GetQueenAttacks(const int square, Bitboard occupancy) {
    return GetBishopAttacks(square, occupancy) | GetRookAttacks(square, occupancy);
}

/****************
* Attack checks *
*****************/

// Used for check detection / determining castling privileges
// Returns 1 if a given square is attacked
uint8_t SqAttacked(const int sq, const int side, const S_BOARD *pos) {

	int pce,t_sq,dir;
	
	ASSERT(SqOnBoard(sq));
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));
	
	// Pawns
	if(side == WHITE) {
		if(pos->pieces[sq-11] == wP || pos->pieces[sq-9] == wP) {
			return TRUE;
		}
	} else {
		if(pos->pieces[sq+11] == bP || pos->pieces[sq+9] == bP) {
			return TRUE;
		}	
	}
	
	// Knights
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

// Used for king safety evaluation
// Sums attack units for a single square
inline uint16_t AtkUnitsOnSq(const S_BOARD *pos, uint8_t sq, const uint8_t side) {

	uint16_t attackUnits = 0;

	// === Attack Unit Weights ===
	// Pawns: 1
	// Knights, bishops: 2
	// Rooks: 3
	// Queens: 5

	/*
	int pce,t_sq,dir;
	
	// ASSERT(SqOnBoard(sq));
	ASSERT(SideValid(side));
	ASSERT(CheckBoard(pos));
	
	// Pawns
	if(side == WHITE) {
		attackUnits = (pos->pieces[sq-11] == wP) + (pos->pieces[sq-9] == wP);
	} else {
		attackUnits = (pos->pieces[sq+11] == bP) + (pos->pieces[sq+9] == bP);
	}
	
	// Knights
	for(int index = 0; index < 8; ++index) {		
		pce = pos->pieces[sq + KnDir[index]];
		ASSERT(PceValidEmptyOffbrd(pce));
		if(pce != OFFBOARD && IsKn(pce) && PieceCol[pce]==side) {
			attackUnits += 2;
		}
	}
	
	// Rooks, Queens
	for(int index = 0; index < 4; ++index) {		
		dir = RkDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsRQ(pce) && PieceCol[pce] == side) {
					attackUnits += (pce == wR || pce == bR) ? 3 : 0;
					attackUnits += (pce == wQ || pce == bQ) ? 5 : 0;
				}
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	
	// Bishops, Queens
	for(int index = 0; index < 4; ++index) {		
		dir = BiDir[index];
		t_sq = sq + dir;
		ASSERT(SqIs120(t_sq));
		pce = pos->pieces[t_sq];
		ASSERT(PceValidEmptyOffbrd(pce));
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsBQ(pce) && PieceCol[pce] == side) {
					attackUnits += (pce == wB || pce == bB) ? 2 : 0;
					attackUnits += (pce == wQ || pce == bQ) ? 5 : 0;
				}
			}
			t_sq += dir;
			ASSERT(SqIs120(t_sq));
			pce = pos->pieces[t_sq];
		}
	}
	*/
	
	return attackUnits;
	
}