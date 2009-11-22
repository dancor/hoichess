/* $Id: eval.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/xiangqi/eval.cc
 *
 * Copyright (C) 2004-2006 Holger Ruckdeschel <holger@hoicher.de>
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
#include "eval.h"
#include "board.h"

/*****************************************************************************
 *
 * Main evaluation functions.
 *
 *****************************************************************************/

/*
 * Advanced draw detection.
 * Basics are done in Search::is_draw() (repetitions, 50 move rule) 
 * and Board::is_material_draw() (insufficient material).
 */
bool Evaluator::is_draw(const Board & board)
{
	/* TODO */
	(void) board;
	return false;
}

int Evaluator::material_balance(int ms, int mxs)
{
	return ms-mxs;
}
	
/*
 * This is a first try to implement a material based phase
 * detection routine. There is a least some fine-tuning that
 * we must do.
 */
unsigned int Evaluator::get_phase(const Board & board)
{
	const int mat = board.material[WHITE] + board.material[BLACK];

	/* Starting material is 2*4800 = 9600 */
	if (mat > 8600) {
		return OPENING;
	} else if (mat > 5400) {
		return MIDGAME;
	} else {
		return ENDGAME;
	}
}

void Evaluator::setup(const Board * board)
{
	ASSERT_DEBUG(board != NULL);
	this->board = board;
	
//	const Color side = board->get_side();
//	const Color xside = XSIDE(side);

	phase = get_phase(*board);

#if 0
	if (pawnhashtable) {
		if (pawnhashtable->probe(board->get_pawnhashkey(),
					&pawnhashentry)){
			if (pawnhashentry.get_phase() == phase) {
				pawnhashtable->incr_hits2();
			}
		}
		/* probe() marks entry invalid if nothing was found in
		 * the table, so we don't need to do this here again. */
	} else {
		pawnhashentry.set_invalid();
	}
#endif

//	pinned_on_king[side] = board->pinned(board->get_king(side), side);
//	pinned_on_king[xside] = board->pinned(board->get_king(xside), xside);
}

void Evaluator::finish()
{
#if 0
	if (pawnhashtable) {
		pawnhashentry.set_hashkey(board->get_pawnhashkey());
		pawnhashentry.set_phase(phase);
		pawnhashtable->put(pawnhashentry);
	}
#endif
}


/*****************************************************************************
 * 
 * Scoring plugins
 * 
 *****************************************************************************/

const struct score_plugin Evaluator::plugins[] = {
	{ "positional",	&Evaluator::score_positional	},

	{ NULL, NULL }
};

const struct score_plugin Evaluator::plugins2[] = {
//	{ "control",	&Evaluator::score_control	},

	{ NULL, NULL }
};


const int Evaluator::positional_scores[][90] = {
	{ // PAWN
	  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,
	-10,  0,-10,  0,-10,  0,-10,  0,-10,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,
	103,105,110,110,110,110,110,105,103,
	105,108,115,115,115,115,115,108,105,
	100,105,110,120,120,120,110,105,100,
	100,100,110,120,125,120,110,100,100,
	100,100,110,120,125,120,110,100,100,
	},
	
	{ // GUARD
	 0,  0,  0,  5,  0,  5,  0,  0,  0,
	 0,  0,  0,  0, 10,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	},
	
	{ // ELEPHANT
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  5,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  3,  0,  0,  0,  3,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	},
	
	{ // KNIGHT
	-5, -8, -2, -2, -2, -2, -2, -8, -5,
	-2,  0,  0,  0,  0,  0,  0,  0, -2,
	-2,  0,  0,  0,  0,  0,  0,  0, -2,
	-2,  0,  0,  0,  0,  0,  0,  0, -2,
	-2,  0,  5,  6,  6,  6,  5,  0, -2,
	-2,  5,  6,  8,  8,  8,  6,  5, -2,
	-2,  5,  8, 10, 10, 10,  8,  5, -2,
	-2,  5, 10,  0,  0,  0, 10,  5, -2,
	-2, 10,  0,  0,  0,  0,  0, 10, -2,
	-5, 10,  0,  0,  0,  0,  0, 10, -5,
	},
	
	{ // CANNON
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  4,  5,  4,  0,  0,  0,
	 8,  8,  4,  0,  0,  0,  4,  8,  8,
	 9,  9,  5,  0,  0,  0,  5,  9,  9,
	 8,  8,  4,  0,  0,  0,  4,  8,  8,
	},
	
	{ // ROOK
	-8,  0,  0,  8,  9,  8,  0,  0, -8,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 0,  0,  0,  8,  9,  8,  0,  0,  0,
	 8,  8,  8,  4,  5,  4,  8,  8,  8,
	 9,  9,  9,  5,  5,  5,  9,  9,  9,
	 8,  8,  8,  4,  5,  4,  8,  8,  8,
	},
	
	{ // KING
	 0,  0,  0, 10, 20, 10,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,-10,-10,-10,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,
	},
};


int Evaluator::score_positional(Color side)
{
	int score = 0;

	for (Square sq=A0; sq<=I9; sq++) {
		if (board->color_at(sq) != side) {
			continue;
		}

		Piece ptype = board->piece_at(sq);
	
		/* positional score */
		Square idx = (side == WHITE) ? sq
	                        : SQUARE(XRANK(RNK(sq)), FIL(sq));
		score += positional_scores[ptype][idx];

		/* penalize repetition during opening phase */
		if (phase == OPENING) {
			score += - (board->get_pce_movecnt(sq)-1)*2;
		}
	}
	
	return score;
}
