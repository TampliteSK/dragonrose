// data.c
#include "defs.h"

char PceChar[] = ".PNBRQKpnbrqk";
char SideChar[] = "wb-";
char RankChar[] = "12345678";
char FileChar[] = "abcdefgh";

// defs.h, boards.c
int PieceBig[13] = { FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE };
int PieceMaj[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE };
int PieceMin[13] = { FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE };
int PieceCol[13] = { BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
	BLACK, BLACK, BLACK, BLACK, BLACK, BLACK };

// evaluate.c, board.c data
// Values from PesTO's evaluation function
int PieceValMg[13]= { 0, 82, 337, 365, 477, 1025, 50000, 82, 337, 365, 477, 1025, 50000  };
int PieceValEg[13]= { 0, 94, 281, 297, 512, 936, 50000, 94, 281, 297, 512, 936, 50000  };
	

// attack.c data
// enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK  };
// bP = wP + 6
// bN = wN + 6
// etc.
int PiecePawn[13] = { FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE };	
int PieceKnight[13] = { FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE };
int PieceKing[13] = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE };
int PieceRookQueen[13] = { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE };
int PieceBishopQueen[13] = { FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE };
int PieceRBN[13] = { FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE };
int PieceSlides[13] = { FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE };

// evaluate.c
int Mirror64[64] = {
56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
};
	