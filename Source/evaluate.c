// evaluate.c

#include <stdio.h>
#include <math.h>
#include "defs.h"
#include <stdint.h>
#include <stdlib.h>

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

/* Pawns */
const int8_t PawnIsolated = -10;
// Basic passed pawn bonus. Values from Stockfish and then averaged
// Failed. Loses elo anyway. Reverted
// original: 0, 5, 10, 20, 35, 60, 100, 200
const uint8_t PawnPassed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };

// Pieces
const uint8_t BishopPair = 30;
const int8_t KnightBlocksPawn = -15;
const int8_t BishopBlocksPawn = -35;
const uint8_t RookOpenFile = 10;
const uint8_t RookSemiOpenFile = 5;
const uint8_t QueenOpenFile = 5;
const uint8_t QueenSemiOpenFile = 3;

/********************************
* PesTO / Rofchade Piece Tables *
********************************/
// Note: All these tables are flipped to match the VICE implementation
// Attempting to change int[] to int16_t[] will break things



// OPENING / MIDDLEGAME

const int PawnMgTable[64] = {
    0,    0,    0,    0,    0,    0,    0,    0,
  -35,   -1,  -20,  -23,  -15,   24,   38,  -22,
  -26,   -4,   -4,  -10,    3,    3,   33,  -12,
  -27,   -2,   -5,   12,   17,    6,   10,  -25,
  -14,   13,    6,   21,   23,   12,   17,  -23,
   -6,    7,   26,   31,   65,   56,   25,  -20,
   98,  134,   61,   95,   68,  126,   34,  -11,
    0,    0,    0,    0,    0,    0,    0,    0,
};

const int KnightMgTable[64] = {
  -105,  -21,  -58,  -33,  -17,  -28,  -19,  -23,
  -29,  -53,  -12,   -3,   -1,   18,  -14,  -19,
  -23,   -9,   12,   10,   19,   17,   25,  -16,
  -13,    4,   16,   13,   28,   19,   21,   -8,
   -9,   17,   19,   53,   37,   69,   18,   22,
  -47,   60,   37,   65,   84,  129,   73,   44,
  -73,  -41,   72,   36,   23,   62,    7,  -17,
 -167,  -89,  -34,  -49,   61,  -97,  -15, -107
};

const int BishopMgTable[64] = {
  -33,   -3,  -14,  -21,  -13,  -12,  -39,  -21,
    4,   15,   16,    0,    7,   21,   33,    1,
    0,   15,   15,   15,   14,   27,   18,   10,
   -6,   13,   13,   26,   34,   12,   10,    4,
   -4,    5,   19,   50,   37,   37,    7,   -2,
  -16,   37,   43,   40,   35,   50,   37,   -2,
  -26,   16,  -18,  -13,   30,   59,   18,  -47,
  -29,    4,  -82,  -37,  -25,  -42,    7,   -8
};

const int RookMgTable[64] = {
  -19,  -13,    1,   17,   16,    7,  -37,  -26,
  -44,  -16,  -20,   -9,   -1,   11,   -6,  -71,
  -45,  -25,  -16,  -17,    3,    0,   -5,  -33,
  -36,  -26,  -12,   -1,    9,   -7,    6,  -23,
  -24,  -11,    7,   26,   24,   35,   -8,  -20,
   -5,   19,   26,   36,   17,   45,   61,   16,
   27,   32,   58,   62,   80,   67,   26,   44,
   32,   42,   32,   51,   63,    9,   31,   43
};

const int QueenMgTable[64] = {
	-1,  -18,   -9,   10,  -15,  -25,  -31,  -50,
   -35,   -8,   11,    2,    8,   15,   -3,    1,
   -14,    2,  -11,   -2,   -5,    2,   14,    5,
    -9,  -26,   -9,  -10,   -2,   -4,    3,   -3,
   -27,  -27,  -16,  -16,   -1,   17,   -2,    1,
   -13,  -17,    7,    8,   29,   56,   47,   57,
   -24,  -39,   -5,    1,  -16,   57,   28,   54,
   -28,    0,   29,   12,   59,   44,   43,   45
};

const int KingMgTable[64] = {
  -15,   36,   12,  -54,    8,  -28,   24,   14,
    1,    7,   -8,  -64,  -43,  -16,    9,    8,
  -14,  -14,  -22,  -46,  -44,  -30,  -15,  -27,
  -49,   -1,  -27,  -39,  -46,  -44,  -33,  -51,
  -17,  -20,  -12,  -27,  -30,  -25,  -14,  -36,
   -9,   24,    2,  -16,  -20,    6,   22,  -22,
   29,   -1,  -20,   -7,   -8,   -4,  -38,  -29,
  -65,   23,   16,  -15,  -56,  -34,    2,   13
};

