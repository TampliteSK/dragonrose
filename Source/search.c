// search.c

#include "stdio.h"
#include <math.h>
#include "string.h"
#include "defs.h"

int rootDepth;

static void CheckUp(S_SEARCHINFO *info) {
	// .. check if time up, or interrupt from GUI
	if(info->timeset == TRUE && GetTimeMs() > info->stoptime) {
		info->stopped = TRUE;
	}
}

static void PickNextMove(int moveNum, S_MOVELIST *list) {

	S_MOVE temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index) {
		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}

	ASSERT(moveNum>=0 && moveNum<list->count);
	ASSERT(bestNum>=0 && bestNum<list->count);
	ASSERT(bestNum>=moveNum);

	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

static int IsRepetition(const S_BOARD *pos) {

	int index = 0;

	for(index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index) {
		ASSERT(index >= 0 && index < MAXGAMEMOVES);
		if(pos->posKey == pos->history[index].posKey) {
			return TRUE;
		}
	}
	return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info) {

	for(int index = 0; index < 13; ++index) {
		for(int index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	for(int index = 0; index < 2; ++index) {
		for(int index2 = 0; index2 < MAXDEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}

	table->overWrite = 0;
	table->hit = 0;
	table->cut = 0;
	table->currentAge++;
	pos->ply = 0;

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static inline int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if(IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	int32_t Score = EvalPosition(pos); // stand-pat score

	ASSERT(Score>-INF_BOUND && Score<INF_BOUND);

	// Beta cutoff
	if(Score >= beta) {
		return beta;
	}
	
	// Delta pruning (dead lost scenario)
	uint16_t big_delta = 936; // queen eg value
	// It is unlikely we can gain much back if we're dead lost, so we can stop searching
	if (Score < alpha - big_delta) {
		return alpha;
	}

	if(Score > alpha) {
		alpha = Score;
	}

	// Delta pruning fails, and we have to search moves
	S_MOVELIST list[1];
    GenerateAllCaps(pos,list);

    int MoveNum = 0;
	int Legal = 0;
	Score = -INF_BOUND;
	
	// Delta pruning buffer.
	#define DELTA_BUFFER 180

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list); // finds highest-scoring move between MoveNum index and count-1 index

		// Delta pruning (general case)
		// int mask = 0xFFFFFFFF & (0b1111 << 14);
		// int capturedPiece = (list->moves[MoveNum].move & mask) >> 14;
		int capturedPiece = CAPTURED(list->moves[MoveNum].move);
		int delta = PieceValMg[capturedPiece]; // mg values
		// If the gain from capturing a piece is too low (not enough to improve alpha), we skip make/undo moves and evalaution
		// buffer: 180
		if (delta + DELTA_BUFFER <= alpha) {
			return alpha;
		}

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -Quiescence( -beta, -alpha, pos, info);
        TakeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}

		if(Score > alpha) {
			if(Score >= beta) {
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = Score;
		}
    }

	return alpha;
}

static inline int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info, int DoNull) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	ASSERT(depth>=0);

	if(depth <= 0) {
		return Quiescence(alpha, beta, pos, info);
		// return EvalPosition(pos);
	}

	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	int InCheck = SqAttacked(pos->KingSq[pos->side],pos->side^1,pos);

	if(InCheck == TRUE) {
		depth++;
	}

	int Score = -INF_BOUND;
	int PvMove = NOMOVE;

	if( ProbeHashEntry(pos, table, &PvMove, &Score, alpha, beta, depth) == TRUE ) {
		table->cut++;
		return Score;
	}

	// Null-move pruning
	//                                     Note kings are considered bigPce, so we have to set the range to >1
	if( DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 1) && depth >= 4) {
		MakeNullMove(pos);
		Score = -AlphaBeta( -beta, -beta + 1, depth-4, pos, table, info, FALSE);
		TakeNullMove(pos);
		if(info->stopped == TRUE) {
			return 0;
		}

		if (Score >= beta && abs(Score) < ISMATE) {
			info->nullCut++;
			return beta;
		}
	}

	S_MOVELIST list[1];
    GenerateAllMoves(pos,list);

    int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;

	int BestScore = -INF_BOUND;

	Score = -INF_BOUND;

	if( PvMove != NOMOVE) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list);
		
		// Futility pruning (default: 365 for normal. 225? for 0.25P)
		#define FUTILITY_MARGIN 225
		// We check if it is a frontier node (1 ply from horizon) and the eval is not close to mate
		if (depth == 1 && abs(Score) < ISMATE) {
			int currentEval = EvalPosition(pos);
			int capturedPiece = CAPTURED(list->moves[MoveNum].move);
			
			// Check to make sure it's not a capture or check
			if (capturedPiece == EMPTY && !SqAttacked(pos->KingSq[!pos->side], pos->side, pos)) {
				// If the gain from capturing a piece is less than a minor piece, we skip this move
				if (currentEval + FUTILITY_MARGIN <= alpha) {
					continue;
				}
			}
		}

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -AlphaBeta( -beta, -alpha, depth-1, pos, table, info, TRUE);
		TakeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}

		if(Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
			if(Score > alpha) {
				if(Score >= beta) {
					if(Legal==1) {
						info->fhf++;
					}
					info->fh++;

					if(!(list->moves[MoveNum].move & MFLAGCAP)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}

					StoreHashEntry(pos, table, BestMove, beta, HFBETA, depth);

					return beta;
				}
				alpha = Score;

				if(!(list->moves[MoveNum].move & MFLAGCAP)) {
					pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
				}
			}
		}
    }

	if(Legal == 0) {
		if(InCheck) {
			return -INF_BOUND + pos->ply;
		} else {
			return 0;
		}
	}

	ASSERT(alpha>=OldAlpha);

	if(alpha != OldAlpha) {
		StoreHashEntry(pos, table, BestMove, BestScore, HFEXACT, depth);
	} else {
		StoreHashEntry(pos, table, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

/*************
*** Iterative Deepening Loop ***
*******************************/

void SearchPosition(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info) {

	int bestMove = NOMOVE;
	int bestScore = -INF_BOUND;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos, table, info);
	
	// Get moves from opening book
	if(EngineOptions->UseBook == TRUE) {
		bestMove = GetBookMove(pos);
	}

	//printf("Search depth:%d\n",info->depth);

	if(bestMove == NOMOVE) {
		for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
								// alpha	 beta
			rootDepth = currentDepth;
			bestScore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, pos, table, info, TRUE);

			if(info->stopped == TRUE) {
				break;
			}

			pvMoves = GetPvLine(currentDepth, pos, table);
			bestMove = pos->PvArray[0];

			// Display mate if there's forced mate
			uint8_t mateFound = FALSE;
			if (abs(bestScore) >= ISMATE) {
				mateFound = TRUE;
				// copysign(1.0, value) outputs +/- 1.0 depending on the sign of "value"
				// this should be a cleaner way than a ternary
				int8_t mateMoves = round( (INF_BOUND - fabs(bestScore)) / 2 ) * copysign(1.0, bestScore);
				printf("info score mate %d depth %d nodes %ld time %d pv",
					mateMoves,currentDepth,info->nodes,GetTimeMs()-info->starttime);
			} else {
				printf("info score cp %d depth %d nodes %ld time %d pv",
					bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
			}

			for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
				printf(" %s",PrMove(pos->PvArray[pvNum]));
			}
			printf("\n");

			// Exit search if mate at current depth is found, in order to save time
			if (bestScore + INF_BOUND == currentDepth) {
				break;
				// Buggy if no search is performed before pruning immediately
			}
		}
	}

	printf("bestmove %s\n",PrMove(bestMove));

}