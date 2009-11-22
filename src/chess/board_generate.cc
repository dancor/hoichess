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
#include "bitboard.h"
#include "move.h"
#include "basic.h"

/*
 * Should we use an independent generate_moves() routine, or simply call
 * generate_captures() and generate_noncaptures()?
 */
//#define USE_INDEPENDENT_GENERATE_MOVES

void Board::generate_moves(Movelist * movelist, bool allpromo) const
{
#ifndef USE_INDEPENDENT_GENERATE_MOVES
	generate_captures(movelist, allpromo);
	generate_noncaptures(movelist);
#else
	Bitboard bb, to_bb;
	Square from, to;

	/* pawn promotions and promotion-captures */
	bb = get_pawns(side) & (side == WHITE ? Bitboard::rank[RANK7]
			: Bitboard::rank[RANK2]);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		/* promotion-captures */
		to_bb = pawn_captures(from, side) & get_pieces(opponent);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			Piece cpce = piece_at(to);
			movelist->add(Move::promotion_capture(from, to,
						QUEEN, cpce));
			movelist->add(Move::promotion_capture(from, to,
						KNIGHT, cpce));
			if (allpromo) {
				movelist->add(Move::promotion_capture(from, to,
							BISHOP, cpce));
				movelist->add(Move::promotion_capture(from, to,
							ROOK, cpce));
			}
		}
		
		/* promotions */
		to_bb = pawn_noncaptures(from, side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::promotion(from, to, QUEEN));
			movelist->add(Move::promotion(from, to, KNIGHT));
			if (allpromo) {
				movelist->add(Move::promotion(from, to,
							BISHOP));
				movelist->add(Move::promotion(from, to,
							ROOK));
			}
		}
	}

	/* pawn non-promotions */
	bb = get_pawns(side) & ~(side == WHITE ? Bitboard::rank[RANK7]
			: Bitboard::rank[RANK2]);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);
		
		/* captures */
		to_bb = pawn_captures(from, side) & ~get_pieces(side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);
			
			if (to == epsq) {
				movelist->add(Move::enpassant(from, to));
			} else {
				movelist->add(Move::capture(from, to, PAWN,
							piece_at(to)));
			}
		}
				
		/* non-captures */
		to_bb = pawn_noncaptures(from, side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);
			
			movelist->add(Move::normal(from, to, PAWN));
		}
	}

	/* knights */
	bb = get_knights(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = knight_attacks(from) & ~get_pieces(side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);
			
			if (get_pieces(opponent).testbit(to)) {
				movelist->add(Move::capture(from, to, KNIGHT,
							piece_at(to)));
			} else {
				movelist->add(Move::normal(from, to, KNIGHT));
			}
		}
	}
			
	/* bishops */
	bb = get_bishops(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = bishop_attacks(from) & ~get_pieces(side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);
			
			if (get_pieces(opponent).testbit(to)) {
				movelist->add(Move::capture(from, to, BISHOP,
							piece_at(to)));
			} else {
				movelist->add(Move::normal(from, to, BISHOP));
			}
		}
	}
	
	/* rooks */
	bb = get_rooks(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = rook_attacks(from) & ~get_pieces(side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);
			
			if (get_pieces(opponent).testbit(to)) {
				movelist->add(Move::capture(from, to, ROOK,
							piece_at(to)));
			} else {
				movelist->add(Move::normal(from, to, ROOK));
			}
		}
	}
	
	/* queens */
	bb = get_queens(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = queen_attacks(from) & ~get_pieces(side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);
			
			if (get_pieces(opponent).testbit(to)) {
				movelist->add(Move::capture(from, to, QUEEN,
							piece_at(to)));
			} else {
				movelist->add(Move::normal(from, to, QUEEN));
			}
		}
	}
	
	/* king, only one */
	from = get_king(side);
	to_bb = king_attacks(from) & ~get_pieces(side);
	while (to_bb) {
		to = to_bb.firstbit();
		to_bb.clearbit(to);

		if (get_pieces(opponent).testbit(to)) {
			movelist->add(Move::capture(from, to, KING,
						piece_at(to)));
		} else {
			movelist->add(Move::normal(from, to, KING));
		}
	}
	
	/* try castling */
	generate_castling(movelist);
#endif // USE_INDEPENDENT_GENERATE_MOVES
}

/*
 * This routine generates all moves that change the material value on
 * the board. This includes pawn promotions, not only captures.
 * Promotions into bishop and rook are only generated when allpromo == true,
 * because we want so skip those useless moves during search.
 */