// ENDGAME

const int PawnEgTable[64] = {
    0,    0,    0,    0,    0,    0,    0,    0,
   13,    8,    8,   10,   13,    0,    2,   -7,
    4,    7,   -6,    1,    0,   -5,   -1,   -8,
   13,    9,   -3,   -7,   -7,   -8,    3,   -1,
   32,   24,   13,    5,   -2,    4,   17,   17,
   94,  100,   85,   67,   56,   53,   82,   84,
  178,  173,  158,  134,  147,  132,  165,  187,
    0,    0,    0,    0,    0,    0,    0,    0
};

const int KnightEgTable[64] = {
  -29,  -51,  -23,  -15,  -22,  -18,  -50,  -64,
  -42,  -20,  -10,   -5,   -2,  -20,  -23,  -44,
  -23,   -3,   -1,   15,   10,   -3,  -20,  -22,
  -18,   -6,   16,   25,   16,   17,    4,  -18,
  -17,    3,   22,   22,   22,   11,    8,  -18,
  -24,  -20,   10,    9,   -1,   -9,  -19,  -41,
  -25,   -8,  -25,   -2,   -9,  -25,  -24,  -52,
  -58,  -38,  -13,  -28,  -31,  -27,  -63,  -99
};

const int BishopEgTable[64] = {
  -23,   -9,  -23,   -5,   -9,  -16,   -5,  -17,
  -14,  -18,   -7,   -1,    4,   -9,  -15,  -27,
  -12,   -3,    8,   10,   13,    3,   -7,  -15,
   -6,    3,   13,   19,    7,   10,   -3,   -9,
   -3,    9,   12,    9,   14,   10,    3,    2,
    2,   -8,    0,   -1,   -2,    6,    0,    4,
   -8,   -4,    7,  -12,   -3,  -13,   -4,  -14,
  -14,  -21,  -11,   -8,   -7,   -9,  -17,  -24
};

const int RookEgTable[64] = {
   -9,    2,    3,   -1,   -5,  -13,    4,  -20,
   -6,   -6,    0,    2,   -9,   -9,  -11,   -3,
   -4,    0,   -5,   -1,   -7,  -12,   -8,  -16,
    3,    5,    8,    4,   -5,   -6,   -8,  -11,
    4,    3,   13,    1,    2,    1,   -1,    2,
    7,    7,    7,    5,    4,   -3,   -5,   -3,
   11,   13,   13,   11,   -3,    3,    8,    3,
   13,   10,   18,   15,   12,   12,    8,    5
};

const int QueenEgTable[64] = {
  -33,  -28,  -22,  -43,   -5,  -32,  -20,  -41,
  -22,  -23,  -30,  -16,  -16,  -23,  -36,  -32,
  -16,  -27,   15,    6,    9,   17,   10,    5,
  -18,   28,   19,   47,   31,   34,   39,   23,
    3,   22,   24,   45,   57,   40,   57,   36,
  -20,    6,    9,   49,   47,   35,   19,    9,
  -17,   20,   32,   41,   58,   25,   30,    0,
   -9,   22,   22,   27,   27,   19,   10,   20
};

const int KingEgTable[64] = {
  -53,  -34,  -21,  -11,  -28,  -14,  -24,  -43,
  -27,  -11,    4,   13,   14,    4,   -5,  -17,
  -19,   -3,   11,   21,   23,   16,    7,   -9,
  -18,   -4,   21,   24,   27,   23,    9,  -11,
   -8,   22,   24,   27,   26,   33,   26,    3,
   10,   17,   23,   15,   20,   45,   44,   13,
  -12,   17,   14,   17,   17,   38,   23,   11,
  -74,  -35,  -18,  -18,  -11,   15,    4,  -17
};

/********************************
***** Evaluation components *****
********************************/

// Applying gamePhase at startpos
#define openingPhase 64

// Returns 1 if it's a light square
uint8_t isLightSq(uint8_t sq) {
	return !( (sq % 2) ^ ( ( sq / 10 ) % 2) );
}

// Gives bonuses if bishop and pawns are of different colour complexes
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

// Calculates the weight of tapered eval. 
double evalWeight(const S_BOARD *pos) {
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

// Test position: 8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41
// Determins if the position is a draw by material (doesn't include pawns)
uint8_t MaterialDraw(const S_BOARD *pos) {

	ASSERT(CheckBoard(pos));

	// Minor piece endgame
    if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
	  if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
	      if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) {  return TRUE; }
	  } else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
	     if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) { return TRUE; }
	  } else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
	    if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN]))  { return TRUE; }
	  }
	} else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {
            if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2)	{ return TRUE; }
        } else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {
            if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2))) { return TRUE; }
        } else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {
            if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2))) { return TRUE; }
        }
    }
  return FALSE;
}

