// init.c

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "attack.h"

// Randomisation is separated as rand() is only 4 bytes
#define RAND_64 	((U64)rand() | \
					(U64)rand() << 15 | \
					(U64)rand() << 30 | \
					(U64)rand() << 45 | \
					((U64)rand() & 0xf) << 60 )

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

U64 FileBBMask[8];
U64 RankBBMask[8];

U64 BlackPassedMask[64];
U64 WhitePassedMask[64];
U64 IsolatedMask[64];

S_OPTIONS EngineOptions[1];

// Attack tables
U64 pawn_attacks[2][64]; // [side][square]
U64 knight_attacks[64]; // [square]
U64 king_attacks[64]; // [square]
U64 bishop_masks[64]; // [square]
U64 rook_masks[64]; // [square]
U64 bishop_attacks[64][512]; // [square][occupancies]
U64 rook_attacks[64][4096]; // [square][occupancies]

/*********************
* Board Manipulation *
*********************/

void InitFilesRanksBrd() {

	for(int index = 0; index < BRD_SQ_NUM; ++index) {
		FilesBrd[index] = OFFBOARD;
		RanksBrd[index] = OFFBOARD;
	}

	for(int rank = RANK_1; rank <= RANK_8; ++rank) {
		for(int file = FILE_A; file <= FILE_H; ++file) {
			int sq = FR2SQ(file,rank);
			FilesBrd[sq] = file;
			RanksBrd[sq] = rank;
		}
	}

}

void InitSq120To64() {

	for(int index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65; // default value if off board
	}

	for(int index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120; // default value if off board
	}
	
	int sq64 = 0;
  	for(int rank = RANK_1; rank <= RANK_8; ++rank) {
    	for(int file = FILE_A; file <= FILE_H; ++file) {
      		int sq = FR2SQ(file,rank);
      		Sq64ToSq120[sq64] = sq;
      		Sq120ToSq64[sq] = sq64;
      		sq64++;
    	}
  	}
	
}

void InitBitMasks() {
	int index = 0;

	for(index = 0; index < 64; index++) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for(index = 0; index < 64; index++) {
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index];
	}
}

void InitEvalMasks() {

	int sq, tsq, r, f;

	for(sq = 0; sq < 8; ++sq) {
        FileBBMask[sq] = 0ULL;
		RankBBMask[sq] = 0ULL;
	}

	for(r = RANK_8; r >= RANK_1; r--) {
        for (f = FILE_A; f <= FILE_H; f++) {
            sq = r * 8 + f;
            FileBBMask[f] |= (1ULL << sq);
            RankBBMask[r] |= (1ULL << sq);
        }
	}

	for(sq = 0; sq < 64; ++sq) {
		IsolatedMask[sq] = 0ULL;
		WhitePassedMask[sq] = 0ULL;
		BlackPassedMask[sq] = 0ULL;
    }

	for(sq = 0; sq < 64; ++sq) {
		tsq = sq + 8;

        while(tsq < 64) {
            WhitePassedMask[sq] |= (1ULL << tsq);
            tsq += 8;
        }

        tsq = sq - 8;
        while(tsq >= 0) {
            BlackPassedMask[sq] |= (1ULL << tsq);
            tsq -= 8;
        }

        if(FilesBrd[SQ120(sq)] > FILE_A) {
            IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1];

            tsq = sq + 7;
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 9;
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }

        if(FilesBrd[SQ120(sq)] < FILE_H) {
            IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];

            tsq = sq + 9;
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 7;
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
	}
}

/***********
* Hashkeys *
***********/

void InitHashKeys() {

	for(int index = 0; index < 13; ++index) {
		for(int index2 = 0; index2 < 120; ++index2) {
			PieceKeys[index][index2] = RAND_64;
		}
	}
	SideKey = RAND_64;
	for(int index = 0; index < 16; ++index) {
		CastleKeys[index] = RAND_64;
	}

}

/****************
* Attack tables *
****************/

