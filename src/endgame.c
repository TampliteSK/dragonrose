// endgame.c

#include "defs.h"

// Test position: 8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41
// Determins if the position is a draw by material (with no pawns)
uint8_t is_material_draw(const S_BOARD *pos, int net_material) {
	// Useful resource with a list of endgames to check if it's a draw
	// https://en.wikipedia.org/wiki/Pawnless_chess_endgame
	
	// Upper bound of a bishop's value in the endgame (slightly higher than actual value due to tapered eval)
	#define MAX_MINOR_PIECE 310

	// General rule + Exception cases
	// Extremely rare cases that are wins but not included here:
	// Q + minor piece v R + 2B of the same colour is a win for the side with the queen
	// Q v 3B of the same colour is a win
	// R+N v 2B of the same colour is a win
	if ( (CountBits(pos->occupancy[BOTH]) == 7) && 
		( 
			( (pos->pceNum[wQ] == 1) && (pos->pceNum[wN] == 1) && (pos->pceNum[bR] == 1) && (pos->pceNum[bB] == 1) && (pos->pceNum[bN]) ) ||
			( (pos->pceNum[bQ] == 1) && (pos->pceNum[bN] == 1) && (pos->pceNum[wR] == 1) && (pos->pceNum[wB] == 1) && (pos->pceNum[wN]) )
		) ) {
		// 7-man: KQN v KRBN is winning
		return FALSE;

	} else if ( (CountBits(pos->occupancy[BOTH]) == 7) && 
		( 
			( (pos->pceNum[wQ] == 1) && ( (pos->pceNum[bN] + pos->pceNum[bB]) == 4 ) ) ||
			( (pos->pceNum[bQ] == 1) && ( (pos->pceNum[wN] + pos->pceNum[wB]) == 4 ) )
		) ) {
		// 7-man: KQ v K + 4 minors is losing
		return FALSE;

	} else if ( (CountBits(pos->occupancy[BOTH]) == 6) && 
		( 
			( (pos->pceNum[wQ] == 1) && (pos->pceNum[wB] == 1) && (pos->pceNum[bR] == 2) ) ||
			( (pos->pceNum[bQ] == 1) && (pos->pceNum[bB] == 1) && (pos->pceNum[wR] == 2) ) 
		) ) {
		// 6-man: KQB v KRR is winning
		return FALSE;

	} else if ( (CountBits(pos->occupancy[BOTH]) == 6) && (pos->pceNum[wQ] == 1) && (pos->pceNum[wR] == 1) && (pos->pceNum[bQ] == 1) && (pos->pceNum[bR] == 1)) {
		// 6-man: KQR v KQR
		// White is actually winning in 83% of these endgames due to first-move-advantage, despite equal material
		return FALSE;
	
	} else if ( (CountBits(pos->occupancy[BOTH]) == 6) && 
		( 
			( (pos->pceNum[wR] == 1) && (pos->pceNum[wB] == 1) && (pos->pceNum[bN] == 2) ) ||
			( (pos->pceNum[bR] == 1) && (pos->pceNum[bB] == 1) && (pos->pceNum[wN] == 2) )
		) ) {
		// 6-man: KRB v KNN is a win
		return FALSE;
	
	} else if ( (CountBits(pos->occupancy[BOTH]) == 6) && 
		( 
			( (pos->pceNum[wR] == 1) && (pos->pceNum[wB] == 1) && (pos->pceNum[bB] == 1) && (pos->pceNum[bN]) ) ||
			( (pos->pceNum[bR] == 1) && (pos->pceNum[bB] == 1) && (pos->pceNum[wB] == 1) && (pos->pceNum[wN]) )
		) ) {
		// 6-man: KRB v KBN is a win if the bishops are of different colours, but draw if the same

		// Check if the bishops are of different colours
		if (isOppColBishops(pos)) {
			return FALSE; // Not a draw
		} else {
			return TRUE; // A draw
		}
		
	} else if ( (CountBits(pos->occupancy[BOTH]) == 6) && 
		( 
			( (pos->pceNum[wB] == 1) && (pos->pceNum[wN] == 2) && (pos->pceNum[bR] == 1) ) ||
			( (pos->pceNum[bB] == 1) && (pos->pceNum[bN] == 2) && (pos->pceNum[wR] == 1) )
		) ) {
		// 6-man: KBNN v KR is a draw
		return TRUE;

	} else if ( (CountBits(pos->occupancy[BOTH]) == 5) && 
		( 
			( (pos->pceNum[wQ] == 1) && (pos->pceNum[bN] == 2) ) ||
			( (pos->pceNum[bQ] == 1) && (pos->pceNum[wN] == 2) )
		) ) {
		// 5-man: KQ v KNN is a draw as side with knights can create a fortress
		return TRUE;

	} else if ( (CountBits(pos->occupancy[BOTH]) == 4) && ( (pos->pceNum[wN] == 2) || (pos->pceNum[bN] == 2) ) ) {
		// 4-man: KNN v K is a draw (and is considered "insufficient material" on websites and GUIs)
		return TRUE;

	} else if (net_material <= MAX_MINOR_PIECE) {
		// A good general rule of thumb for determining material draws, with a few exceptions
		// One caveat with this approach is that is will convince itself everything is a draw and not really calculate
		
		// Special cases that check out:
		// KQ v KRBB
		// KBB v KN (barely exceeds margin - 312 vs 310) and related endgames
		return TRUE;
	}

  	return FALSE;
}