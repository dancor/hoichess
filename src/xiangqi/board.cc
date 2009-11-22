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
	
	for (Square sq = A0; sq <= I9; sq++) {
		position_pieces[sq] = NO_PIECE;
		position_colors[sq] = NO_COLOR;
	}

	king[WHITE] = NO_SQUARE;
	king[BLACK] = NO_SQUARE;
	
//	flags = 0;

	material[WHITE] = 0;
	material[BLACK] = 0;

	for (Square sq = A0; sq <= I9; sq++) {
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
	if (mov.is_capture()) {
		remove_piece(mov.to(), opponent, mov.cap_ptype());
		move_piece(mov.from(), mov.to(), side, mov.ptype());
	}
	else if (mov.is_normal()) {
		move_piece(mov.from(), mov.to(), side, mov.ptype());
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
	
	/* Switch sides and update moveno */
	switch_sides();
	if (side == WHITE) {
		moveno++;
	}
	
	/* Update movecnt50 */
#ifdef USE_UNMAKE_MOVE
	hist.movecnt50 = movecnt50;
#endif
	if ((mov.ptype() == PAWN && RNK(mov.from()) != RNK(mov.to()))
			|| mov.is_capture()) {
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

	/* Move back pieces */
	if (mov.is_capture()) {
		move_piece(mov.to(), mov.from(), side, mov.ptype());
		place_piece(mov.to(), opponent, mov.cap_ptype());
	}
	else if (mov.is_normal()) {
		move_piece(mov.to(), mov.from(), side, mov.ptype());
	}
	else if (mov.is_null()) {
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
	
	/* Check origin square. */
	if (color_at(from) != get_side() || piece_at(from) != mov.ptype())
		return false;

	/* Check destination square. */	
	if (mov.is_capture()) {
		if (color_at(to) != XSIDE(get_side()))
			return false;
	} else {
		if (color_at(to) != NO_COLOR)
			return false;
	}

	/* Check correct piece movement. */
#if 0 /* TODO */
	switch (piece_at(from)) {
	case PAWN:
		if (mov.flags() & MOVE_CAPTURE
				|| mov.flags() & MOVE_ENPASSANT) {
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
#endif

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
	
	position_pieces[sq] = ptype;
	position_colors[sq] = side;
	
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
	
	position_pieces[sq] = NO_PIECE;
	position_colors[sq] = NO_COLOR;

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

	position_pieces[from] = NO_PIECE;
	position_colors[from] = NO_COLOR;
	position_pieces[to] = ptype;
	position_colors[to] = side;
	
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

#if 0
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
#endif

#if 0
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
#endif

