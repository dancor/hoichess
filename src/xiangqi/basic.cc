/* $Id: basic.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/basic.cc
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

#include "common.h"
#include "basic.h"

#if 0
const char piece_char[] = { 'P', 'G', 'E', 'N', 'C', 'R', 'K' };
#else
const char piece_char[] = { 'P', 'F', 'E', 'N', 'O', 'R', 'K' };
#endif
const char file_char[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i' };
const char rank_char[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
const char square_str[][3] = {
	"a0", "b0", "c0", "d0", "e0", "f0", "g0", "h0", "i0",
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "i1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "i2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "i3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "i4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "i5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "i6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "i7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "i8",
	"a9", "b9", "c9", "d9", "e9", "f9", "g9", "h9", "i9",
};

/*
 * Create a string with the FEN of the standard opening position.
 */
#define WP(p) (std::string(1, (char) toupper(piece_char[p])))
#define BP(p) (std::string(1, (char) tolower(piece_char[p])))
#define NP(n) (std::string(1, (char) ('0' + (n))))
std::string opening_fen()
{
	std::string fen;

	fen +=    BP(ROOK) + BP(KNIGHT) + BP(ELEPHANT) + BP(GUARD) + BP(KING)
		+ BP(GUARD) + BP(ELEPHANT) + BP(KNIGHT) + BP(ROOK)
		+ "/"
		+ NP(9)
		+ "/"
		+ NP(1) + BP(CANNON) + NP(5) + BP(CANNON) + NP(1)
		+ "/"
		+ BP(PAWN) + NP(1) + BP(PAWN) + NP(1) + BP(PAWN) + NP(1)
		+ BP(PAWN) + NP(1) + BP(PAWN)
		+ "/"
		+ NP(9)
		+ "/"
		+ NP(9)
		+ "/"
		+ WP(PAWN) + NP(1) + WP(PAWN) + NP(1) + WP(PAWN) + NP(1)
		+ WP(PAWN) + NP(1) + WP(PAWN)
		+ "/"
		+ NP(1) + WP(CANNON) + NP(5) + WP(CANNON) + NP(1)
		+ "/"
		+ NP(9)
		+ "/"
		+ WP(ROOK) + WP(KNIGHT) + WP(ELEPHANT) + WP(GUARD) + WP(KING)
		+ WP(GUARD) + WP(ELEPHANT) + WP(KNIGHT) + WP(ROOK)
		+ " w - - 0 1";

	return fen;
}
#undef WP
#undef BP
#undef NP

/*
 * Initialization function for basic stuff.
 */

void basic_init()
{
}
