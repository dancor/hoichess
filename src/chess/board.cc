/* $Id: board.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/board.cc
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

Board::Board()
{
	clear();
}

Board::Board(const std::string& fen)
{
	clear();
	if (!parse_fen(fen)) {
		BUG("Constructor called with illegal FEN: %s", fen.c_str());
	}
}

void Board::clear()
{
	side = WHITE;
	opponent = BLACK;
	moveno = 1;
	movecnt50 = 0;
	
	for (int i=0; i<6; i++) {
		position[WHITE][i] = NULLBITBOARD;
		position[BLACK][i] = NULLBITBOARD;
	}
	position_all[WHITE] = NULLBITBOARD;
	position_all[BLACK] = NULLBITBOARD;

	occupied = NULLBITBOARD;
	occupied_l90 = NULLBITBOARD;
	occupied_l45 = NULLBITBOARD;
	occupied_r45 = NULLBITBOARD;

	king[WHITE] = NO_SQUARE;
	king[BLACK] = NO_SQUARE;
	
	flags = 0;
	epsq = NO_SQUARE;

	material[WHITE] = 0;
	material[BLACK] = 0;
	has_castled[WHITE] = false;
	has_castled[BLACK] = false;

	for (unsigned int sq = 0; sq < 64; sq++) {
		pce_movecnt[sq] = 0;
	}
	
	hashkey = NULLHASHKEY;
	pawnhashkey = NULLHASHKEY;
}

#ifdef USE_UNMAKE_MOVE
BoardHistory Board::make_move(Move mov)
#else
void Board::make_move(Move mov)
#endif
{
	ASSERT_DEBUG(is_valid_move(mov));

#ifdef USE_UNMAKE_MOVE
	BoardHistory hist;
#ifdef DEBUG
	hist.oldboard = *this;
#endif
	hist.move = mov;
#endif // USE_UNMAKE_MOVE
	
	/* Move pieces */
	if (mov.is_castle()) {
		move_piece(mov.from(), mov.to(), side, KING);
		switch (mov.to()) {
			case C1:
				ASSERT_DEBUG(side == WHITE);
				ASSERT_DEBUG(flags & WQCASTLE);
				move_piece(A1, D1, side, ROOK);
				break;
			case G1: 
				ASSERT_DEBUG(side == WHITE);
				ASSERT_DEBUG(flags & WKCASTLE);
				move_piece(H1, F1, side, ROOK);
				break;
			case C8:
				ASSERT_DEBUG(side == BLACK);
				ASSERT_DEBUG(flags & BQCASTLE);
				move_piece(A8, D8, side, ROOK);
				break;
			case G8:
				ASSERT_DEBUG(side == BLACK);
				ASSERT_DEBUG(flags & BKCASTLE);
				move_piece(H8, F8, side, ROOK);
				break;
			default:
				BUG("invalid 'to' square for castling: %d",
						mov.to());
				break;
		}				
		has_castled[side] = true;
		/* Castling flags will be cleared below. */
	}
	else if (mov.is_enpassant()) {
		ASSERT_DEBUG(mov.to() == epsq);
		move_piece(mov.from(), mov.to(), side, PAWN);
		remove_piece(get_eppawn(), opponent, PAWN);
	}
	else if (mov.is_capture()) {
		if (mov.is_promotion()) {
			remove_piece(mov.from(), side, PAWN);
			remove_piece(mov.to(), opponent, mov.cap_ptype());
			place_piece(mov.to(), side, mov.promote_to());
		} else {
			remove_piece(mov.to(), opponent, mov.cap_ptype());
			move_piece(mov.from(), mov.to(), side, mov.ptype());
		}
	}
	else if (mov.is_normal()) {
		if (mov.is_promotion()) {
			remove_piece(mov.from(), side, PAWN);
			place_piece(mov.to(), side, mov.promote_to());
		} else {
			move_piece(mov.from(), mov.to(), side, mov.ptype());
		}
	} 
	else if (mov.is_null()) {
		/* nothing */
	}
	else {
		BUG("unknown move flags: %x", mov.flags());
	}