void Board::generate_captures(Movelist * movelist, bool allpromo) const 
{
	Bitboard bb, to_bb;
	Square from, to;

	/* pawn promotions and promotion-captures */
	bb = get_pawns(side) & (side == WHITE ? Bitboard::rank[RANK7]
			: Bitboard::rank[RANK2]);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		/* promotion-captures */
		to_bb = pawn_captures(from, side) & get_pieces(opponent);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			Piece cpce = piece_at(to);
			movelist->add(Move::promotion_capture(from, to,
						QUEEN, cpce));
			movelist->add(Move::promotion_capture(from, to,
						KNIGHT, cpce));
			if (allpromo) {
				movelist->add(Move::promotion_capture(from, to,
							BISHOP, cpce));
				movelist->add(Move::promotion_capture(from, to,
							ROOK, cpce));
			}
		}

		/* promotions */
		to_bb = pawn_noncaptures(from, side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::promotion(from, to, QUEEN));
			movelist->add(Move::promotion(from, to, KNIGHT));
			if (allpromo) {
				movelist->add(Move::promotion(from, to,
							BISHOP));
				movelist->add(Move::promotion(from, to,
							ROOK));
			}
		}
	}
	
	/* pawn captures */
	bb = get_pawns(side) & ~(side == WHITE ? Bitboard::rank[RANK7]
			: Bitboard::rank[RANK2]);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = pawn_captures(from, side) & ~get_pieces(side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			if (to == epsq) {
				movelist->add(Move::enpassant(from, to));
			} else {
				movelist->add(Move::capture(from, to, PAWN,
							piece_at(to)));
			}
		}
	}

	/* knights */
	bb = get_knights(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = knight_attacks(from) & get_pieces(opponent);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::capture(from, to, KNIGHT,
						piece_at(to)));
		}
	}
	
	/* bishops */
	bb = get_bishops(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = bishop_attacks(from) & get_pieces(opponent);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::capture(from, to, BISHOP,
						piece_at(to)));
		}
	}

	/* rooks */
	bb = get_rooks(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = rook_attacks(from) & get_pieces(opponent);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::capture(from, to, ROOK,
						piece_at(to)));
		}
	}

	/* queens */
	bb = get_queens(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = queen_attacks(from) & get_pieces(opponent);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::capture(from, to, QUEEN,
						piece_at(to)));
		}
	}

	/* king, only one */
	from = get_king(side);
	to_bb = king_attacks(from) & get_pieces(opponent);
	while (to_bb) {
		to = to_bb.firstbit();
		to_bb.clearbit(to);

		movelist->add(Move::capture(from, to, KING,
					piece_at(to)));
	}
}

/*
 * This routine generates all moves except captures and promotions.
 */
void Board::generate_noncaptures(Movelist * movelist) const
{
	Bitboard bb, to_bb;
	Square from, to;
	
	/* first try castling */
	generate_castling(movelist);

	/* pawn non-captures non-promotions */
	bb = get_pawns(side) & ~(side == WHITE ? Bitboard::rank[RANK7] 
				: Bitboard::rank[RANK2]);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);
	
		to_bb = pawn_noncaptures(from, side);
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::normal(from, to, PAWN));
		}
	}
			
	/* knights */
	bb = get_knights(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = knight_attacks(from) & ~get_blocker();
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::normal(from, to, KNIGHT));
		}
	}
	
	/* bishops */
	bb = get_bishops(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = bishop_attacks(from) & ~get_blocker();
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::normal(from, to, BISHOP));
		}
	}

	/* rooks */
	bb = get_rooks(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = rook_attacks(from) & ~get_blocker();
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::normal(from, to, ROOK));
		}
	}

	/* queens */
	bb = get_queens(side);
	while (bb) {
		from = bb.firstbit();
		bb.clearbit(from);

		to_bb = queen_attacks(from) & ~get_blocker();
		while (to_bb) {
			to = to_bb.firstbit();
			to_bb.clearbit(to);

			movelist->add(Move::normal(from, to, QUEEN));
		}
	}

	/* king, only one */
	from = get_king(side);
	to_bb = king_attacks(from) & ~get_blocker();
	while (to_bb) {
		to = to_bb.firstbit();
		to_bb.clearbit(to);

		movelist->add(Move::normal(from, to, KING));
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
	
	Bitboard from_bb, to_bb;
	Square from, to;

	Square king = get_king(side);
	Bitboard checkers = attackers(king, opponent);
	
	/*
	 * Try to move the king.
	 */
	to_bb = king_attacks(king) & ~get_pieces(side);
	while (to_bb) {
		to = to_bb.firstbit();
		to_bb.clearbit(to);

		if (is_attacked(to, opponent)) {
			continue;
		} else if (get_pieces(opponent).testbit(to)) {
			movelist->add(Move::capture(king, to, KING,
						piece_at(to)));
		} else {
			movelist->add(Move::normal(king, to, KING));
		}
	}

	/* If our king is attacked my more than one
	 * enemy piece, moving the king is the only
	 * possibility */
	if (checkers.popcnt() > 1)
		return;
	
	Square checker = checkers.firstbit();
	Piece checker_ptype = piece_at(checker);
	ASSERT_DEBUG(checker_ptype != KING);

	/*
	 * Try to capture the checking piece.
	 * Captures taken by the king were
	 * already considered above.
	 */
	from_bb = attackers(checker, side) & ~get_kings(side);
	while (from_bb) {
		from = from_bb.firstbit();
		from_bb.clearbit(from);

		add_move(movelist, from, checker);
	}

	/* Also try enpassant capture. */
	if (checker == get_eppawn()) {
		from_bb = pawn_captures(epsq, opponent) & get_pawns(side);
		while (from_bb) {
			from = from_bb.firstbit();
			from_bb.clearbit(from);

			movelist->add(Move::enpassant(from, epsq));
		}
	}

	/*
	 * Try to block the attack.
	 * Blocking by enpassant capture is not possible.
	 */
	if (checker_ptype == PAWN || checker_ptype == KNIGHT)
		return;
	to_bb = Bitboard::ray_bb[checker][king];
	while (to_bb) {
		to = to_bb.firstbit();
		to_bb.clearbit(to);

		ASSERT_DEBUG(piece_at(to) == NO_PIECE);

		from_bb = attackers(to, side);

		/* Hmm, pawn forward moves must
		 * be added separately. */
		if (side == WHITE) {
			if (RNK(to) != RANK1
					&& get_pawns(side).testbit(to-8)) {
				from_bb.setbit(to-8);
			} else if (RNK(to) == RANK4
					&& !get_blocker().testbit(to-8)
					&& get_pawns(side).testbit(to-16)) {
				from_bb.setbit(to-16);
			}
		} else {
			if (RNK(to) != RANK8
					&& get_pawns(side).testbit(to+8)) {
				from_bb.setbit(to+8);
			} else if (RNK(to) == RANK5
					&& !get_blocker().testbit(to+8)
					&& get_pawns(side).testbit(to+16)) {
				from_bb.setbit(to+16);
			}
		}
		
		while (from_bb) {
			from = from_bb.firstbit();
			from_bb.clearbit(from);

			if (get_blocker() & Bitboard::ray_bb[from][to])
				continue;

			add_move(movelist, from, to);
		}
	}
}

