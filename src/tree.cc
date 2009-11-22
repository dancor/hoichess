/* $Id: tree.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/tree.cc
 *
 * Copyright (C) 2005 Holger Ruckdeschel <holger@hoicher.de>
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
#include "tree.h"

/*****************************************************************************
 *
 * Functions of class Node
 *
 *****************************************************************************/

Node::Node()
{
	historytable = NULL;
}

Move Node::first()
{
	current_move_no = -1;

	switch (type) {
	case ROOT:
		/* For the root node, all moves have been generated in
		 * Tree::set_root(). Scores were assigned by search_root()
		 * using set_current_score(). So we can just start returning
		 * moves. */
		state = ALL;
		return next();
		
	case FULLWIDTH:
	case QUIESCE:
		/* First, determine the next state. */
		if (in_check()) {
			/* Some moves might have already been generated
			 * due to internal iterative deepening. */
			if (escapes_generated) {
				state = SCORE_ESCAPES;
			} else {
				state = GEN_ESCAPES;
			}
		} else {
			/* Some moves might have already been generated
			 * due to internal iterative deepening. */
			if (noncaptures_generated) {
				ASSERT_DEBUG(type != QUIESCE);
				state = SCORE_ALL;
			} else if (captures_generated) {
				state = SCORE_CAPTURES;
			} else {
				state = GEN_CAPTURES;
			}
		}

		if (hashmv) {
			return hashmv;
		} else {
			return next();
		}
	
	default:
		BUG("node type is bad: %d", type);
	}

	/* This line is never reached. We add a return statement, in case
	 * the compiler does not know that BUG() will never return. */
	return NO_MOVE;
}

Move Node::next()
{
	const Board & board = tree->get_board();
	Move mov;
	
	switch (state) {
	case GEN_CAPTURES:
		ASSERT_DEBUG(!captures_generated);
		ASSERT_DEBUG(movelist.size() == 0);
		board.generate_captures(&movelist, false);
		captures_generated = true;
		//state = SCORE_CAPTURES;
	
	case SCORE_CAPTURES:
		ASSERT_DEBUG(captures_generated);
		score_moves();
		state = CAPTURES;

	case CAPTURES:
		ASSERT_DEBUG(captures_generated);
		mov = pick();
		if (mov) {
			return mov;
		}
		if (type == QUIESCE) {
			state = DONE;
			break;
		}
		//state == GEN_NONCAPTURES;
		
	case GEN_NONCAPTURES:
		ASSERT_DEBUG(captures_generated);
		ASSERT_DEBUG(!noncaptures_generated);
		board.generate_noncaptures(&movelist);
		noncaptures_generated = true;
		//state = SCORE_NONCAPTURES;
	
	case SCORE_NONCAPTURES:
		ASSERT_DEBUG(noncaptures_generated);
		score_moves();
		state = NONCAPTURES;

	case NONCAPTURES:
		ASSERT_DEBUG(noncaptures_generated);
		mov = pick();
		if (mov) {
			return mov;
		}
		state = DONE;
		break;
		
	case GEN_ESCAPES:
		ASSERT_DEBUG(in_check());
		ASSERT_DEBUG(!escapes_generated);
		ASSERT_DEBUG(movelist.size() == 0);
		//board.generate_escapes(&movelist);
		board.generate_moves(&movelist, false);
		escapes_generated = true;
		//state = SCORE_ESCAPES;

	case SCORE_ESCAPES:
		ASSERT_DEBUG(escapes_generated);
		score_moves();
		state = ESCAPES;

	case ESCAPES:
		ASSERT_DEBUG(escapes_generated);
		mov = pick();
		if (mov) {
			return mov;
		}
		state = DONE;
		break;

	case SCORE_ALL:
		score_moves();
		state = ALL;

	case ALL:
		mov = pick();
		if (mov) {
			return mov;
		}
		state = DONE;
		break;

	default:
		BUG("node status is bad: %d", state);
	}

	return NO_MOVE;
}

Move Node::pick()
{
	int score = -INFTY;
	int m = -1;
	for (unsigned int i=current_move_no+1; i<movelist.size(); i++) {
		if (movelist[i] == hashmv) {
			/* Hash move was already returned by first() */
			continue;
		} else if (movelist.get_score(i) > score) {
			score = movelist.get_score(i);
			m = i;
		}
	}

	if (m == -1) {
		return NO_MOVE;
	}

	Move mov = movelist[m];
	current_move_no++;
	if (current_move_no != m) {
		movelist.swap(current_move_no, m);
	}
	return mov;
}

