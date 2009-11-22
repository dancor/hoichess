/* $Id: board_attack.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/board_attack.cc
 *
 * Copyright (C) 2004, 2005 Holger Ruckdeschel <holger@hoicher.de>
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

#include "common.h"
#include "board.h"
#include "basic.h"

static Square map[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, A0, B0, C0, D0, E0, F0, G0, H0, I0, -1, -1,
	-1, -1, A1, B1, C1, D1, E1, F1, G1, H1, I1, -1, -1,
	-1, -1, A2, B2, C2, D2, E2, F2, G2, H2, I2, -1, -1,
	-1, -1, A3, B3, C3, D3, E3, F3, G3, H3, I3, -1, -1,
	-1, -1, A4, B4, C4, D4, E4, F4, G4, H4, I4, -1, -1,
	-1, -1, A5, B5, C5, D5, E5, F5, G5, H5, I5, -1, -1,
	-1, -1, A6, B6, C6, D6, E6, F6, G6, H6, I6, -1, -1,
	-1, -1, A7, B7, C7, D7, E7, F7, G7, H7, I7, -1, -1,
	-1, -1, A8, B8, C8, D8, E8, F8, G8, H8, I8, -1, -1,
	-1, -1, A9, B9, C9, D9, E9, F9, G9, H9, I9, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static int invmap[] = {
	 28,  29,  30,  31,  32,  33,  34,  35,  36,
	 41,  42,  43,  44,  45,  46,  47,  48,  49,
	 54,  55,  56,  57,  58,  59,  60,  61,  62,
	 67,  68,  69,  70,  71,  72,  73,  74,  75,
	 80,  81,  82,  83,  84,  85,  86,  87,  88,
	 93,  94,  95,  96,  97,  98,  99, 100, 101,
	106, 107, 108, 109, 110, 111, 112, 113, 114,
	119, 120, 121, 122, 123, 124, 125, 126, 127,
	132, 133, 134, 135, 136, 137, 138, 139, 140,
	145, 146, 147, 148, 149, 150, 151, 152, 153,
};

static Square map_palace[] = {
 	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, D0, E0, F0, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, D1, E1, F1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, D2, E2, F2, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, D7, E7, F7, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, D8, E8, F8, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, D9, E9, F9, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static Square map_whitehalf[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, A0, B0, C0, D0, E0, F0, G0, H0, I0, -1, -1,
	-1, -1, A1, B1, C1, D1, E1, F1, G1, H1, I1, -1, -1,
	-1, -1, A2, B2, C2, D2, E2, F2, G2, H2, I2, -1, -1,
	-1, -1, A3, B3, C3, D3, E3, F3, G3, H3, I3, -1, -1,
	-1, -1, A4, B4, C4, D4, E4, F4, G4, H4, I4, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static Square map_blackhalf[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, A5, B5, C5, D5, E5, F5, G5, H5, I5, -1, -1,
	-1, -1, A6, B6, C6, D6, E6, F6, G6, H6, I6, -1, -1,
	-1, -1, A7, B7, C7, D7, E7, F7, G7, H7, I7, -1, -1,
	-1, -1, A8, B8, C8, D8, E8, F8, G8, H8, I8, -1, -1,
	-1, -1, A9, B9, C9, D9, E9, F9, G9, H9, I9, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};


static int dir_guard[] = { +12, +14, -12, -14 };

static int dir_elephant[]  = { +24, +28, -24, -28 };
static int free_elephant[] = { +12, +14, -12, -14 };

static int dir_knight[]  = { -27, -25, -11, +15, +27, +25, +11, -15 };
static int free_knight[] = { -13, -13,  +1,  +1, +13, +13,  -1,  -1 };

static int dir_cannon[] = { +1, +13, -1, -13 };
static int dir_rook[] = { +1, +13, -1, -13 };
static int dir_king[] = { +1, +13, -1, -13 };


/*****************************************************************************
 * 
 * Basic attack functions.
 * 
 *****************************************************************************/

#define MAP() do {		\
	to = map[t];		\
	ok = (to != NO_SQUARE);	\
} while (0)

#define MAP_PALACE() do {	\
	to = map_palace[t];	\
	ok = (to != NO_SQUARE);	\
} while (0)

#define MAP_WHITEHALF() do {	\
	to = map_whitehalf[t];	\
	ok = (to != NO_SQUARE);	\
} while (0)

#define MAP_BLACKHALF() do {	\
	to = map_blackhalf[t];	\
	ok = (to != NO_SQUARE);	\
} while (0)


#define ADD(t) do {		\
	*top++ = to;		\
	n++;			\
} while (0)

#define ATTACK_PROLOGUE()	\
	int n = 0;		\
	Square* top = tos;	\
	int f = invmap[from];	\
	int t;			\
	Square to;		\
	bool ok;
	
#define ATTACK_EPILOGUE()	\
	return n;