/******************
*** King Safety ***
******************/

inline int16_t punishOpenFiles(const S_BOARD *pos, uint8_t kingSq) {
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

inline int16_t pawnShield(const S_BOARD *pos, uint8_t kingSq, uint8_t col) {
	// An attempt was made to rewrite this in bitboard, but it turned out to be way worse

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

	uint8_t kingFile = FilesBrd[kingSq];
	uint8_t kingRank = RanksBrd[kingSq];
	const int8_t PawnShield[4] = { 0, -10, -20, -50 }; // startpos, moved 1 sq, 2 sq, too far away / dead. [3] shouldn't be too high as kingOpenFile exists
	U64 castledKing = 0ULL;
	int16_t shield = 0;

	if (col == WHITE) {
		castledKing = RankBBMask[RANK_1] & ~( (1ULL << SQ64(D1)) | (1ULL << SQ64(E1)) | (1ULL << SQ64(F1)) );
		// Pawn shield only applies to castled king
		if (castledKing & (1ULL << SQ64(kingSq))) {
			// Nested-for loop for the 3x3 rectangle in front of the king
			for (int rank = kingRank + 1; rank <= kingRank + 3; rank++) {
				for (int file = kingFile - 1; file <= kingFile + 1; file++) {
					if (pos->pieces[FR2SQ(file, rank)] == wP) {
						// There's a pawn in this zone
						shield += PawnShield[rank - kingRank - 1];
					} else {
						if (rank == kingRank + 3) {
							// There is no pawn shield within this file
							shield += PawnShield[3];
						}
					}	
				}
			} 
		}
	} 
	else {
		castledKing = RankBBMask[RANK_8] & ~( (1ULL << SQ64(D8)) | (1ULL << SQ64(E8)) | (1ULL << SQ64(F8)) );
		if (castledKing & (1ULL << SQ64(kingSq))) {
			for (int rank = kingRank - 1; rank >= kingRank - 3; rank--) {
				for (int file = kingFile - 1; file <= kingFile + 1; file++) {
					if (pos->pieces[FR2SQ(file, rank)] == bP) {
						// There's a pawn in this zone
						shield += PawnShield[kingRank - rank - 1];
					} else {
						if (rank == kingRank - 3) {
							// There is no pawn shield within this zone (and by extension, file)
							shield += PawnShield[3];
						}
				}		
				}
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


inline double kingTropism(const S_BOARD *pos, uint8_t col) {
	// A coarse method to promote better attacks
	// Seems to do better than Attacking King Zone or Attack Units for some reason
	// Also less expensive to use (less drop in NPS)

	double tropism = 0;
	int opp_king_sq = 0;

	// Obtain opponent's king square
	if (col == WHITE) {
		opp_king_sq = pos->pList[bK][0];
	} else {
		opp_king_sq = pos->pList[wK][0];
	}

	// Loop through every square and sum up weight distances 
	for (int sq = 0; sq < BRD_SQ_NUM; ++sq) {
		uint8_t pce = pos->pieces[sq];
		if ( PieceCol[pce] == col ) {
			if (PieceRBN[pce]) {
				tropism += 5 * ( 15 - dist_between_squares(opp_king_sq, sq) ); // rook knight case
			}
			if ( (pce == wQ) || (pce == bQ) ) {
				tropism += 10 * ( 15 - dist_between_squares(opp_king_sq, sq) ); // queen case
			}
		}
	}
	return tropism;
}

inline double kingSafetyScore(const S_BOARD *pos, uint8_t kingSq, uint8_t col, uint16_t mat) {
	// kingSq = your own king
	// mat = enemy material excluding king
	// The approach of this function is in terms of deductions to your own king

	double kingSafety = punishOpenFiles(pos, kingSq) * 0.85;
	kingSafety += pawnShield(pos, kingSq, col) * 0.15;
	kingSafety += kingTropism(pos, col) * 0.45;

	return kingSafety * mat / 4039.0; // king safety matters less when there's fewer pieces on the bqoard

}

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

// Used for some sort of king eval tapering. Probably not very good, but an interesting approach. Kept for legacy
// #define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK])

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

	// Material draw
	if(!pos->pceNum[wP] && !pos->pceNum[bP] && MaterialDraw(pos) == TRUE) {
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


