#ifdef USE_UNMAKE_MOVE
	hist.flags = flags;
#endif
	
	/* Clear castling flag if a king has moved */
	if (mov.ptype() == KING) {
		if (side == WHITE) {
			clear_flag(WKCASTLE);
			clear_flag(WQCASTLE);
		} else {
			clear_flag(BKCASTLE);
			clear_flag(BQCASTLE);
		}
	}

	/* Clear castling flag if a rook has moved */
	if (mov.ptype() == ROOK) {
		switch (mov.from()) {
			case A1:
				clear_flag(WQCASTLE);
				break;
			case H1:
				clear_flag(WKCASTLE);
				break;
			case A8:
				clear_flag(BQCASTLE);
				break;
			case H8:
				clear_flag(BKCASTLE);
				break;
		}
	} 
	
	/* Clear castling flag if a rook was captured */
	if (mov.is_capture() && mov.cap_ptype() == ROOK) {
		switch (mov.to()) {
			case A1:
				clear_flag(WQCASTLE);
				break;
			case H1:
				clear_flag(WKCASTLE);
				break;
			case A8:
				clear_flag(BQCASTLE);
				break;
			case H8:
				clear_flag(BKCASTLE);
				break;
		}
	}
	
	/* Set/clear enpassant square. */
#ifdef USE_UNMAKE_MOVE
	hist.epsq = epsq;
#endif
	if (mov.ptype() == PAWN) {
		if (mov.to() - mov.from() == 16) {
			set_epsq(mov.to() - 8);
		} else if (mov.to() - mov.from() == -16) {
			set_epsq(mov.to() + 8);
		} else {
			clear_epsq();
		}
	} else {
		clear_epsq();
	}

	/* Switch sides and update moveno */
	switch_sides();
	if (side == WHITE) {
		moveno++;
	}
	
	/* Update movecnt50 */
#ifdef USE_UNMAKE_MOVE
	hist.movecnt50 = movecnt50;
#endif
	if (mov.ptype() == PAWN || mov.is_castle() || mov.is_capture()) {
		movecnt50 = 0;
	} else {
		movecnt50++;
	}

	/* Update pce_movecnt */
	if (!mov.is_null()) {
#ifdef USE_UNMAKE_MOVE
		hist.pce_movecnt_to = pce_movecnt[mov.to()];
#endif
		pce_movecnt[mov.to()] = pce_movecnt[mov.from()] + 1;
		pce_movecnt[mov.from()] = 0;
	}
	

#ifdef USE_UNMAKE_MOVE
	return hist;
#else
	return;
#endif
}

