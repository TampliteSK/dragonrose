// search.c

#include "stdio.h"
#include <stdlib.h>
#include <math.h>
#include "string.h"
#include "defs.h"

static void CheckUp(S_SEARCHINFO *info) {
	// .. check if time up, or interrupt from GUI
	if( (info->timeset == TRUE) && (GetTimeMs() > info->stoptime) ) {
		info->stopped = TRUE;
	}
}

// Sort the move list once and get moves in sequential order
static void sort_move_list(S_MOVELIST *list, int move_count) {

	// Invalid input
	if (list == NULL || move_count <= 0) {
        return;
    }
	
	S_MOVE temp;

	// Insertion sort
	for (int i = 1; i < move_count; ++i) {
        temp = list->moves[i];
        
        // Move elements of list[0..i-1], that have lower score than temp,
        // to one position ahead of their current position
		int j = i - 1;
        while ( (j >= 0) && (list->moves[j].score < temp.score) ) {
			list->moves[j + 1] = list->moves[j];
            j = j - 1;
        }
        list->moves[j + 1] = temp;
    }

}

/*
// Sorts the list and picks the move that is ordered highest
static void PickNextMove(int moveNum, S_MOVELIST *list) {

	S_MOVE temp;
	int index = 0;
	int bestScore = -INF_BOUND;
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
*/

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
			pos->searchHistory[index][index2] = 40000;
		}
	}

	for(int index = 0; index < 2; ++index) {
		for(int index2 = 0; index2 < MAX_DEPTH; ++index2) {
			pos->searchKillers[index][index2] = 40000;
		}
	}

	table->overWrite = 0;
	table->hit = 0;
	table->cut = 0;
	table->currentAge++;
	pos->ply = 0;

	info->stopped = FALSE;
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

	if(IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if(pos->ply >= MAX_DEPTH) {
		return EvalPosition(pos);
	}

	int32_t Score = 0; // Flagged "Conditional jump or move depends on uninitialised value(s)" by Valgrind
	Score = EvalPosition(pos); // stand-pat score

	ASSERT(Score > -INF_BOUND && Score < INF_BOUND);

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
    GenerateAllCaps(pos, list);

    int MoveNum = 0;
	int Legal = 0;
	Score = -INF_BOUND;
	
	// Delta pruning buffer.
	#define DELTA_BUFFER 180

	sort_move_list(list, list->count);

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// PickNextMove(MoveNum, list); // finds highest-scoring move between MoveNum index and count-1 index

		// Delta pruning (general case)
		// int mask = 0xFFFFFFFF & (0b1111 << 14);
		// int capturedPiece = (curr_move & mask) >> 14;
		int capturedPiece = CAPTURED(list->moves[MoveNum].move);
		int delta = PieceValMg[capturedPiece]; // mg values
		// If the gain from capturing a piece is too low (not enough to improve alpha), we skip make/undo moves and evalaution
		// buffer: 180
		if (delta + DELTA_BUFFER <= alpha) {
			return alpha;
		}
		 
		// Check if it's a legal move
        if ( !MakeMove(pos, list->moves[MoveNum].move) )  {
            continue;
        }
		info->nodes++;
		Legal++;

		Score = -Quiescence(-beta, -alpha, pos, info);
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

	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	// Max depth reached
	if(pos->ply >= MAX_DEPTH) {
		return EvalPosition(pos);
	}

	// Move category
	uint8_t InCheck = SqAttacked(pos->KingSq[pos->side],!pos->side,pos);

	// Check extension to avoid horizon effect
	if(InCheck == TRUE) {
		depth++;
	}

	int Score = -INF_BOUND;
	int PvMove = NOMOVE;

	if( ProbeHashEntry(pos, table, &PvMove, &Score, alpha, beta, depth) == TRUE ) {
		table->cut++;
		return Score;
	}

	/*
		Null-move Pruning
	*/
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
    GenerateAllMoves(pos, list); // MVV-LVA included here (see movegen.c)

	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NOMOVE;

	int BestScore = -INF_BOUND;
	Score = -INF_BOUND;

	// Move ordering for PV moves
	if (PvMove != NOMOVE) {
		for(int MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if (list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				break;
			}
		}
	}

	sort_move_list(list, list->count);

	for(int MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// PickNextMove(MoveNum, list);
		int curr_move = list->moves[MoveNum].move;

		/*
			(Extended) Futility Pruning
		*/
		// Discards moves which have no potential of raising alpha

		// Depth 1 margin: ~minor piece
		#define FUTILITY_MARGIN 325
		// Depth 2 margin: ~rook
		#define EXTENDED_FUTILITY_MARGIN 525
	
		// Move classifications
		uint8_t InCheck = SqAttacked(pos->KingSq[pos->side], !pos->side, pos);
		uint8_t IsCheck = SqAttacked(pos->KingSq[!pos->side], pos->side, pos);
		int IsCapture = CAPTURED(curr_move);

		// We check if it is a frontier node (1/2 ply from horizon) and the eval is not very high
		int static_score = EvalPosition(pos);
		if ( ( (depth == 1) || (depth == 2) ) && (abs(static_score) < 1200) ) {

			// Check to make sure it's not a capture or a check
			if (!IsCapture && !IsCheck) {
				if ( (depth == 1) && (static_score + FUTILITY_MARGIN <= alpha) ) {
					continue;
				} else if ( (depth == 2) && (static_score + EXTENDED_FUTILITY_MARGIN <= alpha) ) {
					continue;
				}
			}
		} 
		
		// Check if it's a legal move
		// The move will be made for the rest of the code if it is
        if ( !MakeMove(pos, curr_move) )  {
            continue;
        }
		Legal++;
		info->nodes++;

		/*
			Late Move Reductions
		*/ 
        // We calculate less promising moves at lower depths

        int reduced_depth = depth - 1; // We move further into the tree
        // Do not reduce if it's completely winning / near mating position 
        if (abs(static_score) < 1200) {

            // Check if it's a late move
            if (MoveNum > 3 && depth > 4) {

                uint8_t self_king_sq = pos->KingSq[pos->side];
                uint8_t moving_pce = pos->pieces[FROMSQ(curr_move)];
                uint8_t target_sq = TOSQ(curr_move);

                int IsPromotion = PROMOTED(curr_move);
                // uint8_t MoveIsAttack = IsAttack(moving_pce, target_sq, pos);
                // Checks if a move's target square is within 3 king moves
                uint8_t target_sq_within_king_zone = dist_between_squares(self_king_sq, target_sq) <= 3; 

                if (!IsCapture && !IsPromotion && !InCheck && !IsCheck && !IsPawn(moving_pce) && !target_sq_within_king_zone) {
                    reduced_depth = (int)( log(depth) * log(MoveNum) / 2.25 );
					// reduced_depth = max(reduced_depth, 3);
					reduced_depth = max(reduced_depth, max(4, depth - 4));
                }
            }
        }

		Score = -AlphaBeta(-beta, -alpha, reduced_depth, pos, table, info, TRUE);
		TakeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}

		// Update bestScore, bestMove and killers
		if(Score > BestScore) {
			BestScore = Score;
			BestMove = curr_move;
			if(Score > alpha) {
				if(Score >= beta) {
					if(Legal == 1) {
						info->fhf++;
					}
					info->fh++;

					if(!(curr_move & MFLAGCAP)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = curr_move;
					}

					StoreHashEntry(pos, table, BestMove, beta, HFBETA, depth);

					return beta;
				}
				alpha = Score;

				if(!(curr_move & MFLAGCAP)) {
					pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
				}
			}
		}
    }

	// If there's no legal move it's either checkmate or stalemate
	if (Legal == 0) {
		if(InCheck) {
			return -INF_BOUND + pos->ply;
		} else {
			return 0;
		}
	}

	ASSERT(alpha >= OldAlpha);

	if(alpha != OldAlpha) {
		StoreHashEntry(pos, table, BestMove, BestScore, HFEXACT, depth);
	} else {
		StoreHashEntry(pos, table, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

/*******************************
*** Iterative Deepening Loop ***
*******************************/

void SearchPosition(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info) {

	int bestMove = NOMOVE;
	int bestScore = -INF_BOUND;
	int pvMoves = 0;
	int pvNum = 0;

	// Aspiration windows variables
	uint8_t window_size = 50; // Size for first 6 depths
	int guess = -INF_BOUND;
	int alpha = -INF_BOUND;
	int beta = INF_BOUND;

	ClearForSearch(pos, table, info); // Initialise searchHistory and killers
	
	// Get moves from opening book
	if (EngineOptions->UseBook == TRUE) {
		bestMove = GetBookMove(pos);
	}

	if (bestMove == NOMOVE) {

		for (int currentDepth = 1; currentDepth <= info->depth; ++currentDepth) {

			// Do a full search on the first depth
			if (currentDepth == 1) {
				bestScore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, pos, table, info, TRUE);
			} 
			else {
				
				// Aspiration windows
				if (currentDepth > 6) {
					// Window size decreases linearly with depth, with a minimum value of 25
					window_size = max(-2.5 * currentDepth + 65, 25);
				}
				alpha = guess - window_size;
				beta = guess + window_size;

				uint8_t reSearch = TRUE;
				while (reSearch) {
					bestScore = AlphaBeta(alpha, beta, currentDepth, pos, table, info, TRUE);

					// Re-search with a full window if fail-low or fail-high
					if (bestScore <= alpha || bestScore >= beta) {
						alpha = -INF_BOUND;
						beta = INF_BOUND;
					} else {
						// Successful search, exit re-search loop
						reSearch = FALSE;
					}
            	}
			}
            
			guess = bestScore;

			if(info->stopped == TRUE) {
				break;
			}

			pvMoves = GetPvLine(currentDepth, pos, table);
			bestMove = pos->PvArray[0];

			// Display mate if there's forced mate
			unsigned long long time = GetTimeMs() - info->starttime;
			uint8_t mate_found = FALSE; // Save computation
			int8_t mate_moves = 0;
			if (abs(bestScore) >= ISMATE) {
				mate_found = TRUE;
				// copysign(1.0, value) outputs +/- 1.0 depending on the sign of "value" (i.e. sgn(value))
				// Note that /2 is integer division (e.g. 3/2 = 1)
				mate_moves = round( (INF_BOUND - abs(bestScore) - 1) / 2 + 1) * copysign(1.0, bestScore);
				printf("info score mate %d depth %d nodes %ld hashfull %d time %llu pv",
					mate_moves, currentDepth, info->nodes, (int)(table->numEntries / (double)table->maxEntries * 1000), time);
			} else {
				printf("info score cp %d depth %d nodes %ld hashfull %d time %llu pv",
					bestScore, currentDepth, info->nodes, (int)(table->numEntries / (double)table->maxEntries * 1000), time);
			}
			

			// Print PV
			for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
				printf(" %s",PrMove(pos->PvArray[pvNum]));
			}
			printf("\n");

			// Exit search if mate at current depth is found, in order to save time
			if ( mate_found && ( currentDepth >= (abs(mate_moves) + 1) ) ) {
				break;
				// Buggy if no search is performed before pruning immediately
			}
		}
	}

	printf("bestmove %s\n", PrMove(bestMove));

}