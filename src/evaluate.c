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

// Eval normalisation
// #define squishFactor 0.35

/********************************
***** Evaluation components *****
********************************/

// Applying gamePhase at startpos
#define openingPhase 64

int16_t evaluate_pawn_structure(const S_BOARD *pos, uint8_t pawn_sq, uint8_t col) {

	int16_t pawn_score = 0;

	if( (IsolatedMask[SQ64(pawn_sq)] & pos->pawns[col]) == 0) {
		//printf("wP Iso:%s\n",PrSq(sq));
		pawn_score += PawnIsolated;
	}

	U64 mask = FileBBMask[FilesBrd[pawn_sq]] & pos->pawns[col];
	uint8_t stacked_count = CountBits(mask);
	if(stacked_count > 1) {
		//printf("wP Iso:%s\n",PrSq(sq));
		pawn_score += PawnDoubled * (stacked_count - 1); // Scales with doubled and tripled pawns
	}

	if (col == WHITE) {
		if( (WhitePassedMask[SQ64(pawn_sq)] & pos->pawns[BLACK]) == 0) {
			//printf("wP Passed:%s\n",PrSq(sq));
			pawn_score += PawnPassed[RanksBrd[pawn_sq]];
		}
	} else {
		if( (BlackPassedMask[SQ64(pawn_sq)] & pos->pawns[WHITE]) == 0) {
			//printf("bP Passed:%s\n",PrSq(sq));
			pawn_score += PawnPassed[7 - RanksBrd[pawn_sq]];
		}
	}
	
	return pawn_score;
}

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

// Punishing kings in the center without castling rights
// Should be greater punishment than castled with a broken a pawn shield
static inline int16_t punish_center_kings(const S_BOARD *pos, uint8_t king_sq, uint8_t col) {

	const int file_punishment[8] = { 0, 0, 0, -50, -100, -50, 0, 0 };
	const int rank_punishment[8] = { 0, -25, -75, -125, -150, -175, -200, -225 };
	uint8_t no_castling = 0;
	uint8_t relative_rank = 0;
	if (col == WHITE) {
		no_castling = (pos->castlePerm & 0b0011) == 0;
		relative_rank = RanksBrd[king_sq];
	} else {
		no_castling = (pos->castlePerm & 0b1100) == 0;
		relative_rank = 8 - RanksBrd[king_sq];
	}

	if (no_castling) {
		return file_punishment[FilesBrd[king_sq]] + rank_punishment[relative_rank];
	} else {
		return 0;
	}

}

static inline double king_tropism_for_knight(const S_BOARD *pos, int opp_king_sq, uint8_t pce, uint8_t factor) {

	double tropism = 0;

	for (int i = 0; i < pos->pceNum[pce]; ++i) {
		uint8_t sq = pos->pList[pce][i];
		tropism += factor * ( 15 - (dist_between_squares(opp_king_sq, sq) - 3)); // Minimum possible distance is 2 for queen and 3 for knight
	}

	return tropism;
}

static inline double king_tropism_for_bishop(const S_BOARD *pos, int opp_king_sq, uint8_t pce) {

	const int KiDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 }; // From attack.c
	const uint8_t bishop_king_sq_bonus = 4;
	double tropism = 0;

	for (int i = 0; i < pos->pceNum[pce]; ++i) {
		uint8_t sq = pos->pList[pce][i];
		for (int j = 0; j < 8; ++j) {
			uint8_t sq_near_king = sq + KiDir[j];
			if ( (SQ64(sq_near_king) != 65) && on_same_diagonal(opp_king_sq, sq) ) {
				tropism += bishop_king_sq_bonus;
			}
		}
	}

	return tropism;
}

