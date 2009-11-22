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
#include <string.h>

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
 * Return a string for coordinate notation, e.g.
 * e2e4, e7e8q, e1g1 (may be white king-side castling).
 */
std::string Move::str() const
{
	char ss[6];

	snprintf(ss, sizeof(ss), "%s%s%c",
			square_str[from()],
			square_str[to()],
			(flags() & MOVE_PROMOTION) ?
					tolower(piece_char[promote_to()])
					: '\0' );

	return std::string(ss);
}

/*
 * Return a string for standard algebraic notation, e.g.
 * e4, e8=Q, Nxc3#, O-O.
 *
 * If nonstd is not 0, some variations to SAN will be done, in order to
 * allow Board::parse_move() to accept non standard notations.
 * Values for nonstd:
 * 	0: generate official SAN
 * 	1: leave out trailing '+' or '#'
 * 	2: leave out 'x' for captures
 * 	3: include disambiguation by file even if not necessary
 * 	4: include disambiguation by rank even if not necessary
 * 	5: include disambiguation by file and rank even if not necessary
 */
std::string Move::san(const Board & board, int nonstd) const
{
	char ss[8];	/* longest is 7 (exd8=Q# or Qc3xc6#) */
	memset(ss, 0, sizeof(ss));
	char * s = ss;

	if (flags() & MOVE_CASTLE) {
		if (FIL(to()) == 2) {
			strcpy(s, "O-O-O");
			s += 5;
		} else {
			strcpy(s, "O-O");
			s += 3;
		}
	} else if (ptype() == PAWN) {
		if (flags() & MOVE_CAPTURE 
				|| flags() & MOVE_ENPASSANT) {
			*s++ = file_char[FIL(from())];
		}
	} else {
		*s++ = piece_char[ptype()];
	
		/* ambiguity check */
		bool disamb_file = false;
		bool disamb_rank = false;
	
		Movelist movelist;
		board.generate_moves(&movelist);
		movelist.filter_illegal(board);
		for (unsigned int i=0; i<movelist.size(); i++) {
			Move mov = movelist[i];
			
			if (mov.ptype() != ptype())
				continue;
			if (mov.to() != to())
				continue;
			if (mov.from() == from())
				continue;
		
			/* disambiguation necessary */
			if (FIL(mov.from()) != FIL(from())) {
				disamb_file = true;
			} else if (RNK(mov.from()) != RNK(from())) {
				disamb_rank = true;
			}
		}
		
		if (nonstd == 3) {
			disamb_file = true;			
		} else if (nonstd == 4) {
			disamb_rank = true;
		} else if (nonstd == 5) {
			disamb_file = true;
			disamb_rank = true;
		}
		
		if (disamb_file)
			*s++ = file_char[FIL(from())];
		
		if (disamb_rank)
			*s++ = rank_char[RNK(from())];
	}

	/* capture? */
	if ((flags() & MOVE_CAPTURE || flags() & MOVE_ENPASSANT)
			&& nonstd != 2) {
		*s++ = 'x';
	}
	
	/* destination square */
	if (! (flags() & MOVE_CASTLE)) {
		*s++ = file_char[FIL(to())];
		*s++ = rank_char[RNK(to())];
	}

	/* pawn promotion? */
	if (flags() & MOVE_PROMOTION) {
		*s++ = '=';
		*s++ = piece_char[promote_to()];	
	}
	
	/* check or checkmate? */
	if (nonstd != 1) {
		Board tmpboard = board;
		tmpboard.make_move(*this);
		if (tmpboard.in_check()) {
			if (tmpboard.is_mate()) {
				*s++ = '#';
			} else {
				*s++ = '+';
			}
		}
	}

	return std::string(ss);
}

/*
 * Print the raw move data (for debugging purposes).
 */
void Move::print() const
{
	printf("mov = 0x%08lx\n", (unsigned long) mov);
	
	if (from() >= 0 && from() <= 63) {
		printf("\tfrom = '%s'\n", square_str[from()]);
	} else {
		printf("\tfrom = %d\n", from());
	}
	
	if (to() >= 0 && to() <= 63) {
		printf("\tto = '%s'\n", square_str[to()]);
	} else {
		printf("\tto = %d\n", to());
	}
	
	if (ptype() >= PAWN && ptype() <= KING) {
		printf("\tptype = '%c'\n", piece_char[ptype()]);
	} else {
		printf("\tptype = %d\n", ptype());
	}
		
	if (promote_to() >= PAWN && promote_to() <= KING) {
		printf("\tpromote_to = '%c'\n", piece_char[promote_to()]);
	} else {
		printf("\tpromote_to = %d\n", promote_to());
	}
	
	if (cap_ptype() >= PAWN && cap_ptype() <= KING) {
		printf("\tcap_ptype = '%c'\n", piece_char[cap_ptype()]);
	} else {
		printf("\tcap_ptype = %d\n", cap_ptype());
	}
	
	printf("\tflags = 0x%x\n", flags());
}

