/* $Id: basic.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/basic.h
 *
 * Copyright (C) 2004-2007 Holger Ruckdeschel <holger@hoicher.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
#ifndef BASIC_H
#define BASIC_H

#include "common.h"


/*
 * Basic data types, macros and functions.
 */

typedef int Color;
typedef int Piece;
typedef int Square;

enum colors { NO_COLOR = -1, WHITE, BLACK };
enum pieces { NO_PIECE = -1,
	PAWN, GUARD, ELEPHANT, KNIGHT, CANNON, ROOK, KING };

enum squares {
	NO_SQUARE = -1,
	A0, B0, C0, D0, E0, F0, G0, H0, I0,
	A1, B1, C1, D1, E1, F1, G1, H1, I1,
	A2, B2, C2, D2, E2, F2, G2, H2, I2,
	A3, B3, C3, D3, E3, F3, G3, H3, I3,
	A4, B4, C4, D4, E4, F4, G4, H4, I4,
	A5, B5, C5, D5, E5, F5, G5, H5, I5,
	A6, B6, C6, D6, E6, F6, G6, H6, I6,
	A7, B7, C7, D7, E7, F7, G7, H7, I7,
	A8, B8, C8, D8, E8, F8, G8, H8, I8,
	A9, B9, C9, D9, E9, F9, G9, H9, I9,
};

#define BOARDSIZE 90

enum files { FILEA = 0, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEI };
enum ranks { RANK0 = 0, RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7,
	RANK8, RANK9 };

#define FIL(sq)		((sq) % 9)
#define RNK(sq)		((sq) / 9)
#define SQUARE(rnk,fil)	((rnk)*9 + (fil))
#define XRANK(rnk)	(RANK9-(rnk))
#define XSIDE(side)	(!(side))

extern const char piece_char[7];
extern const char file_char[9];
extern const char rank_char[10];
extern const char square_str[90][3];

static const int mat_values[7] = {
	100,	// PAWN
	200,	// GUARD
	200, 	// ELEPHANT
	400,	// KNIGHT
	450,	// CANNON
	900,	// ROOK
	0	// KING
};

/* Manhattan distance between two squares. */
static inline int sq_distance(Square sq1, Square sq2)
{
	return abs(RNK(sq1)-RNK(sq2)) + abs(FIL(sq1)-FIL(sq2));
}

/* FEN of standard opening position. */
std::string opening_fen();

/*
 * Initialization function for basic stuff.
 */

extern void basic_init();

#endif // BASIC_H
