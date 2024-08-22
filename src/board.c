// board.c

#include <stdio.h>
#include "defs.h"

// Initialises the board
void ResetBoard(S_BOARD *pos) {
	// Sets board to empty, and borders to offboard
  	for(int index = 0; index < BRD_SQ_NUM; ++index) {
		pos->pieces[index] = OFFBOARD;
	}
	for(int index = 0; index < 64; ++index) {
		pos->pieces[SQ120(index)] = EMPTY;
	}

	// Aggregate piece counts
	for(int index = 0; index < 2; ++index) {
		pos->bigPce[index] = 0;
		pos->majPce[index] = 0;
		pos->minPce[index] = 0;
		pos->pawns[index] = 0ULL;
		pos->occupancy[index] = 0ULL;
	}
	pos->pawns[2] = 0ULL;
	pos->occupancy[2] = 0ULL;

	// Piece counts
	for(int index = 0; index < 13; ++index) {
		pos->pceNum[index] = 0;
	}

	// King squares
	pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

	// Misc data
	pos->side = BOTH;
	pos->enPas = NO_SQ;
	pos->fiftyMove = 0;
	pos->castlePerm = 0;

	pos->ply = 0;
	pos->hisPly = 0;

	pos->posKey = 0ULL;

}

int PceListOk(const S_BOARD *pos) {
	int pce = wP;
	int sq;
	int num;
	for(pce = wP; pce <= bK; ++pce) {
		if(pos->pceNum[pce]<0 || pos->pceNum[pce]>=10) return FALSE;
	}

	if(pos->pceNum[wK]!=1 || pos->pceNum[bK]!=1) return FALSE;

	for(pce = wP; pce <= bK; ++pce) {
		for(num = 0; num < pos->pceNum[pce]; ++num) {
			sq = pos->pList[pce][num];
			if(!SqOnBoard(sq)) return FALSE;
		}
	}
    return TRUE;
}

