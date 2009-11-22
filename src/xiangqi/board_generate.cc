/* $Id: board_generate.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/board_generate.cc
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
#include "move.h"
#include "basic.h"

#define ADD_IF_NONCAPTURE(movelist, from, to, ptype) do {		\
	if (color_at(to) == NO_COLOR) {					\
		(movelist)->add(Move::normal((from), (to), (ptype)));	\
	}								\
} while (0)

#define ADD_IF_CAPTURE(movelist, from, to, ptype) do {			\
	if (color_at(to) == opponent) {					\
		(movelist)->add(Move::capture((from), (to), (ptype),	\
					      piece_at(to)));		\
	}								\
} while (0)


/*
 * Should we use an independent generate_moves() routine, or simply call
 * generate_captures() and generate_noncaptures()?
 */
//#define USE_INDEPENDENT_GENERATE_MOVES

void Board::generate_moves(Movelist * movelist) const
{
#ifndef USE_INDEPENDENT_GENERATE_MOVES
	generate_captures(movelist);
	generate_noncaptures(movelist);
#else
	/* TODO not implemented */
#endif // USE_INDEPENDENT_GENERATE_MOVES
}

/*
 * This routine generates all moves that change the material value on
 * the board.
 */
void Board::generate_captures(Movelist * movelist) const 
{
	Square tos[128];

	for (Square from=A0; from<=I9; from++) {
		if (color_at(from) != side) {
			continue;
		}

		Piece ptype = piece_at(from);
		unsigned int n;

		switch (ptype) {
		case PAWN:
			n = pawn_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_CAPTURE(movelist, from, to, ptype);
			}
			break;

		case GUARD:
			n = guard_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_CAPTURE(movelist, from, to, ptype);
			}
			break;

		case ELEPHANT:
			n = elephant_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_CAPTURE(movelist, from, to, ptype);
			}
			break;

		case KNIGHT:
			n = knight_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_CAPTURE(movelist, from, to, ptype);
			}
			break;

		case CANNON:
			n = cannon_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_CAPTURE(movelist, from, to, ptype);
			}
			break;

		case ROOK:
			n = rook_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_CAPTURE(movelist, from, to, ptype);
			}
			break;

		case KING:
			n = king_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_CAPTURE(movelist, from, to, ptype);
			}
			break;

		default:
			BUG("illegal piece: %d", ptype);
		}
	}
}

/*
 * This routine generates all moves except captures and promotions.
 */
void Board::generate_noncaptures(Movelist * movelist) const
{
	Square tos[128];

	for (Square from=A0; from<=I9; from++) {
		if (color_at(from) != side) {
			continue;
		}

		Piece ptype = piece_at(from);
		unsigned int n;

		switch (ptype) {
		case PAWN:
			n = pawn_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_NONCAPTURE(movelist, from, to, ptype);
			}
			break;

		case GUARD:
			n = guard_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_NONCAPTURE(movelist, from, to, ptype);
			}
			break;

		case ELEPHANT:
			n = elephant_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_NONCAPTURE(movelist, from, to, ptype);
			}
			break;

		case KNIGHT:
			n = knight_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_NONCAPTURE(movelist, from, to, ptype);
			}
			break;

		case CANNON:
			n = cannon_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_NONCAPTURE(movelist, from, to, ptype);
			}
			break;

		case ROOK:
			n = rook_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_NONCAPTURE(movelist, from, to, ptype);
			}
			break;

		case KING:
			n = king_attacks(from, side, tos);
			for (unsigned int i=0; i<n; i++) {
				Square to = tos[i];
				ADD_IF_NONCAPTURE(movelist, from, to, ptype);
			}
			break;

		default:
			BUG("illegal piece: %d", ptype);
		}
	}
}


/*
 * Generate all moves that bring us out of check.
 * 
 * FIXME This is not really true, we only generate pseudo-legal
 * moves, so the king might actually be left in check!
 */
void Board::generate_escapes(Movelist * movelist) const
{
	if (!in_check()) {
		DBG(1, "generate_escapes() called but not in check!");
		return;
	}
	
	/* TODO not implemented */
	BUG("not implemented");
}