#ifdef USE_UNMAKE_MOVE
void Board::unmake_move(const BoardHistory & hist)
{
	Move mov = hist.move;
	
	/* Restore pce_movecnt */
	if (!mov.is_null()) {
		pce_movecnt[mov.from()] = pce_movecnt[mov.to()] - 1;	
		pce_movecnt[mov.to()] = hist.pce_movecnt_to;
	}
	
	/* Restore movecnt50 */
	movecnt50 = hist.movecnt50;

	/* Switch back sides */
	if (side == WHITE) {
		moveno--;
	}
	switch_sides();

	/* Restore enpassant square */
	set_epsq(hist.epsq);

	/* Restore flags */
	if (hist.flags & WKCASTLE) {
		set_flag(WKCASTLE);
	}
	if (hist.flags & WQCASTLE) {
		set_flag(WQCASTLE);
	}
	if (hist.flags & BKCASTLE) {
		set_flag(BKCASTLE);
	}
	if (hist.flags & BQCASTLE) {
		set_flag(BQCASTLE);
	}

	/* Move back pieces */
	if (mov.flags() & MOVE_CASTLE) {
		move_piece(mov.to(), mov.from(), side, KING);
		switch (mov.to()) {
			case C1:
				ASSERT_DEBUG(side == WHITE);
				ASSERT_DEBUG(flags & WQCASTLE);
				move_piece(D1, A1, side, ROOK);
				break;
			case G1: 
				ASSERT_DEBUG(side == WHITE);
				ASSERT_DEBUG(flags & WKCASTLE);
				move_piece(F1, H1, side, ROOK);
				break;
			case C8:
				ASSERT_DEBUG(side == BLACK);
				ASSERT_DEBUG(flags & BQCASTLE);
				move_piece(D8, A8, side, ROOK);
				break;
			case G8:
				ASSERT_DEBUG(side == BLACK);
				ASSERT_DEBUG(flags & BKCASTLE);
				move_piece(F8, H8, side, ROOK);
				break;
			default:
				BUG("invalid 'to' square for castling: %d",
						mov.to());
				break;
		}				
		has_castled[side] = false;
	}
	else if (mov.flags() & MOVE_ENPASSANT) {
		ASSERT_DEBUG(mov.to() == epsq);
		move_piece(mov.to(), mov.from(), side, PAWN);
		place_piece(get_eppawn(), opponent, PAWN);
	}
	else if (mov.flags() & MOVE_CAPTURE) {
		if (mov.flags() & MOVE_PROMOTION) {
			remove_piece(mov.to(), side, mov.promote_to());
			place_piece(mov.to(), opponent, mov.cap_ptype());
			place_piece(mov.from(), side, PAWN);
		} else {
			move_piece(mov.to(), mov.from(), side, mov.ptype());
			place_piece(mov.to(), opponent, mov.cap_ptype());
		}
	}
	else if (mov.flags() & MOVE_NORMAL) {
		if (mov.flags() & MOVE_PROMOTION) {
			remove_piece(mov.to(), side, mov.promote_to());
			place_piece(mov.from(), side, PAWN);
		} else {
			move_piece(mov.to(), mov.from(), side, mov.ptype());
		}
	}
	else if (mov.flags() & MOVE_NULL) {
		/* nothing */
	}
	else {
		BUG("unknown move flags: %x", mov.flags());
	}

	
	ASSERT_DEBUG(memcmp(this, &hist.oldboard, sizeof(Board)) == 0);
	ASSERT_DEBUG(is_valid_move(mov));
}
#endif

/*
 * Check if a move is valid (= pseudo-legal) on this board.
 */
