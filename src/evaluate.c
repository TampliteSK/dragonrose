// evaluate.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#include "defs.h"
#include "evaluate.h"

// Test positions for evaluation
// Pos1 (startpos) [+0.2, e4/d4/Nf3/c4]
// Pos2 (traxler) [+0.4, Nxe4]: r1bqk2r/pppp1Npp/2n2n2/4p3/2B1P3/8/PPPP1KPP/RNBQ3R b kq - 0 6
// Pos3 (fried liver) [0.0 Na5]: r1bqkb1r/ppp2ppp/2n2n2/3Pp1N1/2B5/8/PPPP1PPP/RNBQK2R b KQkq - 0 5
// Pos4 (king safety test): r1bqkbnr/pp1p1p1p/2n1p3/2p3p1/4P3/3B1N2/PPPP1PPP/RNBQ1RK1 w kq - 0 5
// Pos5 (king safety test) [-0.47, Qd2]: r2Nk2r/ppp5/2np1n2/2b1p3/2B1P1b1/3P2p1/PPP2PPP/RN1Q1RK1 w kq - 1 12

// Pos5 (h6 blunders): 8/8/6R1/5ppP/5k2/3r1P2/8/6K1 w - - 9 57
// Pos6 (Qc6/Qe7/Be7) [+0.6]: rnb1kb1r/pppp1ppp/5n2/8/4q3/5N2/PPPPBPPP/RNBQK2R b KQkq - 1 5

// Temporary hack. Scales down the eval in case it's too high (assuming the code works fine)
// #define squishFactor 0.35

/********************************
***** Evaluation components *****
********************************/

// Returns 1 if it's a light square
/*
uint8_t isLightSq(uint8_t sq) {
	return !( (sq % 2) ^ ( ( sq / 10 ) % 2) );
}
*/

// Gives bonuses if bishop and pawns are of different colour complexes
/*
uint8_t bishopPawnComplex(const S_BOARD *pos, uint8_t bishopSq, uint8_t col) {

	uint8_t subscore = 0;
	uint8_t bonus = 5;
	uint8_t pce = (col == WHITE) ? wP : bP;

	for (int pawn = 0; pawn < 8; ++pawn) {
		uint8_t pawnSq = pos->pList[pce][pawn];
		if ( isLightSq(bishopSq) != isLightSq(pawnSq) ) {
			subscore += bonus;
		}
	}
	
	return subscore;

}
*/

// Applying gamePhase at startpos
#define openingPhase 64

// Calculates the weight of tapered eval.
inline double evalWeight(const S_BOARD *pos) {
	// PesTO has its own tapered eval but it's 17 +/-22 elo worse than Caissa's
	// Scaling by material is strictly worse, and is about 225-275 elo weaker.

	ASSERT(CheckBoard(pos));

	// Caissa tapered eval (0.11e)
	uint8_t gamePhase = 3 * ( pos->pceNum[wN] + pos->pceNum[bN] + pos->pceNum[wB] + pos->pceNum[bB] );
	gamePhase += 5 * ( pos->pceNum[wR] + pos->pceNum[bR] );
	gamePhase += 10 * ( pos->pceNum[wQ] + pos->pceNum[bQ] );
	gamePhase = fmin(gamePhase, openingPhase); // capped at opening phase

	// not sure if linear is the best, but it is the norm
	// sqrt() is strictly worse
	return gamePhase / (double)openingPhase;

}

/******************
*** King Safety ***
******************/