/*
 * Generate castling, if possible.
 * Note that we generate pseudo-legal moves here too,
 * so we might castle into check. (But not _out_of_ check
 * or _through_ check, since this cannot be verified later.)
 */
void Board::generate_castling(Movelist * movelist) const
{
	if (in_check())
		return;
	
	if (side == WHITE) {
		if (flags & WKCASTLE
				&& !(get_blocker() & Bitboard::ray_bb[E1][H1])
				&& !is_attacked(F1, BLACK)) {
			movelist->add(Move::castle(E1, G1));
		}
		if (flags & WQCASTLE
				&& !(get_blocker() & Bitboard::ray_bb[E1][A1])
				&& !is_attacked(D1, BLACK)) {
			movelist->add(Move::castle(E1, C1));
		}
	} else {
		if (flags & BKCASTLE
				&& !(get_blocker() & Bitboard::ray_bb[E8][H8])
				&& !is_attacked(F8, WHITE)) {
			movelist->add(Move::castle(E8, G8));
		}
		if (flags & BQCASTLE
				&& !(get_blocker() & Bitboard::ray_bb[E8][A8])
				&& !is_attacked(D8, WHITE)) {
			movelist->add(Move::castle(E8, C8));
		}
	}	
}

/*
 * Add many moves described by from-square and to-bitboard to the movelist.
 * This function uses add_move(), so see its comment below.
 */
void Board::add_moves(Movelist * movelist, Square from, Bitboard to_bb) const
{
	Square to;

	while (to_bb) {
		to = to_bb.firstbit();
		to_bb.clearbit(to);

		add_move(movelist, from, to);
	}
}

/*
 * Add a single move described only by from- and to-square to the movelist.
 * This is a very inefficient function, because we must find out all
 * information about the move here, so avoid it if possible.
 */
void Board::add_move(Movelist * movelist, Square from, Square to) const
{
	ASSERT_DEBUG(to != get_king(opponent));
	
	Piece ptype = piece_at(from);

	if (ptype == PAWN && to == epsq) {
		movelist->add(Move::enpassant(from, to));
	} else if (get_pieces(opponent).testbit(to)) {
		Piece cap_ptype = piece_at(to);
		if (ptype == PAWN && ((side == WHITE && RNK(to) == RANK8)
				   || (side == BLACK && RNK(to) == RANK1))) {
			movelist->add(Move::promotion_capture(from, to, QUEEN,
						cap_ptype));
			movelist->add(Move::promotion_capture(from, to, KNIGHT,
						cap_ptype));
			movelist->add(Move::promotion_capture(from, to, BISHOP,
						cap_ptype));
			movelist->add(Move::promotion_capture(from, to, ROOK,
						cap_ptype));
		} else {
			movelist->add(Move::capture(from, to, ptype,
						cap_ptype));
		}
	} else {
		if (ptype == PAWN && ((side == WHITE && RNK(to) == RANK8)
				   || (side == BLACK && RNK(to) == RANK1))) {
			movelist->add(Move::promotion(from, to, QUEEN));
			movelist->add(Move::promotion(from, to, KNIGHT));
			movelist->add(Move::promotion(from, to, BISHOP));
			movelist->add(Move::promotion(from, to, ROOK));
		} else {
			movelist->add(Move::normal(from, to, ptype));
		}
	}
}

