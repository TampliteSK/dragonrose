// init.c

#include "defs.h"
#include "stdio.h"
#include "stdlib.h"

// Randomisation is separated as rand() is only 4 bytes
#define RAND_64 	((U64)rand() | \
					(U64)rand() << 15 | \
					(U64)rand() << 30 | \
					(U64)rand() << 45 | \
					((U64)rand() & 0xf) << 60 )

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

U64 FileBBMask[8];
U64 RankBBMask[8];

U64 BlackPassedMask[64];
U64 WhitePassedMask[64];
U64 IsolatedMask[64];

S_OPTIONS EngineOptions[1];

void InitEvalMasks() {

	int sq, tsq, r, f;

	for(sq = 0; sq < 8; ++sq) {
        FileBBMask[sq] = 0ULL;
		RankBBMask[sq] = 0ULL;
	}

	for(r = RANK_8; r >= RANK_1; r--) {
        for (f = FILE_A; f <= FILE_H; f++) {
            sq = r * 8 + f;
            FileBBMask[f] |= (1ULL << sq);
            RankBBMask[r] |= (1ULL << sq);
        }
	}

	for(sq = 0; sq < 64; ++sq) {
		IsolatedMask[sq] = 0ULL;
		WhitePassedMask[sq] = 0ULL;
		BlackPassedMask[sq] = 0ULL;
    }

	for(sq = 0; sq < 64; ++sq) {
		tsq = sq + 8;

        while(tsq < 64) {
            WhitePassedMask[sq] |= (1ULL << tsq);
            tsq += 8;
        }

        tsq = sq - 8;
        while(tsq >= 0) {
            BlackPassedMask[sq] |= (1ULL << tsq);
            tsq -= 8;
        }
		
		// Pawn is on B-H files
        if(FilesBrd[SQ120(sq)] > FILE_A) {
			
			// Mask the file to the left of a potential pawn
            IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1];

            tsq = sq + 7;
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 9;
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }

		// Pawn is on A-G files
        if(FilesBrd[SQ120(sq)] < FILE_H) {

			// Mask the file to the right of a potential pawn
            IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];

            tsq = sq + 9;
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 7;
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
	}
}

void InitFilesRanksBrd() {

	for(int index = 0; index < BRD_SQ_NUM; ++index) {
		FilesBrd[index] = OFFBOARD;
		RanksBrd[index] = OFFBOARD;
	}

	for(int rank = RANK_1; rank <= RANK_8; ++rank) {
		for(int file = FILE_A; file <= FILE_H; ++file) {
			int sq = FR2SQ(file,rank);
			FilesBrd[sq] = file;
			RanksBrd[sq] = rank;
		}
	}

}

void InitHashKeys() {

	for(int index = 0; index < 13; ++index) {
		for(int index2 = 0; index2 < 120; ++index2) {
			PieceKeys[index][index2] = RAND_64;
		}
	}
	SideKey = RAND_64;
	for(int index = 0; index < 16; ++index) {
		CastleKeys[index] = RAND_64;
	}

}

void InitBitMasks() {
	int index = 0;

	for(index = 0; index < 64; index++) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for(index = 0; index < 64; index++) {
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index];
	}
}

void InitSq120To64() {

	for(int index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65; // default value if off board
	}

	for(int index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120; // default value if off board
	}
	
	int sq64 = 0;
  	for(int rank = RANK_1; rank <= RANK_8; ++rank) {
    	for(int file = FILE_A; file <= FILE_H; ++file) {
      		int sq = FR2SQ(file,rank);
      		Sq64ToSq120[sq64] = sq;
      		Sq120ToSq64[sq] = sq64;
      		sq64++;
    	}
  	}
	
}

void AllInit() {
	
	// init.c
	InitSq120To64();
	InitFilesRanksBrd();

	// attack.c (High priority)
	init_attack_tables();

	// init.c (cont.)
	InitBitMasks();
	InitEvalMasks();
	InitHashKeys();
	

	// polybook.c
	InitPolyBook();

	// movegen.c
	InitMvvLva();

}