static inline int16_t punishOpenFiles(const S_BOARD *pos, uint8_t kingSq) {
	uint8_t kingFile = FilesBrd[kingSq];
	const int16_t KingOpenFile[3] = { -100, -120, -100 };
	int16_t openLines = 0;

	//    For edge cases
	if (kingFile == FILE_A || kingFile == FILE_H) {
		if (!(pos->pawns[BOTH] & FileBBMask[kingFile])) {
			openLines += KingOpenFile[1];
		}
		if (kingFile == FILE_A) {
			if (!(pos->pawns[BOTH] & FileBBMask[FILE_B])) {
				openLines += KingOpenFile[0];
			} 
		} else {
			if (!(pos->pawns[BOTH] & FileBBMask[FILE_G])) {
				openLines += KingOpenFile[0];
			} 
		}
	} else {
		// For general cases
		for (int file = kingFile - 1; file <= kingFile + 1; ++file) {
			if (!(pos->pawns[BOTH] & FileBBMask[file])) {
				openLines += KingOpenFile[file - kingFile + 1];
			} 
		}
	}

	return openLines;

}

static U64 generate_king_zone(uint8_t kingSq, uint8_t col) {
	U64 king_zone = 0ULL;
	uint8_t kingFile = FilesBrd[kingSq];
	uint8_t kingRank = RanksBrd[kingSq];

	if (col == WHITE) {
		for (int rank = kingRank + 1; rank <= kingRank + 2; ++rank) {
			for (int file = kingFile - 1; file <= kingFile + 1; ++file) {
				king_zone |= 1ULL << SQ64(FR2SQ(file, rank));
			}
		}
	} else {
		for (int rank = kingRank - 1; rank <= kingRank - 3; --rank) {
			for (int file = kingFile - 1; file <= kingFile + 1; ++file) {
				king_zone |= 1ULL << SQ64(FR2SQ(file, rank));
			}
		}
	}

	return king_zone;
	
}

static inline int16_t pawnShield(const S_BOARD *pos, uint8_t kingSq, uint8_t col) {

	/*
	--------
	--------
	--------
	--------
	--------
	-----xxx
	-----xxx
	------K-
	*/

	// uint8_t kingFile = FilesBrd[kingSq];
	// uint8_t kingRank = RanksBrd[kingSq];
	const int8_t PawnShield[4] = { 0, -8, -15, -50 }; // startpos, moved 1 sq, 2 sq, too far away / dead. [3] shouldn't be too high as kingOpenFile exists
	U64 castled_king = 0ULL;
	int16_t shield = 0;

	if (col == WHITE) {
		// Pawn shield only applies to castled king
		castled_king = RankBBMask[RANK_1];
		castled_king ^= (1ULL << SQ64(D1)) | (1ULL << SQ64(E1)) | (1ULL << SQ64(F1)); // Not on d1 e1 or f1
		if (castled_king & (1ULL << SQ64(kingSq))) {
			U64 king_zone = generate_king_zone(kingSq, WHITE);
			U64 pawns_in_zone = pos->pawns[WHITE] & king_zone;

			uint8_t bits = CountBits(pawns_in_zone);
			if (bits < 3) {
				// At least one pawn is too far advanced or dead
				shield += PawnShield[3] * (3 - bits);
			}

			while (pawns_in_zone) {
				uint8_t pawn_sq = PopBit(&pawns_in_zone);
				uint8_t rank = pawn_sq / 8;
				shield += PawnShield[rank - 2];
			}
		}
	} 
	else {
		castled_king = RankBBMask[RANK_8];
		castled_king ^= (1ULL << SQ64(D8)) | (1ULL << SQ64(E8)) | (1ULL << SQ64(F8)); // Not on d8 e8 or f8
		if (castled_king & (1ULL << SQ64(kingSq))) {
			U64 king_zone = generate_king_zone(kingSq, BLACK);
			U64 pawns_in_zone = pos->pawns[BLACK] & king_zone;

			uint8_t bits = CountBits(pawns_in_zone);
			if (bits < 3) {
				// At least one pawn is too far advanced or dead
				shield += PawnShield[3] * (3 - bits);
			}

			while (pawns_in_zone) {
				uint8_t pawn_sq = PopBit(&pawns_in_zone);
				uint8_t rank = pawn_sq / 8;
				shield += PawnShield[7 - rank];
			}
		}
	}

	return shield;

}

