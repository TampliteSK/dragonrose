#ifndef DEFS_H
#define DEFS_H

/*******************
** System Headers **
*******************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/*******************
*** Definitions ****
*******************/

// Uncomment to activate ASSERT() statements
// #define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

typedef unsigned long long U64;

#define NAME "Dragonrose 0.28"
// 1: TRUE, 0: FALSE
#define OPENBENCH_MODE 1

/*
	MAX_HASH: Maximum hash size - 65536 MB (64 GB), or 2,863,311,530 positions
	MAX_GAME_MOVES: Maximum no. of moves in a game
	MAX_POSITION_MOVES: Maximum expected legal moves in a given position
		Note: There is a position that breaks the conventional 256 limit (credits to Caisssa and Quanticade):
		QQQQQQBk/Q6B/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1 (265 moves)
*/

#define BRD_SQ_NUM 120
#define MAX_HASH 65536
#define MAX_GAME_MOVES 2048
#define MAX_DEPTH 64
#define MAX_POSITION_MOVES 280

#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define INF_BOUND 30000
#define ISMATE (INF_BOUND - MAX_DEPTH)

/*******************
****** Enums *******
*******************/

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK  };
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };

enum { WHITE, BLACK, BOTH };
enum {
  A1 = 21, B1, C1, D1, E1, F1, G1, H1,
  A2 = 31, B2, C2, D2, E2, F2, G2, H2,
  A3 = 41, B3, C3, D3, E3, F3, G3, H3,
  A4 = 51, B4, C4, D4, E4, F4, G4, H4,
  A5 = 61, B5, C5, D5, E5, F5, G5, H5,
  A6 = 71, B6, C6, D6, E6, F6, G6, H6,
  A7 = 81, B7, C7, D7, E7, F7, G7, H7,
  A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

enum { FALSE, TRUE };

enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };

/*******************
***** Structs ******
*******************/

// Move struct. See below for format of move
typedef struct {
	int move;
	int score; // goes up to 2,000,000 for hash moves move ordering
} S_MOVE;

typedef struct {
	S_MOVE moves[MAX_POSITION_MOVES]; // [280]
	int count;
} S_MOVELIST;

// Move flag for hash entry
enum { HFNONE, HFALPHA, HFBETA, HFEXACT };

// 18 bytes (will be padded to 24)
typedef struct {
	U64 posKey;
	int move;
	int16_t score;
	uint8_t depth;
	uint8_t flags;
	uint16_t age; // indicates how new an entry is
} S_HASHENTRY;

typedef struct {
	S_HASHENTRY *pTable;
	int maxEntries; // maximum entries based on given hash size
	int numEntries; // number of entries at any given time
	int newWrite;
	int overWrite;
	int hit; // tracks the number of entires probed
	int cut; // max number of probes allowed before hash table is full (to avoid collision of entries)
	int currentAge; // increments every move
} S_HASHTABLE;

typedef struct {
	int move;
	uint8_t castlePerm;
	uint8_t enPas; // en passant square
	int fiftyMove;
	U64 posKey;
} S_UNDO;

typedef struct {

	int pieces[BRD_SQ_NUM];
	U64 pawns[3];
	U64 occupancy[3]; // bitboard storing every piece (inc. pawns)
				   // used for generating attacks in combination with pre-generated attack boards

	// piece list
	int pList[13][10]; // [pieceType][max no of one piece]. defaulted to NO_SQ
  // usage eg.: pList[wN][0] = e1; for the position of 1st knight

	int pceNum[13];
	int bigPce[2];
	int majPce[2];
	int minPce[2];
	// moved material inside evaluate to save time
	// int material[2];

	int KingSq[2];
	uint8_t side;
	uint8_t enPas;
	uint8_t fiftyMove;
	uint8_t castlePerm;

	uint16_t ply;
	uint16_t hisPly;

	S_UNDO history[MAX_GAME_MOVES];
	int PvArray[MAX_DEPTH];

	int searchHistory[13][BRD_SQ_NUM];
	int searchKillers[2][MAX_DEPTH];

	U64 posKey;

} S_BOARD;

