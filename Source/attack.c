// attack.c
// Major credits to Alexandria / BBC for most of the following code

#include <stdio.h>
#include <stdint.h>
#include "defs.h"
#include "attack.h"

/*************************
* Attack Masks (Leapers) *
*************************/

// Generate pawn attacks
U64 MaskPawnAttacks(int side, int square) {
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;
    // set piece on board
    bitboard |= (1ULL << SQ64(square));

    // white pawns
    if (!side) {
        // generate pawn attacks
        if ((bitboard >> 7) & NOT_A_FILE)
            attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & NOT_H_FILE)
            attacks |= (bitboard >> 9);
    }
    // black pawns
    else {
        // generate pawn attacks
        if ((bitboard << 7) & NOT_H_FILE)
            attacks |= (bitboard << 7);
        if ((bitboard << 9) & NOT_A_FILE)
            attacks |= (bitboard << 9);
    }
    // return attack map
    return attacks;
}

// generate king attacks
U64 MaskKingAttacks(int square) {
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;
    // set piece on board
    bitboard |= (1ULL << SQ64(square));

    // generate king attacks
    if (bitboard >> 8)
        attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & NOT_H_FILE)
        attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & NOT_A_FILE)
        attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & NOT_H_FILE)
        attacks |= (bitboard >> 1);
    if (bitboard << 8)
        attacks |= (bitboard << 8);
    if ((bitboard << 9) & NOT_A_FILE)
        attacks |= (bitboard << 9);
    if ((bitboard << 7) & NOT_H_FILE)
        attacks |= (bitboard << 7);
    if ((bitboard << 1) & NOT_H_FILE)
        attacks |= (bitboard << 1);

    // return attack map
    return attacks;
}

/*
// Generate king attacks
U64 MaskKingAttacks(int sq120) {
  	U64 output = 0ULL;
  
  	for (int index = 0; index < 8; ++index) {
    	int sq = sq120 + KiDir[index];
    	if (SQ64(sq) != 65) {
			output |= (1ULL << SQ64(sq120));
    	}
  	}

  	return output;
}
*/

// generate knight attacks
U64 MaskKnightAttacks(int square) {
    U64 attacks = 0ULL;

    // piece bitboard
    U64 bitboard = 0ULL;
    // set piece on board
    bitboard |= (1ULL << SQ64(square));
    // generate knight attacks
    if ((bitboard >> 17) & NOT_H_FILE)
        attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & NOT_A_FILE)
        attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & NOT_HG_FILE)
        attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & NOT_AB_FILE)
        attacks |= (bitboard >> 6);
    if ((bitboard << 17) & NOT_A_FILE)
        attacks |= (bitboard << 17);
    if ((bitboard << 15) & NOT_H_FILE)
        attacks |= (bitboard << 15);
    if ((bitboard << 10) & NOT_AB_FILE)
        attacks |= (bitboard << 10);
    if ((bitboard << 6) & NOT_HG_FILE)
        attacks |= (bitboard << 6);

    // return attack map
    return attacks;
}

/*
// Generate knight attacks
U64 MaskKnightAttacks(int sq120) {
  	U64 output = 0ULL;
  
  	for (int index = 0; index < 8; ++index) {
    	int sq = sq120 + KnDir[index];
    	if (SQ64(sq) != 65) {
      		output |= (1ULL << SQ64(sq120));
    	}
  	}

  	return output;
}
*/

/*************************
* Attack Masks (Sliders) *
*************************/

// mask bishop attacks
U64 MaskBishopAttacks(int square) {
    U64 attacks = 0ULL;

    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // mask relevant bishop occupancy bits
    for (int r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
        attacks |= (1ULL << (r * 8 + f));
    for (int r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
        attacks |= (1ULL << (r * 8 + f));
    for (int r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
        attacks |= (1ULL << (r * 8 + f));
    for (int r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
        attacks |= (1ULL << (r * 8 + f));

    // return attack map
    return attacks;
}

/*
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
*/

// mask rook attacks
U64 MaskRookAttacks(int square) {
    U64 attacks = 0ULL;

    // init target rank & files
    U64 tr = square / 8;
    U64 tf = square % 8;

    // mask relevant rook occupancy bits
    for (int r = tr + 1; r <= 6; r++)
        attacks |= (1ULL << (r * 8 + tf));
    for (int r = tr - 1; r >= 1; r--)
        attacks |= (1ULL << (r * 8 + tf));
    for (int f = tf + 1; f <= 6; f++)
        attacks |= (1ULL << (tr * 8 + f));
    for (int f = tf - 1; f >= 1; f--)
        attacks |= (1ULL << (tr * 8 + f));
    // return attack map
    return attacks;
}

/*
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
*/

// generate bishop attacks on the fly
U64 BishopAttacksOnTheFly(int square, U64 block) {
    U64 attacks = 0ULL;

    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // generate bishop atacks
    for (int r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }

    for (int r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }

    for (int r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }

    for (int r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block)
            break;
    }
    // return attack map
    return attacks;
}

/*
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
*/

// generate rook attacks on the fly
U64 RookAttacksOnTheFly(int square, U64 block) {
    U64 attacks = 0ULL;

    // init target rank & files
    int tr = square / 8;
    int tf = square % 8;

    // generate rook attacks
    for (int r = tr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block)
            break;
    }

    for (int r = tr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block)
            break;
    }

    for (int f = tf + 1; f <= 7; f++) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block)
            break;
    }

    for (int f = tf - 1; f >= 0; f--) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block)
            break;
    }
    // return attack map
    return attacks;
}

