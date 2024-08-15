// endgame.c

#include "defs.h"
#include <stdint.h>

// Test position: 8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41
// Determins if the position is a draw by material (with no pawns)
// Outputs: TRUE - It is a draw | FALSE - It is not a draw

inline uint8_t is_material_draw(const S_BOARD *pos, int net_material) {
	// Useful resource with a list of endgames to check if it's a draw
	// https://en.wikipedia.org/wiki/Pawnless_chess_endgame

	// General rule + Exception cases
	// Extremely rare cases that are wins but not included here:
	// Q + minor piece v R + 2B of the same colour is a win for the side with the queen
	// Q v 3B of the same colour is a win
	// R+N v 2B of the same colour is a win

	// Upper bound of a bishop's value in the endgame (slightly higher than actual value due to tapered eval)
	#define MAX_MINOR_PIECE 310

	// General case
	if (net_material <= MAX_MINOR_PIECE) {

		// A good general rule of thumb for determining material draws, with a few exceptions
		// One caveat with this approach is that is will convince itself everything is a draw and not really calculate
		
		uint8_t man_count = CountBits(pos->occupancy[BOTH]);

		uint8_t wQ_num = pos->pceNum[wQ];
		uint8_t wR_num = pos->pceNum[wR];
		uint8_t wB_num = pos->pceNum[wB];
		uint8_t wN_num = pos->pceNum[wN];

		uint8_t bQ_num = pos->pceNum[bQ];
		uint8_t bR_num = pos->pceNum[bR];
		uint8_t bB_num = pos->pceNum[bB];
		uint8_t bN_num = pos->pceNum[bN];

		// 7-man endgames
		if (man_count == 7) {
			if ( 
				( (wQ_num == 1) && (wN_num == 1) && (bR_num == 1) && (bB_num == 1) && (bN_num) ) ||
				( (bQ_num == 1) && (bN_num == 1) && (wR_num == 1) && (wB_num == 1) && (wN_num) )
			) {
				// 7-man: KQN v KRBN is winning
				return FALSE;
			} else if (
				( (wQ_num == 1) && ( (bN_num + bB_num) == 4 ) ) ||
				( (bQ_num == 1) && ( (wN_num + wB_num) == 4 ) )
			) {
				// 7-man: KQ v K + 4 minors is losing
				return FALSE;
			}
		}

		// 6-man endgames
		else if (man_count == 6) {
			if (
				( (wQ_num == 1) && (wB_num == 1) && (bR_num == 2) ) ||
				( (bQ_num == 1) && (bB_num == 1) && (wR_num == 2) ) 
			) {
				// 6-man: KQB v KRR is winning
				return FALSE;
			} else if ( (man_count == 6) && (wQ_num == 1) && (wR_num == 1) && (bQ_num == 1) && (bR_num == 1) ) {
				// 6-man: KQR v KQR
				// White is actually winning in 83% of these endgames due to first-move-advantage, despite equal material
				return FALSE;
			} else if (
				( (wR_num == 1) && (wB_num == 1) && (bN_num == 2) ) ||
				( (bR_num == 1) && (bB_num == 1) && (wN_num == 2) )
			) {
				// 6-man: KRB v KNN is a win
				return FALSE;
			} else if (
				( (wB_num == 1) && (wN_num == 2) && (bR_num == 1) ) ||
				( (bB_num == 1) && (bN_num == 2) && (wR_num == 1) )
			) {
				// 6-man: KBNN v KR is a draw
				return TRUE;
			} else if (
				( (wR_num == 1) && (wB_num == 1) && (bB_num == 1) && (bN_num) ) ||
				( (bR_num == 1) && (bB_num == 1) && (wB_num == 1) && (wN_num) )
			) {
				// 6-man: KRB v KBN is a win if the bishops are of different colours, but draw if the same

				// Check if the bishops are of different colours
				if (isOppColBishops(pos)) {
					return FALSE; // Not a draw
				} else {
					return TRUE; // A draw
				}
			}
		}

		// 5-man endgames
		else if (man_count == 5) {
			if (
				( (wQ_num == 1) && (bN_num == 2) ) ||
				( (bQ_num == 1) && (wN_num == 2) )
			) {
				// 5-man: KQ v KNN is a draw as side with knights can create a fortress
				return TRUE;
			} 
		}
		

		// 4-man endgames
		else if (man_count == 4) {
			if ( (man_count == 4) && ( (wN_num == 2) || (bN_num == 2) ) ) {
				// 4-man: KNN v K is a draw (and is considered "insufficient material" on websites and GUIs)
				return TRUE;
			}
		}

		else {
			// Special cases that check out:
			// KQ v KRBB
			// KBB v KN (barely exceeds margin - 312 vs 310) and related endgames
			return TRUE;
		//}
		
	} else {
		return FALSE;
	}

}