typedef struct {

	unsigned long long starttime;
	unsigned long long stoptime;
	uint8_t depth;
	int timeset;
	uint16_t movestogo;

	unsigned long nodes;

	uint8_t quit;
	uint8_t stopped;

	float fh; // beta cutoffs
	float fhf; // legal moves
	uint16_t nullCut;

} S_SEARCHINFO;

// UCI options struct
typedef struct {
	uint8_t UseBook; // check, TRUE / FALSE
} S_OPTIONS;

/* GAME MOVE */

/*
0000 0000 0000 0000 0000 0111 1111 -> From 0x7F (7 bits)
0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F (7 bits)
0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF (4 bits)
0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000 (1 bit)
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000 (1 bit)
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF (4 bits)
0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000 (1 bit)

Mask to get captured:
0000 0000 0011 1111 1111 1111 1111
0x3FFFF
*/

// Macros for obtaining info from move
#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

#define MFLAGEP 0x40000
#define MFLAGPS 0x80000

// Piece captured
#define MFLAGCA 0x1000000
// isCapture (captured + en passant)
#define MFLAGCAP 0x7C000
// isPromotion
#define MFLAGPROM 0xF00000

#define NOMOVE 0


/*******************
****** Macros ******
*******************/

#define FR2SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) )
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])
#define POP(b) PopBit(b)
#define CNT(b) CountBits(b)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])

#define IsPawn(p) (PiecePawn[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsBishop(p) (PieceBishop[(p)])
#define IsRook(p) (PieceRook[(p)])
#define IsQueen(p) (PieceQueen[(p)])
#define IsKi(p) (PieceKing[(p)])

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])

#define MIRROR64(sq) (Mirror64[(sq)])

#define min(x, y) ((x) > (y) ? (y) : (x))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define clamp(low, value, high) ((value) < (low) ? (low) : ((value) > (high) ? (high) : (value)))

/*******************
***** Globals ******
*******************/

// attack.c
extern U64 bishop_masks[64];
extern U64 rook_masks[64];
extern U64 bishop_attacks[64][512];
extern U64 rook_attacks[64][4096];

// bitboard.c
extern U64 SetMask[64];
extern U64 ClearMask[64];

// init.c
extern int Sq120ToSq64[BRD_SQ_NUM];
extern int Sq64ToSq120[64];
extern int FilesBrd[BRD_SQ_NUM];
extern int RanksBrd[BRD_SQ_NUM];

// hashkeys.c
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];
extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

// board.c, data.c
extern int PieceBig[13];
extern int PieceMaj[13];
extern int PieceMin[13];

extern int PieceValMg[13];
extern int PieceValEg[13];
extern int PieceCol[13];
extern int PiecePawn[13];

extern int PiecePawn[13];
extern int PieceKnight[13];
extern int PieceBishop[13];
extern int PieceRook[13];
extern int PieceQueen[13];
extern int PieceKing[13];

extern int PieceRookQueen[13];
extern int PieceBishopQueen[13];
extern int PieceRBN[13];
extern int PieceSlides[13];

extern int Mirror64[64];

extern U64 FileBBMask[8];
extern U64 RankBBMask[8];

extern U64 BlackPassedMask[64];
extern U64 WhitePassedMask[64];
extern U64 IsolatedMask[64];

// main.c, init.c, uci.c, search.c, polybook.c,
extern S_OPTIONS EngineOptions[1];
extern S_HASHTABLE HashTable[1]; // brought out from board struct to make it global (to make Lazy SMP work)

/*******************
***** Functions ****
*******************/

// attack.c
extern uint8_t SqAttacked(const int sq, const int side, const S_BOARD *pos);
extern uint8_t SqAttackedS(const int sq, const int side, const S_BOARD *pos);
extern uint8_t IsAttack(const int pce, const int sq, const S_BOARD *pos);
extern uint16_t SqAttackedByWho(const int sq, const int side, const S_BOARD *pos);