// Manhattan distance
inline int dist_between_squares(uint8_t sq_1, uint8_t sq_2) {
	uint8_t file_1 = FilesBrd[sq_1];
	uint8_t rank_1 = RanksBrd[sq_1];
	uint8_t file_2 = FilesBrd[sq_2];
	uint8_t rank_2 = RanksBrd[sq_2];

	return abs(file_1 - file_2) + abs(rank_1 - rank_2); // standard definition
	// return max( abs(file_1 - file_2), abs(rank_1 - rank_2) ); // alternative definition, to be used for bishops
}

static inline double king_tropism_for_piece(const S_BOARD *pos, int opp_king_sq, uint8_t pce, uint8_t factor) {
	double tropism = 0;

	for (int i = 0; i < pos->pceNum[pce]; ++i) {
		uint8_t sq = pos->pList[pce][i];
		tropism += factor * ( 15 - dist_between_squares(opp_king_sq, sq) );
	}

	return tropism;
}

static inline double kingTropism(const S_BOARD *pos, uint8_t col) {
	// A coarse method to promote better attacks
	// Seems to do better than Attacking King Zone or Attack Units for some reason
	// Also less expensive to use (less drop in NPS)

	double tropism = 0;
	int opp_king_sq = 0;
	uint8_t colour_offset = (col == BLACK) ? 6 : 0;

	// Obtain opponent's king square
	opp_king_sq = pos->KingSq[!col];
	
	// Knights
	uint8_t pce = wN + colour_offset;
	tropism += king_tropism_for_piece(pos, opp_king_sq, pce, 5);

	// Bishops
	pce = wB + colour_offset;
	tropism += king_tropism_for_piece(pos, opp_king_sq, pce, 5);

	// Rooks
	pce = wR + colour_offset;
	tropism += king_tropism_for_piece(pos, opp_king_sq, pce, 5);

	// Queens
	pce = wQ + colour_offset;
	tropism += king_tropism_for_piece(pos, opp_king_sq, pce, 10);

	return tropism;
}

inline double kingSafetyScore(const S_BOARD *pos, uint8_t kingSq, uint8_t col, uint16_t mat) {
	// kingSq = your own king
	// mat = enemy material excluding king
	// The approach of this function is in terms of deductions to your own king

	double kingSafety = punishOpenFiles(pos, kingSq) * 0.85;
	kingSafety += pawnShield(pos, kingSq, col) * 0.15; // default: 0.15
	kingSafety += kingTropism(pos, col) * 0.45;

	return kingSafety * mat / 4039.0; // king safety matters less when there's fewer pieces on the bqoard

}

/*************************
*** Endgame Adjustment ***
*************************/

// Used for some sort of king eval tapering. Probably not very good, but an interesting approach. Kept for legacy
// #define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK])

// Material eval
inline double CountMaterial(const S_BOARD *pos, double *whiteMat, double *blackMat) {
	
	*whiteMat = 0;
	*blackMat = 0;
	for(int index = 0; index < BRD_SQ_NUM; ++index) {
		int piece = pos->pieces[index];
		ASSERT(PceValidEmptyOffbrd(piece));
		if(piece != OFFBOARD && piece != EMPTY) {
			int colour = PieceCol[piece];
			ASSERT(SideValid(colour));
			if (colour == WHITE)
				*whiteMat += PieceValMg[piece] * evalWeight(pos) + PieceValEg[piece] * ( 1 - evalWeight(pos) );
			else 
				*blackMat += PieceValMg[piece] * evalWeight(pos) + PieceValEg[piece] * ( 1 - evalWeight(pos) );
		}
	}

	return *whiteMat - *blackMat;

}