// Checks if the board struct is aligned with the position
// Returns 1 if success. 0 if ASSERT fails.
int CheckBoard(const S_BOARD *pos) {
	// Temp board
	int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int t_bigPce[2] = { 0, 0};
	int t_majPce[2] = { 0, 0};
	int t_minPce[2] = { 0, 0};

	int sq64,t_piece,t_pce_num,sq120,colour,pcount;

	U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};
	U64 t_occupancy[3] = {0ULL, 0ULL, 0ULL};

	t_pawns[WHITE] = pos->pawns[WHITE];
	t_pawns[BLACK] = pos->pawns[BLACK];
	t_pawns[BOTH] = pos->pawns[BOTH];
	t_occupancy[WHITE] = pos->occupancy[WHITE];
	t_occupancy[BLACK] = pos->occupancy[BLACK];
	t_occupancy[BOTH] = pos->occupancy[BOTH];

	// check piece lists
	for(t_piece = wP; t_piece <= bK; ++t_piece) {
		for(t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; ++t_pce_num) {
			sq120 = pos->pList[t_piece][t_pce_num];
			ASSERT(pos->pieces[sq120]==t_piece);
		}
	}

	// check piece count and other counters
	for(sq64 = 0; sq64 < 64; ++sq64) {
		sq120 = SQ120(sq64);
		t_piece = pos->pieces[sq120];
		t_pceNum[t_piece]++;
		colour = PieceCol[t_piece];
		if( PieceBig[t_piece] == TRUE) t_bigPce[colour]++;
		if( PieceMin[t_piece] == TRUE) t_minPce[colour]++;
		if( PieceMaj[t_piece] == TRUE) t_majPce[colour]++;
	}

	for(t_piece = wP; t_piece <= bK; ++t_piece) {
		ASSERT(t_pceNum[t_piece]==pos->pceNum[t_piece]);
	}

	// Check bitboards count
	pcount = CNT(t_pawns[WHITE]);
	ASSERT(pcount == pos->pceNum[wP]);
	pcount = CNT(t_pawns[BLACK]);
	ASSERT(pcount == pos->pceNum[bP]);
	pcount = CNT(t_pawns[BOTH]);
	ASSERT(pcount == (pos->pceNum[bP] + pos->pceNum[wP]));

	// Check pawns and occupancy bitboards
	while(t_pawns[WHITE]) {
		sq64 = POP(&t_pawns[WHITE]);
		ASSERT(pos->pieces[SQ120(sq64)] == wP);
	}

	while(t_pawns[BLACK]) {
		sq64 = POP(&t_pawns[BLACK]);
		ASSERT(pos->pieces[SQ120(sq64)] == bP);
	}

	while(t_pawns[BOTH]) {
		sq64 = POP(&t_pawns[BOTH]);
		ASSERT( (pos->pieces[SQ120(sq64)] == bP) || (pos->pieces[SQ120(sq64)] == wP) );
	}

	while(t_occupancy[WHITE]) {
		sq64 = POP(&t_occupancy[WHITE]);
		ASSERT(pos->pieces[SQ120(sq64)] != EMPTY);
		ASSERT(PieceCol[ pos->pieces[SQ120(sq64)] ] == WHITE);
	}

	while(t_occupancy[BLACK]) {
		sq64 = POP(&t_occupancy[BLACK]);
		ASSERT(pos->pieces[SQ120(sq64)] != EMPTY);
		ASSERT(PieceCol[ pos->pieces[SQ120(sq64)] ] == BLACK);
	}

	while(t_occupancy[BOTH]) {
		sq64 = POP(&t_occupancy[BOTH]);
		ASSERT(pos->pieces[SQ120(sq64)] != EMPTY);
	}

	// Check material
	ASSERT(t_minPce[WHITE]==pos->minPce[WHITE] && t_minPce[BLACK]==pos->minPce[BLACK]);
	ASSERT(t_majPce[WHITE]==pos->majPce[WHITE] && t_majPce[BLACK]==pos->majPce[BLACK]);
	ASSERT(t_bigPce[WHITE]==pos->bigPce[WHITE] && t_bigPce[BLACK]==pos->bigPce[BLACK]);

	// Check sides, en passant, king square and castlePerms
	ASSERT(pos->side==WHITE || pos->side==BLACK);
	ASSERT(GeneratePosKey(pos)==pos->posKey);

	// En passant sq must either not exist, or 6th/3rd rank
	ASSERT(pos->enPas==NO_SQ || ( RanksBrd[pos->enPas]==RANK_6 && pos->side == WHITE)
		 || ( RanksBrd[pos->enPas]==RANK_3 && pos->side == BLACK));

	ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK);
	ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK);

	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);

	ASSERT(PceListOk(pos));

	return TRUE;
}