// extern U64 mask_bishop_attacks(uint8_t sq64);
// extern U64 mask_rook_attacks(uint8_t sq64);
extern U64 get_bishop_attacks(uint8_t sq, U64 occupancy);
extern U64 get_rook_attacks(uint8_t sq, U64 occupancy);
extern U64 get_queen_attacks(uint8_t sq, U64 occupancy);
extern void init_attack_tables();

// bitboards.c
extern void PrintBitBoard(U64 bb);
extern int PopBit(U64 *bb);
extern int CountBits(U64 b);
extern U64 flip_bitboard(U64 bb);

// board.c
extern void ResetBoard(S_BOARD *pos);
extern int ParseFen(char *fen, S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern void UpdateListsMaterial(S_BOARD *pos);
extern int CheckBoard(const S_BOARD *pos);
extern void MirrorBoard(S_BOARD *pos);

// endgame.c - Used in evaluate.c
extern uint8_t is_material_draw(const S_BOARD *pos, int net_material);

// evaluate.c
// extern U64 generate_king_zone(uint8_t kingSq);
// extern U64 generate_shield_zone(uint8_t kingSq, uint8_t col);
// extern int16_t attack_units(const S_BOARD *pos, uint8_t col);
extern double evalWeight(const S_BOARD *pos);
// extern int8_t pawn_storm(const S_BOARD *pos, uint8_t kingSq, uint8_t col);
extern int16_t EvalPosition(const S_BOARD *pos);
extern void MirrorEvalTest(S_BOARD *pos);

// hashkeys.c
extern U64 GeneratePosKey(const S_BOARD *pos);

// init.c
extern void AllInit();

// io.c
extern char *PrMove(const int move);
extern char *PrSq(const int sq);
extern void PrintMoveList(const S_MOVELIST *list);
extern int ParseMove(char *ptrChar, S_BOARD *pos);

// makemove.c
extern int MakeMove(S_BOARD *pos, int move);
extern void TakeMove(S_BOARD *pos);
extern void MakeNullMove(S_BOARD *pos);
extern void TakeNullMove(S_BOARD *pos);

// misc.c
extern uint64_t GetTimeMs();
extern uint8_t isLightSq(uint8_t sq);
extern uint8_t on_same_diagonal(uint8_t sq_1, uint8_t sq_2);
extern uint8_t isOppColBishops(const S_BOARD *pos);
extern uint8_t dist_between_squares(uint8_t sq_1, uint8_t sq_2);
extern int8_t max_between_squares(uint8_t sq_1, uint8_t sq_2);

// movegen.c
extern void GenerateSliders(const S_BOARD *pos, S_MOVELIST *list);
extern void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
extern int MoveExists(S_BOARD *pos, const int move);
extern void InitMvvLva();

// perft.c
extern void PerftTest(int depth, S_BOARD *pos);

// search.c
extern void SearchPosition(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info);

// polybook.c
extern int GetBookMove(S_BOARD *board);
extern void CleanPolyBook();
extern void InitPolyBook();

// pvtable.c
extern void InitHashTable(S_HASHTABLE *table, const int MB);
extern void StoreHashEntry(S_BOARD *pos, S_HASHTABLE *table, const int move, int score, const int flags, const int depth);
extern int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE *table, int *move, int *score, int alpha, int beta, int depth);
extern int ProbePvMove(const S_BOARD *pos, const S_HASHTABLE *table);
extern int GetPvLine(const int depth, S_BOARD *pos, const S_HASHTABLE *table);
extern void ClearHashTable(S_HASHTABLE *table);

// uci.c
extern void ParseGo(char* line, S_SEARCHINFO *info, S_BOARD *pos, S_HASHTABLE *table);
extern void ParsePosition(char* lineIn, S_BOARD *pos);
extern void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info);

// validate.c
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceValidEmpty(const int pce);
extern int PieceValid(const int pce);
extern void MirrorEvalTest(S_BOARD *pos);
extern int SqIs120(const int sq);
extern int PceValidEmptyOffbrd(const int pce);
extern int MoveListOk(const S_MOVELIST *list,  const S_BOARD *pos);
extern void DebugAnalysisTest(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info);

#endif