static inline int16_t king_tropism_for_RQ(const S_BOARD *pos, uint8_t opp_king_sq, uint8_t col) {
	// Applies to Rooks and Queens
	
	const uint8_t rook_KingSemiopenFile = 10;
	const uint8_t rook_KingOpenFile[3] = { 25, 55, 25 };
	const uint8_t queen_KingSemiopenFile = 10;
	const uint8_t queen_KingOpenFile[3] = { 40, 70, 40 };

	uint8_t king_file = FilesBrd[opp_king_sq];
	uint8_t col_offset = (col == WHITE) ? 0 : 6;
	uint16_t tropism = 0;

	uint8_t pce = wR + col_offset;
	for (int i = 0; i < pos->pceNum[pce]; ++i) {
		uint8_t sq = pos->pList[pce][i];
		uint8_t relative_file = FilesBrd[sq] - king_file;

		if (abs(relative_file) < 1) {
			if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
				tropism -= rook_KingSemiopenFile;
			} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
				uint8_t relative_file = FilesBrd[sq] - king_file;
				tropism += rook_KingOpenFile[relative_file];
			}
		}
	}

	pce = wQ + col_offset;
	for (int i = 0; i < pos->pceNum[pce]; ++i) {
		uint8_t sq = pos->pList[pce][i];
		uint8_t relative_file = FilesBrd[sq] - king_file;

		if (abs(relative_file) < 1) {
			if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
				tropism += queen_KingSemiopenFile;
			} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
				uint8_t relative_file = FilesBrd[sq] - king_file;
				tropism += queen_KingOpenFile[relative_file];
			}
		}
	}


	return tropism;
}

static inline double king_tropism(const S_BOARD *pos, uint8_t col) {
	// A coarse method to promote better attacks
	// Seems to do better than Attacking King Zone or Attack Units for some reason
	// Also less expensive to use (less drop in NPS)

	double tropism = 0;
	int opp_king_sq = 0;
	uint8_t colour_offset = (col == BLACK) ? 6 : 0;

	// Obtain opponent's king square
	opp_king_sq = pos->KingSq[!col];
	
	// Bishops and Knights
	uint8_t pce = wN + colour_offset;
	tropism += king_tropism_for_knight(pos, opp_king_sq, pce, 1); // 3 ~ 15

	pce = wB + colour_offset;
	tropism += king_tropism_for_bishop(pos, opp_king_sq, pce); // 0 ~ 24

	// Rooks and Queens
	tropism += king_tropism_for_RQ(pos, opp_king_sq, col) * 0.7; // 0 ~ 180. Default 0.8

	return tropism;
}