unsigned int Board::pawn_attacks(Square from, Color side, Square tos[]) 
	const
{
	ATTACK_PROLOGUE();

	if (side == WHITE) {
		t = f + 13; MAP(); if (ok) ADD();
		if (RNK(from) >= RANK5) {
			t = f + 1; MAP(); if (ok) ADD();
			t = f - 1; MAP(); if (ok) ADD();
		}
	} else {
		t = f - 13; MAP(); if (ok) ADD();
		if (RNK(from) <= RANK4) {
			t = f + 1; MAP(); if (ok) ADD();
			t = f - 1; MAP(); if (ok) ADD();
		}
	}

	ATTACK_EPILOGUE();
}

unsigned int Board::guard_attacks(Square from, Color side, Square tos[]) 
	const
{
	(void) side;
	ATTACK_PROLOGUE();
	
	for (unsigned int i=0; i<4; i++) {
		t = f + dir_guard[i]; MAP_PALACE(); if (ok) ADD();
	}

	ATTACK_EPILOGUE();
}

unsigned int Board::elephant_attacks(Square from, Color side, Square tos[])
	const
{
	(void) side;
	ATTACK_PROLOGUE();

	for (unsigned int i=0; i<4; i++) {
		t = f + free_elephant[i];
		if (side == WHITE) {
			MAP_WHITEHALF();
		} else {
			MAP_BLACKHALF();
		}
			
		if (!ok || color_at(to) != NO_COLOR) {
			continue;
		}
		
		t = f + dir_elephant[i];
		if (side == WHITE) {
			MAP_WHITEHALF();
		} else {
			MAP_BLACKHALF();
		}
		
		if (ok) {
			ADD();
		}
	}
	
	ATTACK_EPILOGUE();
}

unsigned int Board::knight_attacks(Square from, Color side, Square tos[])
	const
{
	(void) side;
	ATTACK_PROLOGUE();

	for (unsigned int i=0; i<8; i++) {
		t = f + free_knight[i];
		MAP();
		if (!ok || color_at(to) != NO_COLOR) {
			continue;
		}
		
		t = f + dir_knight[i];
		MAP();
		if (ok) {
			ADD();
		}
	}
	
	ATTACK_EPILOGUE();
}

unsigned int Board::cannon_attacks(Square from, Color side, Square tos[])
	const
{
	ATTACK_PROLOGUE();

	for (unsigned int i=0; i<4; i++) {
		t = f;

		/* non-capture moves */
		do {
			t += dir_cannon[i];
			MAP();
			if (!ok) {
				goto break2;
			} else if (color_at(to) != NO_COLOR) {
				break;
			} else {
				ADD();
			}
		} while (1);

		/* capture move */
		do {
			t += dir_cannon[i];
			MAP();
			if (!ok || color_at(to) == side) {
				goto break2;
			} else if (color_at(to) == XSIDE(side)) {
				ADD();
				goto break2;
			}
		} while (1);
break2:;
	}
	
	ATTACK_EPILOGUE();
}

unsigned int Board::rook_attacks(Square from, Color side, Square tos[])
	const
{
	(void) side;
	ATTACK_PROLOGUE();

	for (unsigned int i=0; i<4; i++) {
		t = f;
		do {
			t += dir_rook[i];
			MAP();
			if (ok) {
				ADD();
			}
		} while (ok && color_at(to) == NO_COLOR);
	}
	
	ATTACK_EPILOGUE();
}

unsigned int Board::king_attacks(Square from, Color side, Square tos[]) 
	const
{
	(void) side;
	ATTACK_PROLOGUE();

	for (unsigned int i=0; i<4; i++) {
		t = f + dir_king[i]; MAP_PALACE(); if (ok) ADD();
	}
	
	ATTACK_EPILOGUE();
}




/*****************************************************************************
 * 
 * Returns true if square 'to' is attacked by any piece of 'atkside'.
 * 
 *****************************************************************************/

bool Board::is_attacked(Square to, Color atkside) const
{
	/* TODO this is just a quick'n'dirty implementation */
	
	Board b = *this;
	if (atkside != b.get_side()) {
		b.make_move(Move::null());
	}
	
	Movelist movelist;
	b.generate_moves(&movelist);
	for (unsigned int i=0; i<movelist.size(); i++) {
		Move mov = movelist[i];
		if (mov.to() == to) {
			return true;
		}
	}
	
	return false;
}

/*****************************************************************************
 * 
 * Returns true if the two kings are facing each other, i.e. they are on the
 * same file and no piece is between them.
 * 
 *****************************************************************************/

bool Board::kings_facing() const
{
	Square sqw = king[WHITE];
	Square sqb = king[BLACK];

	if (FIL(sqw) != FIL(sqb)) {
		return false;
	}

	for (Square sq = sqw+9; sq != sqb; sq += 9) {
		if (color_at(sq) != NO_COLOR) {
			return false;
		}
	}

	return true;
}