/*
 * Assign scores to moves. For root node, the score are set
 * by Search::search_root() using set_current_score().
 */
void Node::score_moves()
{
	if (type == ROOT)
		return;

	for (unsigned int i=current_move_no+1; i<movelist.size(); i++) {
		int score = 0;
		Move mov = movelist[i];
		if (mov == hashmv) {
			/* The hash move is treated specially. It won't be
			 * returned by pick(), thus we do not need to assign
			 * any score to it. */
			continue;
		} else if (mov.is_capture()
#ifdef HOICHESS
				|| mov.is_promotion()
				|| mov.is_enpassant()
#endif
				) {
			/* TODO This could be improved I think */
			score += 10000
				+ mat_values[mov.cap_ptype()]
				- mat_values[mov.ptype()];
#ifdef HOICHESS
			if (mov.is_promotion()) {
				score += mat_values[mov.promote_to()];
			}
#endif
		} else {
#ifdef USE_KILLER
			/* TODO This could be improved I think */
			if (mov == killer1 || mov == killer2) {
				score += 5000;
			}
#endif
#ifdef USE_HISTORY
			/* TODO This could be improved I think */
			if (historytable) {
				score += historytable->get(mov);
			}
#endif
		}
		movelist.set_score(i, score);
	}
}


/*****************************************************************************
 *
 * Functions of class Tree
 *
 *****************************************************************************/

Tree::Tree()
{
	nodes = new Node[MAXPLY];
	for (unsigned int ply=0; ply<MAXPLY; ply++) {
		nodes[ply].tree = this;
	}
}

Tree::~Tree()
{
	delete[] nodes;
}

void Tree::clear_killer()
{
	for (unsigned int ply=0; ply<MAXPLY; ply++) {
		nodes[ply].killer1 = NO_MOVE;
		nodes[ply].killer2 = NO_MOVE;
	}
}

void Tree::set_root(const Board & board)
{
#ifdef USE_UNMAKE_MOVE
	this->board = board;
	this->rootboard = board;
#else
	nodes[0].board = board;
#endif
	current_ply = 0;
	
	nodes[0].hashkey = board.get_hashkey();
	nodes[0].incheck = board.in_check();
	nodes[0].material = board.material_difference();
	nodes[0].movelist.clear();
	board.generate_moves(&nodes[0].movelist, false);
	nodes[0].movelist.filter_illegal(board);
	ASSERT(nodes[0].movelist.size() > 0);
	nodes[0].captures_generated = true;
	nodes[0].noncaptures_generated = true;
	nodes[0].escapes_generated = true;
	
	nodes[0].set_type(Node::ROOT);
	/* Assign one legal move as best, in case search terminates without
	 * choosing a move. */
	nodes[0].set_best(nodes[0].movelist[0]);
	nodes[0].set_hashmv(NO_MOVE);
	nodes[0].played_move = NO_MOVE;
}

Node * Tree::make_move(Move mov)
{
	ASSERT_DEBUG(current_ply < MAXPLY-1);
#ifdef USE_UNMAKE_MOVE
	BoardHistory hist = board.make_move(mov);
	nodes[current_ply].hist = hist;
#else
	nodes[current_ply+1].board = nodes[current_ply].board;
	nodes[current_ply+1].board.make_move(mov);
#endif
	current_ply++;

#ifdef USE_UNMAKE_MOVE
	nodes[current_ply].hashkey = board.get_hashkey();
	nodes[current_ply].incheck = board.in_check();
	nodes[current_ply].material = board.material_difference();
#else
	nodes[current_ply].hashkey = nodes[current_ply].board.get_hashkey();
	nodes[current_ply].incheck = nodes[current_ply].board.in_check();
	nodes[current_ply].material = nodes[current_ply].board.material_difference();
#endif
	nodes[current_ply].movelist.clear();
	nodes[current_ply].captures_generated = false;
	nodes[current_ply].noncaptures_generated = false;
	nodes[current_ply].escapes_generated = false;
	
	nodes[current_ply].set_type(Node::UNKNOWN);
	nodes[current_ply].set_best(NO_MOVE);
	nodes[current_ply].set_hashmv(NO_MOVE);
	nodes[current_ply].played_move = mov;

	return &nodes[current_ply];
}

void Tree::unmake_move()
{
	ASSERT_DEBUG(current_ply > 0);
#ifdef USE_UNMAKE_MOVE
	BoardHistory hist = nodes[current_ply-1].hist;
	board.unmake_move(hist);
#endif
	current_ply--;
}

 