static inline int16_t punish_open_files(const S_BOARD *pos, uint8_t kingSq, uint8_t col) {
	// col = Side with the king

	uint8_t king_file = FilesBrd[kingSq];
	const int16_t KingOpenFile[3] = { -50, -70, -50 };
	int16_t openLines = 0;

	//    For edge cases
	if (king_file == FILE_A || king_file == FILE_H) {
		// Open king file
		if (!(pos->pawns[BOTH] & FileBBMask[king_file])) {
			openLines += KingOpenFile[1];
		}
		if (king_file == FILE_A) {
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
		for (int file = king_file - 1; file <= king_file + 1; ++file) {
			// Open king file
			if (!(pos->pawns[BOTH] & FileBBMask[file])) {
				openLines += KingOpenFile[file - king_file + 1];
			} 
		}
	}

	return openLines;

}

U64 generate_shield_zone(uint8_t kingSq, uint8_t col) {

	/*
	--------
	--------
	--------
	--------
	-----xxx
	-----xxx
	-----xxx
	------K-
	*/

	U64 shield_zone = 0ULL;
	uint8_t kingFile = FilesBrd[kingSq];
	uint8_t kingRank = RanksBrd[kingSq];

	if (col == WHITE) {
		for (int rank = kingRank + 1; rank <= kingRank + 3; ++rank) {
			for (int file = kingFile - 1; file <= kingFile + 1; ++file) {
				uint8_t sq = SQ64(FR2SQ(file, rank));
				if (sq != 65) {
					shield_zone |= 1ULL << sq;
				}
			}
		}
	} else {
		for (int rank = kingRank - 1; rank >= kingRank - 3; --rank) {
			for (int file = kingFile - 1; file <= kingFile + 1; ++file) {
				uint8_t sq = SQ64(FR2SQ(file, rank));
				if (sq != 65) {
					shield_zone |= 1ULL << sq;
				}
			}
		}
	}

	return shield_zone;
}

static inline int16_t pawn_shield(const S_BOARD *pos, uint8_t kingSq, uint8_t col) {

	// [0]: starting sq
	// [1]: moved 1 sq
	// [2]: moved 2 sq
	// [3]: moved 3+ sq / dead. this value shouldn't be too high as kingOpenFile exists
	const int PawnShield[4] = { 0, -10, -18, -125 }; // startpos, moved 1 sq, 2 sq, too far away / dead. [3] shouldn't be too high as kingOpenFile exists
	U64 castled_king = 0ULL;
	int16_t shield = 0;

	if (col == WHITE) {
		// Pawn shield only applies to castled king
		castled_king = RankBBMask[RANK_1];
		castled_king ^= (1ULL << SQ64(D1)) | (1ULL << SQ64(E1)) | (1ULL << SQ64(F1)); // Not on d1 e1 or f1
		if (castled_king & (1ULL << SQ64(kingSq))) {
			U64 king_zone = generate_shield_zone(kingSq, WHITE);
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
			U64 king_zone = generate_shield_zone(kingSq, BLACK);
			U64 pawns_in_zone = pos->pawns[BLACK] & king_zone;

			uint8_t bits = CountBits(pawns_in_zone);
			if (bits < 3) {
				// At least one pawn is too far advanced or dead
				// Works even in cases when king is on A/H files
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

static inline double king_safety_score(const S_BOARD *pos, uint8_t kingSq, uint8_t col, uint16_t mat) {
	// kingSq = your own king
	// mat = enemy material excluding king
	// The approach of this function is in terms of deductions to your own king

	double king_safety = 0;
	king_safety += pawn_shield(pos, kingSq, col) * 0.35;
	// king_safety += punish_center_kings(pos, kingSq, col) * 1; // default: 1
	king_safety += king_tropism(pos, col) * 1; // default: 1, Pending rewrite and tuning
	king_safety += punish_open_files(pos, kingSq, col) * 0.85; // Pending tuning

	// Will have to try a different way of scoring the phase, like number of pieces but greater value for queens
	return king_safety * mat / 4039.0; // king safety matters less when there's fewer pieces on the bqoard

}

/*************************
*** Endgame Adjustment ***
*************************/

// Used for some sort of king eval tapering. Probably not very good, but an interesting approach. Kept for legacy
// #define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK])

// Rewarding active kings and punishing immobile kings to assist mates
static inline int16_t king_mobility(const S_BOARD *pos, uint8_t king_sq, uint8_t col) {

	const int KiDir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 }; // From attack.c
	//                               0    1    2    3   4  5   6   7   8
	const int mobility_bonus[9] = { -75, -50, -33, -25, 0, 5, 10, 11, 12};

	uint8_t mobile_squares = 0;
	for (int i = 0; i < 8; ++i) {
		if (SQ64(king_sq + KiDir[i]) != 65) {
			if (!SqAttacked(king_sq + KiDir[i], !col, pos)) {
				mobile_squares++;
			}
		}
		
	}

	return mobility_bonus[mobile_squares];
}

// Material eval
static inline double CountMaterial(const S_BOARD *pos, double weight, double *whiteMat, double *blackMat) {
	
	*whiteMat = 0;
	*blackMat = 0;

	for(int index = 0; index < BRD_SQ_NUM; ++index) {
		int piece = pos->pieces[index];
		ASSERT(PceValidEmptyOffbrd(piece));
		// Removing piece != OFFBOARD causes major problems for some reason
		if (piece != OFFBOARD && piece != EMPTY) {
			uint8_t col = PieceCol[piece];
			ASSERT(SideValid(col));
			if (col == WHITE)
				*whiteMat += PieceValMg[piece] * weight + PieceValEg[piece] * (1 - weight);
			else 
				*blackMat += PieceValMg[piece] * weight + PieceValEg[piece] * (1 - weight);
		}
	}

	return *whiteMat - *blackMat;

}

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
	// Tapered eval weight (middlegame perspective)
	double weight = evalWeight(pos);

	/*
		Material
	*/

	double whiteMaterial = 0;
	double blackMaterial = 0;
	double net_material = CountMaterial(pos, weight, &whiteMaterial, &blackMaterial);
	score += net_material;

	// Material draw
	// TODO: Make is_material_draw include 1 pawn or less for each side
	// Check if it's in endgame (8 or less pieces exc. pawns on the board)
	uint8_t piece_count = CountBits(pos->occupancy[BOTH]) - CountBits(pos->pawns[BOTH]);
	uint8_t isEndgame = piece_count < 8;
	if (isEndgame) {
		if ( !pos->pceNum[wP] && !pos->pceNum[bP] && is_material_draw(pos, (int)fabs(net_material)) ) {
			return 0;
		}
	}

	/*
		Evaluate pawns
	*/

	pce = wP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += PawnMgTable[SQ64(sq)] * weight + PawnEgTable[SQ64(sq)] * (1 - weight);
		score += evaluate_pawn_structure(pos, sq, WHITE);
	}

	pce = bP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		score -= PawnMgTable[MIRROR64(SQ64(sq))] * weight + PawnEgTable[MIRROR64(SQ64(sq))] * (1 - weight);	
		score -= evaluate_pawn_structure(pos, sq, BLACK);
	}

	/*
		Evaluate knights
	*/

	pce = wN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += KnightMgTable[SQ64(sq)] * weight + KnightEgTable[SQ64(sq)] * (1 - weight);

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
		score -= KnightMgTable[MIRROR64(SQ64(sq))] * weight + KnightEgTable[MIRROR64(SQ64(sq))] * (1 - weight);

		// Punish knights in front of c-pawn
		U64 mask = pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]];
		int pawnSq = PopBit(&mask);
		if ( (pawnSq - SQ64(sq) == 8) && (FilesBrd[sq] == FILE_C) ) {
			score -= KnightBlocksPawn;
		}
	}

	/*
		Evaluate bishops
	*/

	pce = wB;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {

		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += BishopMgTable[SQ64(sq)] * weight + BishopEgTable[SQ64(sq)] * (1 - weight);

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
	}

	pce = bB;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {

		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
		score -= BishopMgTable[MIRROR64(SQ64(sq))] * weight + BishopEgTable[MIRROR64(SQ64(sq))] * (1 - weight);
		
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
	}
	
	/*
		Evaluate rooks
	*/

	pce = wR;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += RookMgTable[SQ64(sq)] * weight + RookEgTable[SQ64(sq)] * (1 - weight);

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
		score -= RookMgTable[MIRROR64(SQ64(sq))] * weight + RookEgTable[MIRROR64(SQ64(sq))] * (1 - weight);
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= RookOpenFile;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= RookSemiOpenFile;
		}
	}

	/*
		Evaluate queens
	*/

	pce = wQ;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += QueenMgTable[SQ64(sq)] * weight + QueenEgTable[SQ64(sq)] * (1 - weight);
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
		score -= QueenMgTable[MIRROR64(SQ64(sq))] * weight + QueenEgTable[MIRROR64(SQ64(sq))] * (1 - weight);
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenOpenFile;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenSemiOpenFile;
		}
	}

	/*
		Evaluate kings
	*/

	// Test position: 8/p6k/6p1/5p2/P4K2/8/5pB1/8 b - - 2 62
	pce = wK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
	score += KingMgTable[SQ64(sq)] * weight + KingEgTable[SQ64(sq)] * (1 - weight);
	score += king_safety_score(pos, sq, WHITE, blackMaterial - 50000);
	if (isEndgame) {
		score += king_mobility(pos, sq, WHITE);
	}

	pce = bK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(MIRROR64(SQ64(sq))>=0 && MIRROR64(SQ64(sq))<=63);
	score -= KingMgTable[MIRROR64(SQ64(sq))] * weight + KingEgTable[MIRROR64(SQ64(sq))] * (1 - weight);
	score -= king_safety_score(pos, sq, BLACK, whiteMaterial - 50000);
	if (isEndgame) {
		score -= king_mobility(pos, sq, BLACK);
	}


	/*
		Bonuses and Adjustments
	*/

	// Bishop pair bonus
	if(pos->pceNum[wB] >= 2) score += BishopPair;
	if(pos->pceNum[bB] >= 2) score -= BishopPair;

	// 50-move rule adjustment
	if (score < ISMATE) {
		score = (int)( score * ( (100 - pos->fiftyMove) / 100.0) );
	}

	// No good way of calculating mobility

	// Perspective adjustment
	if(pos->side == WHITE) {
		return score;
	} else {
		return -score;
	}
}


















