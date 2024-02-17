// attack.h
#ifndef ATTACK_H
#define ATTACK_H

extern int KiDir[8];
extern int KnDir[8];
extern int RkDir[4];
extern int BiDir[4]; 

/*
	Magic Bitboard / Attackgen Constants from BBC / Alexandria
*/

// Not A/H/AB/HG file constants
extern U64 NOT_A_FILE;
extern U64 NOT_H_FILE;
extern U64 NOT_HG_FILE;
extern U64 NOT_AB_FILE;

// Relevant occupancy bit counts for bishops and rooks
// Relevant occupancy bits: Pieces on these squares can block the bishop/rook's vision
// The array simply gives the number of such squares for each given bishop/rook location
extern int bishop_relevant_bits[64];
extern int rook_relevant_bits[64];

// Magic numbers for rooks and bishops
extern U64 rook_magic_numbers[64];
extern U64 bishop_magic_numbers[64];

// They are defined in data.c
#endif