bool Board::is_valid_move(Move mov) const
{
	if (mov.is_null()) {
		return true;
	}
	
	Square from = mov.from();
	Square to = mov.to();
	
	/* Check castling. */
	if (mov.is_castle()) {
		ASSERT_DEBUG(mov.ptype() == KING);
		ASSERT_DEBUG(mov.from() == get_king(get_side()));

		if (in_check())
			return false;

		switch (to) {
		case G1:
			ASSERT_DEBUG(get_side() == WHITE);
			if ((get_flags() & WKCASTLE) == 0
					|| (get_blocker()
						& Bitboard::ray_bb[E1][H1])
					|| is_attacked(F1, BLACK))
				return false;
			break;
		case C1:
			ASSERT_DEBUG(get_side() == WHITE);
			if ((get_flags() & WQCASTLE) == 0
					|| (get_blocker()
						& Bitboard::ray_bb[A1][E1])
					|| is_attacked(D1, BLACK))
				return false;
			break;
		case G8:
			ASSERT_DEBUG(get_side() == BLACK);
			if ((get_flags() & BKCASTLE) == 0
					|| (get_blocker()
						& Bitboard::ray_bb[E8][H8])
					|| is_attacked(F8, WHITE))
				return false;
			break;
		case C8:
			ASSERT_DEBUG(get_side() == BLACK);
			if ((get_flags() & BQCASTLE) == 0
					|| (get_blocker()
						& Bitboard::ray_bb[A8][E8])
					|| is_attacked(D8, WHITE))
				return false;
			break;
		default:
			BUG("invalid 'to' square for castling: %d", to);
			break;
		}
		
		/* Ok, castling is allowed. */
		return true;
	}

	/* Check origin square. */
	if (color_at(from) != get_side() || piece_at(from) != mov.ptype())
		return false;

	/* Check destination square. */	
	if (mov.is_capture()) {
		if (color_at(to) != XSIDE(get_side()))
			return false;
	} else if (mov.is_enpassant()) {
		if (to != get_epsq())
			return false;
	} else {
		if (color_at(to) != NO_COLOR)
			return false;
	}

	/* Check from/to ranks if promotion. */
	if (mov.is_promotion()) {
		if (get_side() == WHITE) {
			ASSERT_DEBUG(RNK(from) == RANK7);
			ASSERT_DEBUG(RNK(to) == RANK8);
		} else {
			ASSERT_DEBUG(RNK(from) == RANK2);
			ASSERT_DEBUG(RNK(to) == RANK1);
		}
	}

	/* Check correct piece movement. */
	switch (piece_at(from)) {
	case PAWN:
		if (mov.is_capture() || mov.is_enpassant()) {
			if (!pawn_captures(from, color_at(from)).testbit(to))
				return false;
		} else {
			if (!pawn_noncaptures(from, color_at(from)).testbit(to))
				return false;
		}
		break;
	case KNIGHT:
		if (!knight_attacks(from).testbit(to))
			return false;
		break;
	case BISHOP:
		if (!bishop_attacks(from).testbit(to))
			return false;
		break;
	case ROOK:
		if (!rook_attacks(from).testbit(to))
			return false;
		break;
	case QUEEN:
		if (!queen_attacks(from).testbit(to))
			return false;
		break;
	case KING:
		if (!king_attacks(from).testbit(to))
			return false;
		break;
	default:
		BUG("huh?");
	}
	
	/* Ok, this move is pseudo-legal. */
	return true;
}

/*
 * Check if a move is legal on this board.
 */
bool Board::is_legal_move(Move mov) const
{
	ASSERT_DEBUG(is_valid_move(mov));
			
	Board tmpboard = *this;
	tmpboard.make_move(mov);
	return tmpboard.is_legal();
}

Color Board::color_at(Square sq) const
{
	for (Color clr = WHITE; clr <= BLACK; clr++)
		if (position_all[clr].testbit(sq))
			return clr;
	return NO_COLOR;
}

/*
 * Perhaps we should add an additional
 * Piece[64] array to speed up this function.
 */
Piece Board::piece_at(Square sq) const
{
	for (Piece pce = PAWN; pce <= KING; pce++)
		if ((position[WHITE][pce] | position[BLACK][pce]).testbit(sq))
			return pce;
	return NO_PIECE;
}

/*
 * Return the square where the enpassant pawn is 
 * located. This is NOT the enpassant square!
 */
Square Board::get_eppawn() const
{
	if (RNK(epsq) == RANK3)
		return epsq + 8;
	else if (RNK(epsq) == RANK6)
		return epsq - 8;
	else
		return NO_SQUARE;
}

void Board::set_side(Color _side)
{
	if (side != _side)
		switch_sides();
}

void Board::switch_sides()
{
	side = XSIDE(side);
	opponent = XSIDE(opponent);
	hashkey ^= hash_side;
	pawnhashkey ^= hash_side;
}

void Board::place_piece(Square sq, Color side, Piece ptype)
{
	ASSERT_DEBUG(color_at(sq) == NO_COLOR);
	ASSERT_DEBUG(piece_at(sq) == NO_PIECE);
	
	position[side][ptype].setbit(sq);
	position_all[side].setbit(sq);
	occupied.setbit(sq);
	occupied_l90.setbit(Bitboard::map_l90[sq]);
	occupied_l45.setbit(Bitboard::map_l45[sq]);
	occupied_r45.setbit(Bitboard::map_r45[sq]);
		
	if (ptype == KING) {
		king[side] = sq;
	}
	
	material[side] += mat_values[ptype];
	
	hashkey ^= hashkeys[side][ptype][sq];
	if (ptype == PAWN) {
		pawnhashkey ^= hashkeys[side][ptype][sq];
	}
}