// Updates board struct as per position
void UpdateListsMaterial(S_BOARD *pos) {

	int piece, sq, colour;

	for(int sq = 0; sq < BRD_SQ_NUM; ++sq) {
		piece = pos->pieces[sq];
		ASSERT(PceValidEmptyOffbrd(piece));
		if(piece != OFFBOARD && piece != EMPTY) {
			colour = PieceCol[piece];
			ASSERT(SideValid(colour));

		    if( PieceBig[piece] == TRUE) pos->bigPce[colour]++;
		    if( PieceMin[piece] == TRUE) pos->minPce[colour]++;
		    if( PieceMaj[piece] == TRUE) pos->majPce[colour]++;

			ASSERT(pos->pceNum[piece] < 10 && pos->pceNum[piece] >= 0);
			
			// Positions of the piece [pieceType] [no. of the piece] = the square
			pos->pList[piece][pos->pceNum[piece]] = sq;
			pos->pceNum[piece]++; // tracks no of piece / pointer of pList

			// Setting king square
			if(piece == wK) pos->KingSq[WHITE] = sq;
			if(piece == bK) pos->KingSq[BLACK] = sq;

			// Setting pawn bitboards
			if(piece == wP) {
				SETBIT(pos->pawns[WHITE], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			} else if(piece == bP) {
				SETBIT(pos->pawns[BLACK], SQ64(sq));
				SETBIT(pos->pawns[BOTH], SQ64(sq));
			}

			// Setting occupancy bitboards
			pos->occupancy[BOTH] |= (1ULL << SQ64(sq));
			if (PieceCol[piece] == WHITE) {
				pos->occupancy[WHITE] |= (1ULL << SQ64(sq));
			} else {
				pos->occupancy[BLACK] |= (1ULL << SQ64(sq));
			}

		}
	}
}

// Returns -1 if error, 0 otherwise.
int ParseFen(char *fen, S_BOARD *pos) {
	
	// To prevent crashes
	ASSERT(fen!=NULL);
	ASSERT(pos!=NULL);

	/********************
  	**  Parsing Pieces **
  	****************** */

	int  rank = RANK_8;
    int  file = FILE_A;
    int  piece = 0;
    int  count = 0; // no. of consecutive empty squares / placeholder
	int  sq64 = 0;
	int  sq120 = 0;

	ResetBoard(pos);

	while ((rank >= RANK_1) && *fen) {
	    count = 1;
		switch (*fen) {
            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0'; // number of consecutive empty squares
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("FEN error \n");
                return -1;
        }

		// PUtting pieces on the board
		for (int i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
			sq120 = SQ120(sq64);
            if (piece != EMPTY) { // Skips a file if empty square
                pos->pieces[sq120] = piece;
            }
			file++;
        }
		fen++;
	}

	/********************
  	* Parsing Misc Data *
	****************** */

	// Side-to-move parsing
	ASSERT(*fen == 'w' || *fen == 'b');
	pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	// Castling perm parsing
	for (int i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
		switch(*fen) {
			case 'K': pos->castlePerm |= WKCA; break;
			case 'Q': pos->castlePerm |= WQCA; break;
			case 'k': pos->castlePerm |= BKCA; break;
			case 'q': pos->castlePerm |= BQCA; break;
			default:	     break;
        }
		fen++;
	}
	fen++;
	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);

	// En passant parsing
	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file>=FILE_A && file <= FILE_H);
		ASSERT(rank>=RANK_1 && rank <= RANK_8);

		pos->enPas = FR2SQ(file,rank);
    }

	// Zobrist key
	pos->posKey = GeneratePosKey(pos);

	UpdateListsMaterial(pos);

	return 0;
}

// Prints the board out
void PrintBoard(const S_BOARD *pos) {
	printf("\nGame Board:\n\n");

	for(int rank = RANK_8; rank >= RANK_1; rank--) {
		printf("%d  ",rank+1);
		for(int file = FILE_A; file <= FILE_H; file++) {
			int sq = FR2SQ(file,rank);
			int piece = pos->pieces[sq];
			printf("%3c",PceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for(int file = FILE_A; file <= FILE_H; file++) {
		printf("%3c",'a'+file);
	}
	printf("\n");
	printf("side:%c\n",SideChar[pos->side]);
	printf("enPas:%d\n",pos->enPas);
	printf("castle:%c%c%c%c\n",
			pos->castlePerm & WKCA ? 'K' : '-',
			pos->castlePerm & WQCA ? 'Q' : '-',
			pos->castlePerm & BKCA ? 'k' : '-',
			pos->castlePerm & BQCA ? 'q' : '-'
			);
	printf("PosKey:%llX\n",pos->posKey);
}

// Mirrors the board
void MirrorBoard(S_BOARD *pos) {

    int tempPiecesArray[64];
    int tempSide = pos->side^1;
	int SwapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
    int tempCastlePerm = 0;
    int tempEnPas = NO_SQ;

	int sq;
	int tp;

    if (pos->castlePerm & WKCA) tempCastlePerm |= BKCA;
    if (pos->castlePerm & WQCA) tempCastlePerm |= BQCA;

    if (pos->castlePerm & BKCA) tempCastlePerm |= WKCA;
    if (pos->castlePerm & BQCA) tempCastlePerm |= WQCA;

	if (pos->enPas != NO_SQ)  {
        tempEnPas = SQ120(Mirror64[SQ64(pos->enPas)]);
    }

    for (sq = 0; sq < 64; sq++) {
        tempPiecesArray[sq] = pos->pieces[SQ120(Mirror64[sq])];
    }

    ResetBoard(pos);

	for (sq = 0; sq < 64; sq++) {
        tp = SwapPiece[tempPiecesArray[sq]];
        pos->pieces[SQ120(sq)] = tp;
    }

	pos->side = tempSide;
    pos->castlePerm = tempCastlePerm;
    pos->enPas = tempEnPas;

    pos->posKey = GeneratePosKey(pos);

	UpdateListsMaterial(pos);

    ASSERT(CheckBoard(pos));
}
