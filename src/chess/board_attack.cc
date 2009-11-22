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
#include "bitboard.h"
#include "basic.h"

/*
 * Returns true if square 'to' is attacked by any piece of 'atkside'.
 */
bool Board::is_attacked(Square to, Color atkside) const
{
	/* pawns */
	if (pawn_captures(to, !atkside) & get_pawns(atkside))
		return true;
	
	/* rooks and queens */
	if (rook_attacks(to) & (get_rooks(atkside) | get_queens(atkside)))
		return true;

	/* bishops and queens */
	if (bishop_attacks(to) & (get_bishops(atkside) | get_queens(atkside)))
		return true;

	/* knights */
	if (knight_attacks(to) & get_knights(atkside))
		return true;

	/* kings */
	if (king_attacks(to) & get_kings(atkside))
		return true;

	return false;
}

/*
 * Returns a Bitboard with all pieces of 'atkside'
 * that attack the square 'to'.
 */
Bitboard Board::attackers(Square to, Color atkside) const
{
	Bitboard ret_bb = NULLBITBOARD;
	
	/* pawns */
	ret_bb |= (pawn_captures(to, XSIDE(atkside)) & get_pawns(atkside));
	
	/* knights */
	ret_bb |= (knight_attacks(to) & get_knights(atkside));
	
	/* bishops */
	ret_bb |= (bishop_attacks(to) & get_bishops(atkside));
	
	/* rooks */
	ret_bb |= (rook_attacks(to) & get_rooks(atkside));

	/* queens */
	ret_bb |= (queen_attacks(to) & get_queens(atkside));
	
	/* kings */
	ret_bb |= (king_attacks(to) & get_kings(atkside));
	
	return ret_bb;
}

/*
 * Find all pieces of 'side' that are pinned to side's piece on square 'to'.
 */
Bitboard Board::pinned(Square to, Color side) const
{	
	ASSERT_DEBUG(piece_at(to) != NO_PIECE);
	ASSERT_DEBUG(color_at(to) == side);

	Bitboard ret_bb = NULLBITBOARD;
	Bitboard bb;
	Bitboard tmp;
	Color atkside = XSIDE(side);
	Square from;

	/* bishops and queens */
	bb = get_bishops(atkside) | get_queens(atkside);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);
		
		/* all pieces on diagonal between from and to */
		tmp = Bitboard::attack_bb[BISHOP][to] 
			& Bitboard::ray_bb[from][to] & get_blocker();

		/* if there are any pieces of atkside, there is no pin */
		if (tmp & get_pieces(atkside)) {
			continue;
		}

		/* if there is exactly one piece of side, it is pinned */
		//tmp &= get_pieces(side);
		if (tmp.popcnt() == 1) {
			Square sq = tmp.firstbit();
			ret_bb.setbit(sq);
		}
	}
		
	/* rooks and queens */
	bb = get_rooks(atkside) | get_queens(atkside);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);
		
		/* all pieces on file or rank between from and to */
		tmp = Bitboard::attack_bb[ROOK][to] 
			& Bitboard::ray_bb[from][to] & get_blocker();

		/* if there are any pieces of atkside, there is no pin */
		if (tmp & get_pieces(atkside)) {
			continue;
		}

		/* if there is exactly one piece of side, it is pinned */
		//tmp &= get_pieces(side);
		if (tmp.popcnt() == 1) {
			Square sq = tmp.firstbit();
			ret_bb.setbit(sq);
		}
	}

	return ret_bb;
}