void Board::remove_piece(Square sq, Color side, Piece ptype)
{
	ASSERT_DEBUG(color_at(sq) == side);
	ASSERT_DEBUG(piece_at(sq) == ptype);
	
	position[side][ptype].clearbit(sq);
	position_all[side].clearbit(sq);
	occupied.clearbit(sq);
	occupied_l90.clearbit(Bitboard::map_l90[sq]);
	occupied_l45.clearbit(Bitboard::map_l45[sq]);
	occupied_r45.clearbit(Bitboard::map_r45[sq]);

	if (ptype == KING) {
		king[side] = NO_SQUARE;
	}
	
	material[side] -= mat_values[ptype];
	
	hashkey ^= hashkeys[side][ptype][sq];
	if (ptype == PAWN) {
		pawnhashkey ^= hashkeys[side][ptype][sq];
	}
}
	
void Board::move_piece(Square from, Square to, Color side, Piece ptype)
{
	ASSERT_DEBUG(color_at(from) == side);
	ASSERT_DEBUG(piece_at(from) == ptype);
	ASSERT_DEBUG(color_at(to) == NO_COLOR);
	ASSERT_DEBUG(piece_at(to) == NO_PIECE);
	
	position[side][ptype].clearbit(from);
	position[side][ptype].setbit(to);
	position_all[side].clearbit(from);
	position_all[side].setbit(to);
	occupied.clearbit(from);
	occupied.setbit(to);
	occupied_l90.clearbit(Bitboard::map_l90[from]);
	occupied_l90.setbit(Bitboard::map_l90[to]);
	occupied_l45.clearbit(Bitboard::map_l45[from]);
	occupied_l45.setbit(Bitboard::map_l45[to]);
	occupied_r45.clearbit(Bitboard::map_r45[from]);
	occupied_r45.setbit(Bitboard::map_r45[to]);

	if (ptype == KING) {
		king[side] = to;
	}
	
	hashkey ^= hashkeys[side][ptype][from];
	hashkey ^= hashkeys[side][ptype][to];
	if (ptype == PAWN) {
		pawnhashkey ^= hashkeys[side][ptype][from];
		pawnhashkey ^= hashkeys[side][ptype][to];
	}
}

void Board::set_flag(unsigned int flag)
{
	if (flags & flag)
		return;

	flags |= flag;

	switch (flag) {
		case WKCASTLE:
			hashkey ^= hash_wk;
			break;
		case WQCASTLE:
			hashkey ^= hash_wq;
			break;
		case BKCASTLE:
			hashkey ^= hash_bk;
			break;
		case BQCASTLE:
			hashkey ^= hash_bq;
			break;
	}
}

void Board::clear_flag(unsigned int flag)
{
	if (! (flags & flag))
		return;

	flags &= ~flag;

	switch (flag) {
		case WKCASTLE:
			hashkey ^= hash_wk;
			break;
		case WQCASTLE:
			hashkey ^= hash_wq;
			break;
		case BKCASTLE:
			hashkey ^= hash_bk;
			break;
		case BQCASTLE:
			hashkey ^= hash_bq;
			break;
	}
}


void Board::set_epsq(Square sq)
{
	if (epsq != NO_SQUARE) {
		hashkey ^= hash_ep[epsq];
		//pawnhashkey ^= hash_ep[epsq];
	}
	if (sq != NO_SQUARE) {
		hashkey ^= hash_ep[sq];
		//pawnhashkey ^= hash_ep[sq];
	}
	epsq = sq;
}

void Board::clear_epsq()
{
	if (epsq != NO_SQUARE) {
		hashkey ^= hash_ep[epsq];
		//pawnhashkey ^= hash_ep[epsq];
	}
	epsq = NO_SQUARE;
}