// Test position: 8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41
// Determins if the position is a draw by material (with no pawns)
static inline uint8_t MaterialDraw(int net_material) {
	
	// Upper bound of a bishop's value in the endgame (slightly higher than actual value due to tapered eval)
	#define MAX_MINOR_PIECE 310

	// Seems to replace VICE MaterialDraw() and a few more cases, such as K+Q v K+R+B+B
	if (net_material <= MAX_MINOR_PIECE) {
		return TRUE;
	}

  	return FALSE;
}

// Detects if it's an opposite-coloured bishop endgame, and used to apply a drawish factor
/*
inline uint8_t is_opposite_bishop(const S_BOARD *pos) {

	// Determines if any side has more than one bishop
	if ( (pos->pceNum[wB] == 1) && (pos->pceNum[bB] == 1) ) {
		uint8_t wB_sq = pos->pList[wB][0];
		uint8_t bB_sq = pos->pList[bB][0];
		return ( isLightSq(wB_sq) != isLightSq(bB_sq) );
	} else {
		return FALSE;
	}

}
*/

/********************************
*** Main Evaluation Function ****
********************************/

// Evaluation function
inline int16_t EvalPosition(const S_BOARD *pos) {

	ASSERT(CheckBoard(pos));

	uint8_t pce;
	int pceNum;
	uint8_t sq;

	double score = 0;

	// Material eval
	double whiteMaterial = 0;
	double blackMaterial = 0;
	score += CountMaterial(pos, &whiteMaterial, &blackMaterial);

	// Material draw (checks if there are no pawns as well)
	int netMaterial = (int)fabs(whiteMaterial - blackMaterial);
	if(!pos->pceNum[wP] && !pos->pceNum[bP] && MaterialDraw(netMaterial) == TRUE) {
		return 0;
	}

	// Tapered eval weight. Calculated only once to save resources
	double weight = evalWeight(pos);

	/************
	*** Pawns ***
	************/

	pce = wP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += PawnMgTable[SQ64(sq)] * weight + PawnEgTable[SQ64(sq)] * ( 1 - weight );

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			score += PawnIsolated;
		}

		if( (WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("wP Passed:%s\n",PrSq(sq));
			score += PawnPassed[RanksBrd[sq]];
		}

	}

	pce = bP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		score -= PawnMgTable[MIRROR64(SQ64(sq))] * weight + PawnEgTable[MIRROR64(SQ64(sq))] * ( 1 - weight );

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("bP Iso:%s\n",PrSq(sq));
			score -= PawnIsolated;
		}

		if( (BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("bP Passed:%s\n",PrSq(sq));
			score -= PawnPassed[7 - RanksBrd[sq]];
		}
	}

	/************
	** Knights **
	************/

	pce = wN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += KnightMgTable[SQ64(sq)] * weight + KnightEgTable[SQ64(sq)] * ( 1 - weight );

		// Punish knights in front of c-pawn
		U64 mask = pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]];
		int pawnSq = PopBit(&mask);
		if ( (SQ64(sq) - pawnSq == 8) && (FilesBrd[sq] == FILE_C) ) {
			score += KnightBlocksPawn;
		}
	}

	pce = bN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		score -= KnightMgTable[MIRROR64(SQ64(sq))] * weight + KnightEgTable[MIRROR64(SQ64(sq))] * ( 1 - weight );

		// Punish knights in front of c-pawn
		U64 mask = pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]];
		int pawnSq = PopBit(&mask);
		if ( (pawnSq - SQ64(sq) == 8) && (FilesBrd[sq] == FILE_C) ) {
			score -= KnightBlocksPawn;
		}
	}

	/************
	** Bishops **
	************/

	pce = wB;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {

		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += BishopMgTable[SQ64(sq)] * weight + BishopEgTable[SQ64(sq)] * ( 1 - weight );

		// Punish bishops in front of e- or d-pawn
		U64 mask = pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]];
		int pawnSq = PopBit(&mask);
		if ( (SQ64(sq) - pawnSq == 8) && (FilesBrd[sq] == FILE_D)) {
			score += BishopBlocksPawn;
		} else {
			if ( (SQ64(sq) - pawnSq == 8) && (FilesBrd[sq] == FILE_E) ) {
				score += BishopBlocksPawn;
			}
		}

		// Bonus for bishop-pawn interaction
		// score += bishopPawnComplex(pos, sq, WHITE);
	
	}

	pce = bB;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {

		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		score -= BishopMgTable[MIRROR64(SQ64(sq))] * weight + BishopEgTable[MIRROR64(SQ64(sq))] * ( 1 - weight );
		
		// Punish bishops in front of e- or d-pawn
		U64 mask = pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]];
		int pawnSq = PopBit(&mask);
		if ( (pawnSq - SQ64(sq) == 8) && (FilesBrd[sq] == FILE_D)) {
			score -= BishopBlocksPawn;
		} else {
			if ( (pawnSq - SQ64(sq) == 8) && (FilesBrd[sq] == FILE_E) ) {
				score -= BishopBlocksPawn;
			}
		}

		// Bonus for bishop-pawn interaction
		// score -= bishopPawnComplex(pos, sq, BLACK);

	}
	
	/************
	*** Rooks ***
	************/

	pce = wR;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += RookMgTable[SQ64(sq)] * weight + RookEgTable[SQ64(sq)] * ( 1 - weight );

		ASSERT(FileRankValid(FilesBrd[sq]));

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score += RookOpenFile;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			score += RookSemiOpenFile;
		}
	}

	pce = bR;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		score -= RookMgTable[MIRROR64(SQ64(sq))] * weight + RookEgTable[MIRROR64(SQ64(sq))] * ( 1 - weight );
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= RookOpenFile;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= RookSemiOpenFile;
		}
	}

	/************
	** Queens ***
	************/

	pce = wQ;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += QueenMgTable[SQ64(sq)] * weight + QueenEgTable[SQ64(sq)] * ( 1 - weight );
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score += QueenOpenFile;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			score += QueenSemiOpenFile;
		}
	}

	pce = bQ;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		score -= QueenMgTable[MIRROR64(SQ64(sq))] * weight + QueenEgTable[MIRROR64(SQ64(sq))] * ( 1 - weight );
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenOpenFile;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenSemiOpenFile;
		}
	}

	/************
	*** Kings ***
	************/

	// Test position: 8/p6k/6p1/5p2/P4K2/8/5pB1/8 b - - 2 62
	pce = wK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
	score += KingMgTable[SQ64(sq)] * weight + KingEgTable[SQ64(sq)] * ( 1 - weight );
	score += kingSafetyScore(pos, sq, WHITE, blackMaterial - 50000);

	pce = bK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
	score -= KingMgTable[MIRROR64(SQ64(sq))] * weight + KingEgTable[MIRROR64(SQ64(sq))] * ( 1 - weight );
	score -= kingSafetyScore(pos, sq, BLACK, whiteMaterial - 50000);


	/****************
	* Other bonuses *
	****************/

	// Bishop pair bonus
	if(pos->pceNum[wB] >= 2) score += BishopPair;
	if(pos->pceNum[bB] >= 2) score -= BishopPair;

	/*
	// Opposite-coloured bishop endgame adjustment
	if (is_opposite_bishop(pos) && netMaterial < 310) {
		// Drawishness increases with less (non-king) material on the board
		double drawish_factor = (whiteMaterial + blackMaterial - 100000) / (4039 * 2);
		drawish_factor = log(drawish_factor) / 2.8 + 1; // slope increases as the fraction goes from 1 to 0
		score = (int)(score * drawish_factor);
	}
	*/

	// No good way of calculating mobility
	// Kinda counted already by PSQT and open files / bishop pair bonuses. 
	// Perhaps have to do some trick with setting rooks on certain files to limit king movement in endgame.

	// Perspective adjustment
	if(pos->side == WHITE) {
		return score;
	} else {
		return -score;
	}
}


















