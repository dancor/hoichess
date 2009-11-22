/* $Id: move.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/move.cc
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* TODO Inline them.
 * Problem: Board::is_{valid,legal}_move() are not yet declared in
 * move.h due to mutual inclusion of board.h and move.h :-( */
bool Move::is_valid(const Board & board) const
{
	return board.is_valid_move(*this);
}

bool Move::is_legal(const Board & board) const
{
	return board.is_legal_move(*this);
}

/*
 * Return a string for coordinate notation, e.g. e2e4.
 */
std::string Move::str() const
{
	char ss[6];

	snprintf(ss, sizeof(ss), "%s%s",
			square_str[from()],
			square_str[to()]);

	return std::string(ss);
}

std::string Move::san(const Board & board, int nonstd) const
{
	(void) nonstd;

	char ss[12];
	
	const char * check = "";
	Board b = board;
	b.make_move(*this);
	if (b.is_mate() || b.is_stalemate()) {
		check = "#";
	} else if (b.in_check()) {
		check = "+";
	}
	
	snprintf(ss, sizeof(ss), "%c%s%s%s%s",
			piece_char[board.piece_at(from())],
			square_str[from()],
			is_capture() ? "x" : "-",
			square_str[to()],
			check);

	return std::string(ss);
}

/*
 * Print the raw move data (for debugging purposes).
 */
void Move::print() const
{
	if (from() >= A0 && from() <= I9) {
		printf("\tfrom = '%s'\n", square_str[from()]);
	} else {
		printf("\tfrom = %d\n", from());
	}
	
	if (to() >= A0 && to() <= I9) {
		printf("\tto = '%s'\n", square_str[to()]);
	} else {
		printf("\tto = %d\n", to());
	}
	
	if (ptype() >= PAWN && ptype() <= KING) {
		printf("\tptype = '%c'\n", piece_char[ptype()]);
	} else {
		printf("\tptype = %d\n", ptype());
	}
		
	if (cap_ptype() >= PAWN && cap_ptype() <= KING) {
		printf("\tcap_ptype = '%c'\n", piece_char[cap_ptype()]);
	} else {
		printf("\tcap_ptype = %d\n", cap_ptype());
	}
	
	printf("\tflags = 0x%x\n", flags());
}

