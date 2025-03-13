// attackgen.c

#include <stdio.h>
#include <string.h> // memset()
#include "defs.h"
#include "attackgen.h"

U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];
U64 bishop_masks[64];
U64 rook_masks[64];

/*
	Attack generation
*/

U64 set_occupancy(int index, int bits_in_mask, uint64_t attack_mask) {

	U64 occupancy = 0ULL;

	for (int count = 0; count < bits_in_mask; count++) {
		int sq = PopBit(&attack_mask);

		// Make sure occupancy is on board
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
U64 mask_bishop_attacks(uint8_t bbc_sq) {

	U64 attacks = 0ULL;
	int r, f;

	// Target ranks & files
	int tr = bbc_sq / 8;
	int tf = bbc_sq % 8;

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
U64 mask_rook_attacks(uint8_t bbc_sq) {

	U64 attacks = 0ULL;
	int r, f;

	// Target rank & files
	int tr = bbc_sq / 8;
	int tf = bbc_sq % 8;

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

U64 bishop_attacks_on_the_fly(uint8_t bbc_sq, uint64_t block) {

	U64 attacks = 0ULL;
	int r, f;

	// Target rank & files
	int tr = bbc_sq / 8;
	int tf = bbc_sq % 8;

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

U64 rook_attacks_on_the_fly(uint8_t bbc_sq, uint64_t block) {

	U64 attacks = 0ULL;
	int r, f;

	// Target rank & files
	int tr = bbc_sq / 8;
	int tf = bbc_sq % 8;

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

U64 get_bishop_attacks(uint8_t sq64, U64 occupancy) {
	
	uint8_t bbc_sq = Mirror64[sq64]; // Convert from VICE SQ64 indexing to BBC indexing 
	U64 blockers = flip_bitboard(occupancy) & bishop_masks[sq64]; // Mask out the (flipped) occupancy map to get relevant blockers
	PrintBitBoard(flip_bitboard(bishop_masks[bbc_sq]));
	PrintBitBoard(flip_bitboard(blockers));
	int magic_index = (blockers * bishop_magic_numbers[bbc_sq]) >> (64 - bishop_relevant_bits[bbc_sq]); // Compress blockers map into a hash table index for lookup

	return flip_bitboard(bishop_attacks[bbc_sq][magic_index]); // Flip it back to VICE SQ64 indexing
}

U64 get_rook_attacks(uint8_t sq64, U64 occupancy) {

	uint8_t bbc_sq = Mirror64[sq64];       
	U64 blockers = flip_bitboard(occupancy) & rook_masks[bbc_sq];
	int magic_index = (blockers * rook_magic_numbers[bbc_sq]) >> (64 - rook_relevant_bits[bbc_sq]);

	return flip_bitboard(rook_attacks[bbc_sq][magic_index]);
}

U64 get_queen_attacks(uint8_t sq64, U64 occupancy) {
	return get_rook_attacks(sq64, occupancy) | get_bishop_attacks(sq64, occupancy);
}

/*
	Magic numbers generation
*/

U64 find_magics(uint8_t sq, int relevant_bits, uint8_t is_bishop) {

	U64 blockers[4096];
	U64 attacks[4096];
	U64 used_attacks[4096];

	U64 attack_mask = is_bishop ? mask_bishop_attacks(sq) : mask_rook_attacks(sq);
	int occupancy_indices = 1 << relevant_bits;

	for (int i = 0; i < occupancy_indices; ++i) {
		blockers[i] = set_occupancy(i, relevant_bits, attack_mask);
		attacks[i] = is_bishop ? bishop_attacks_on_the_fly(sq, blockers[i]) : rook_attacks_on_the_fly(sq, blockers[i]);
	}

	for (int trial = 0; trial < 100000000; ++trial) {
		U64 magic_candidate = generate_magic_number();

		if (CountBits( (attack_mask * magic_candidate) & 0xFF00000000000000 ) < 6) {
			continue; // Skip inappropriate magic numbers
		}

		memset(used_attacks, 0ULL, sizeof(used_attacks));
		
		int i;
		uint8_t fail;

		for (i = 0, fail = FALSE; !fail && i < occupancy_indices; ++i) {
			int magic_index = (int)( (blockers[i] * magic_candidate) >> (64 - relevant_bits) );
			
			// Check if this index is occupied
			if (used_attacks[magic_index] == 0ULL) {
				used_attacks[magic_index] = attacks[i];
			} else if (used_attacks[magic_index] != attacks[i]) {
				// Occupied, check if there's a collision
				fail = TRUE;
			}
		}

		// Check if the magic number works
		if (!fail) {
			return magic_candidate;
		}
	}

	printf("Magic number fails!\n");
	return 0ULL;
}

void init_magic_numbers() {

	int count = 0;

	for (int rank = RANK_1; rank <= RANK_8; ++rank) {
		for (int file = FILE_A; file <= FILE_H; ++file) {
			++count;
			uint8_t sq = SQ64(FR2SQ(file, rank));
			printf("0x%llxULL, ", find_magics(sq, bishop_relevant_bits[sq], TRUE));
			if (count % 4) {
				printf("\n");
			}
		}
	}
	
	printf("\n\n");
	count = 0;

	for (int rank = RANK_1; rank <= RANK_8; ++rank) {
		for (int file = FILE_A; file <= FILE_H; ++file) {
			++count;
			uint8_t sq = SQ64(FR2SQ(file, rank));
			printf("0x%llxULL, ", find_magics(sq, rook_relevant_bits[sq], FALSE));
			if (count % 4) {
				printf("\n");
			}
		}
	}

	printf("\n");
}

/*
	Attack tables initialisation
*/

void init_slider_attacks() {

	for (int rank = RANK_1; rank <= RANK_8; ++rank) {
		for (int file = FILE_A; file <= FILE_H; ++file) {

			uint8_t sq = SQ64(FR2SQ(file, rank)); // The index doesn't check out but it works somehow

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