/*
// Generate rook attacks on the fly
// Will be used to generate lookup tables for search
U64 MaskRookAttacks(int square, U64 blockers) {
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
*/

// set occupancies
U64 SetOccupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;

    // loop over the range of bits within attack mask
    for (int count = 0; count < bits_in_mask; count++) {
        // get LSB index of attacks mask
        int square = PopBit(attack_mask);
        // make sure occupancy is on board
        if (index & (1 << count))
            // populate occupancy map
            occupancy |= (1ULL << square);
    }
    // return occupancy map
    return occupancy;
}

/*
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
*/

// get bishop attacks
U64 GetBishopAttacks(const int square, U64 occupancy) {
    // get bishop attacks assuming current board occupancy
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];

    // return bishop attacks
    return bishop_attacks[square][occupancy];
}

/*
// Get bishop attacks from table
U64 GetBishopAttacks(const int square, U64 occupancy) {

    // Apply the same calculation as before
    uint64_t magicIndex = (occupancy & bishop_masks[square]) * bishop_magic_numbers[square] >> 
			 			(64 - bishop_relevant_bits[square]);

    return bishop_attacks[square][magicIndex];

}
*/

// get rook attacks
U64 GetRookAttacks(const int square, U64 occupancy) {
    // get rook attacks assuming current board occupancy
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    // return rook attacks
    return rook_attacks[square][occupancy];
}

/*
// Get rook attacks from table
U64 GetRookAttacks(const int square, U64 occupancy) {

	// Apply the same calculation as before
	uint64_t magicIndex = (occupancy & rook_masks[square]) * rook_magic_numbers[square] >>
    					(64 - rook_relevant_bits[square]);

    return rook_attacks[square][magicIndex];
}

// Get queen attacks from table-ish
U64 GetQueenAttacks(const int square, U64 occupancy) {
    return GetBishopAttacks(square, occupancy) | GetRookAttacks(square, occupancy);
}
*/

// Test function
U64 GetOccupancy(const S_BOARD *pos) {
	U64 occupancy = 0ULL;
	for (int pce = wP; pce <= bK; ++pce) {
		for (int num = 0; num < 10; ++num) {
			if (num < pos->pceNum[pce]) {
				int sq = SQ64(pos->pList[pce][num]);
				occupancy |= (1ULL << sq);
			}
		}
	}
	return occupancy;
}

/****************
* Attack checks *
*****************/

// Used for check detection / determining castling privileges
// Returns 1 if a given square is attacked
uint8_t SqAttacked(const int sq, const int side, const S_BOARD *pos) {

	int pce, count, t_sq, dir;
	U64 mask = 1ULL << SQ64(sq);
	U64 occupancy = GetOccupancy(pos);
	
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
	
	// Bishops
	pce = (side == WHITE) ? wB : bB;
	count = pos->pceNum[pce];
    for (int num = 0; num < count; ++num) {
        int pceSq = SQ64(pos->pList[pce][num]);
        if (GetBishopAttacks(pceSq, occupancy) & mask) {
            return TRUE;
        }
    }

	// Rooks
	pce = (side == WHITE) ? wR : bR;
	count = pos->pceNum[pce];
    for (int num = 0; num < count; ++num) {
        int pceSq = SQ64(pos->pList[pce][num]);
        if (GetRookAttacks(pceSq, occupancy) & mask) {
            return TRUE;
        }
    }

	// Queens
	pce = (side == WHITE) ? wQ : bQ;
	count = pos->pceNum[pce];
    for (int num = 0; num < count; ++num) {
        int pceSq = SQ64(pos->pList[pce][num]);
        if ((GetBishopAttacks(pceSq, occupancy) | GetRookAttacks(pceSq, occupancy)) & mask) {
            return TRUE;
        }
    }

	/*
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
	*/
	
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