// init attack tables for all the piece types, indexable by square
void InitAttackTables() {
    for (int square = 0; square < BRD_SQ_NUM; square++) {

        // init pawn attacks
        pawn_attacks[WHITE][square] = MaskPawnAttacks(WHITE, square);
        pawn_attacks[BLACK][square] = MaskPawnAttacks(BLACK, square);

        // init knight attacks
        knight_attacks[square] = MaskKnightAttacks(square);

        // init king attacks
        king_attacks[square] = MaskKingAttacks(square);

        // init bishop attacks
        bishop_masks[square] = MaskBishopAttacks(square);

        // init current mask
        U64 bishop_mask = bishop_masks[square];

        // get the relevant occupancy bit count
        int relevant_bits_count = CountBits(bishop_mask);

        // init occupancy indices
        int occupancy_indices = (1 << relevant_bits_count);

        // loop over occupancy indices
        for (int index = 0; index < occupancy_indices; index++) {
                // init current occupancy variation
                U64 occupancy =
                    SetOccupancy(index, relevant_bits_count, bishop_mask);

                // init magic index
                uint64_t magic_index = (occupancy * bishop_magic_numbers[square]) >>
                    (64 - bishop_relevant_bits[square]);

                // init bishop attacks
                bishop_attacks[square][magic_index] =
                    BishopAttacksOnTheFly(square, occupancy);
        }

        // init rook attacks
        rook_masks[square] = MaskRookAttacks(square);

        // init current mask
        U64 rook_mask = rook_masks[square];

        // init relevant occupancy bit count
        relevant_bits_count = CountBits(rook_mask);

        // init occupancy indices
        occupancy_indices = (1 << relevant_bits_count);

        // loop over occupancy indices
        for (int index = 0; index < occupancy_indices; index++) {
            // init current occupancy variation
            U64 occupancy =
                    SetOccupancy(index, relevant_bits_count, rook_mask);

            // init magic index
            uint64_t magic_index = (occupancy * rook_magic_numbers[square]) >>
                                                                            (64 - rook_relevant_bits[square]);

            // init rook attacks
            rook_attacks[square][magic_index] =
                    RookAttacksOnTheFly(square, occupancy);
        }
    }
}

/*
// Initialises leaper attacks
void InitLeapersAttacks() {

    for (int square = 0; square < 64; square++) {
        // Pawn attacks
        pawn_attacks[WHITE][square] = MaskPawnAttacks(WHITE, square);
        pawn_attacks[BLACK][square] = MaskPawnAttacks(BLACK, square);

        // Knight attacks
        knight_attacks[square] = MaskKnightAttacks(square);

        // King attacks
        king_attacks[square] = MaskKingAttacks(square);
    }

}

// Initialises slider attacks
void InitSlidersAttacks() {

    // Bishop attacks
    for (int square = 0; square < 64; square++) {
        bishop_masks[square] = MaskBishopOccupancies(square);
        U64 bishop_occupancy_mask = bishop_masks[square];
        int relevant_bits_count = CountBits(bishop_occupancy_mask); // Init relevant occupancy bit count
        int occupancy_indicies = (1 << relevant_bits_count); // Init occupancy indices

        // Loop over occupancy indicies
        for (int index = 0; index < occupancy_indicies; index++) {
                U64 blockers = SetBlockers(index, relevant_bits_count, bishop_occupancy_mask); // Get the map for blockers (occupancies & mask)
                // Init magic index     (blockers * magic)                        >> (64 - index_bits)
                uint64_t magic_index = (blockers * bishop_magic_numbers[square]) >>
                    (64 - bishop_relevant_bits[square]);
                bishop_attacks[square][magic_index] = MaskBishopAttacks(square, blockers); // Init bishop attacks (finally....)
        }
    }

    // Rook attacks
    for (int square = 0; square < 64; square++) {
        rook_masks[square] = MaskRookOccupancies(square);
        U64 rook_occupancy_mask = rook_masks[square];
        int relevant_bits_count = CountBits(rook_occupancy_mask); // Init relevant occupancy bit count
        int occupancy_indicies = (1 << relevant_bits_count); // Init occupancy indices

        // Loop over occupancy indicies
        for (int index = 0; index < occupancy_indicies; index++) {
            U64 blockers = SetBlockers(index, relevant_bits_count, rook_occupancy_mask); // Get the map for blockers (occupancies & mask)
			// Init magic index     (blockers * magic)                        >> (64 - index_bits)
            uint64_t magic_index = (blockers * rook_magic_numbers[square]) >>
                (64 - rook_relevant_bits[square]);
            rook_attacks[square][magic_index] = MaskRookAttacks(square, blockers); // Init rook attacks (finally....)
        }
    }

}
*/

void AllInit() {
	InitFilesRanksBrd();
	InitSq120To64();
	InitBitMasks();
	InitEvalMasks();
	InitHashKeys();

	// Attack tables
    InitAttackTables();
	// InitLeapersAttacks();
	// InitSlidersAttacks();

	// From other sources
	InitMvvLva();
	InitPolyBook();
}