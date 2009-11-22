/* $Id: eval.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/chess/eval.cc
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
#include "bitboard.h"
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

#if 1
/*
 * Calculate material balance. To discourage piece tradeing 
 * for side that has less material, we multiply the actual
 * material difference by a factor that gets higher when
 * the total amount of material is lower:
 *
 *       / max_mat - (mat[s]+mat[xs])     \
 * bal = | -------------------------- + 1 | * (mat[s]-mat[xs])
 *       \         max_mat * k            /
 *
 * k is some scaling constant, k=8 seems to give good results.
 * max_mat is the maximum possible material value (2*3950 = 7900).
 * However, to make use of fast shift operations, we define
 * max_mat = 8192.
 *
 * Example: Let's say xside is one pawn behind, e.g.
 * 	ms = 1200, mxs = 1100 => bal = 108
 * Now a rook is traded:
 * 	ms =  700, mxs =  600 => bal = 110
 * Obviously, tradeing a rook is bad for the side that has less material.
 *
 * To avoid floating point calculations, we slightly transform above 
 * equation so that the division becomes the last operation.
 */
int Evaluator::material_balance(int ms, int mxs)
{
	const int k = 8;
	const int max_mat = 8192;
	
	int bal = (max_mat - (ms+mxs) + max_mat * k) * (ms-mxs) / (max_mat * k);
	return bal;
}
#else
int Evaluator::material_balance(int ms, int mxs)
{
	return ms-mxs;
}
#endif
	
/*
 * This is a first try to implement a material based phase
 * detection routine. There is a least some fine-tuning that
 * we must do.
 */
unsigned int Evaluator::get_phase(const Board & board)
{
	const int mat = board.material[WHITE] + board.material[BLACK];

	/* Starting material is 7900 */
	if (mat > 7000) {
		return OPENING;
	} else if (mat > 3200) {
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
	
//	pinned_on_king[side] = board->pinned(board->get_king(side), side);
//	pinned_on_king[xside] = board->pinned(board->get_king(xside), xside);
}

void Evaluator::finish()
{
	if (pawnhashtable) {
		pawnhashentry.set_hashkey(board->get_pawnhashkey());
		pawnhashentry.set_phase(phase);
		pawnhashtable->put(pawnhashentry);
	}
}
	

/*****************************************************************************
 * 
 * Scoring plugins
 * 
 *****************************************************************************/

const struct score_plugin Evaluator::plugins[] = {
	{ "pawns",	&Evaluator::score_pawns		},
	{ "knights",	&Evaluator::score_knights	},
	{ "bishops",	&Evaluator::score_bishops	},
	{ "rooks",	&Evaluator::score_rooks		},
	{ "queens",	&Evaluator::score_queens	},
	{ "king",	&Evaluator::score_king		},
	{ "devel",	&Evaluator::score_devel		},
	{ "combo",	&Evaluator::score_combo		},

	{ NULL, NULL }
};

const struct score_plugin Evaluator::plugins2[] = {
	{ "control",	&Evaluator::score_control	},

	{ NULL, NULL }
};


const int Evaluator::pawn_scores_opening[] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  8,  5,  8,-10,-15,  8,  5,  8,
	 -5,  5,  0, -8, -8,  0,  5, -5,
	 -5, -5, -5, 24, 32, -5, -5, -5,
	  0,  0,  0, 12, 12,  0,  0,  0,
	  0,  0,  8,  8,  8,  8,  0,  0,
	  0,  8,  8,  8,  8,  8,  8,  0,
	  0,  0,  0,  0,  0,  0,  0,  0
};

const int Evaluator::pawn_scores_midgame[] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  5,  5,  4,-25,-25,  4,  5,  5,
	  0,  0,  0,-10,-10,  0,  0,  0,
	  0,  0,  0, 32, 32,  0,  0,  0,
	  0,  0,  0, 12, 12,  0,  0,  0,
	  0,  0,  8,  8,  8,  8,  0,  0,
	  0,  8,  8,  8,  8,  8,  8,  0,
	  0,  0,  0,  0,  0,  0,  0,  0
};

const int Evaluator::pawn_scores_endgame[] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,
	  2,  2,  2,  2,  2,  2,  2,  2,
	  4,  4,  4,  4,  4,  4,  4,  4,
	  8,  8,  8,  8,  8,  8,  8,  8,
	 12, 12, 12, 12, 12, 12, 12, 12,
	 16, 16, 16, 16, 16, 16, 16, 16,
	  0,  0,  0,  0,  0,  0,  0,  0
};

const int Evaluator::knight_scores[] = {
	-15, -5, -5, -5, -5, -5, -5,-15,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	 -5,  0,  5, 10, 10,  5,  0, -5,
	 -5,  0,  5, 10, 10,  5,  0, -5,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	 -5,  0,  0,  0,  0,  0,  0, -5,
	-15, -5, -5, -5, -5, -5, -5,-15
};

const int Evaluator::king_scores[] = {
	 12, 12, 10,  0,  0, 10, 16, 16,
	  0,-70,-70,-70,-70,-70,-70,  0,
	  0,-70,-75,-75,-75,-75,-70,  0,
	  0,-70,-75,-80,-80,-75,-70,  0,
	  0,-70,-75,-80,-80,-75,-70,  0,
	  0,-70,-75,-75,-75,-75,-70,  0,
	  0,-70,-70,-70,-70,-70,-70,  0,
	 12, 12, 10,  0,  0, 10, 16, 16
};

const int Evaluator::king_scores_endgame[] = {
	  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  5,  5,  5,  5,  5,  5,  0,
	  0,  5, 15, 15, 15, 15,  5,  0,
	  0,  5, 15, 20, 20, 15,  5,  0,
	  0,  5, 15, 20, 20, 15,  5,  0,
	  0,  5, 15, 15, 15, 15,  5,  0,
	  0,  5,  5,  5,  5,  5,  5,  0,
	  0,  0,  0,  0,  0,  0,  0,  0
};


/*
 * Pawn evaluation.
 */

#define EVAL_DOUBLEPAWNS	-5	/* TODO perhaps -50 ? */
//#define EVAL_EIGHTPAWNS		-10
//#define EVAL_PAWNRAMS		-10
#define EVAL_ISOLATEDPAWN	-10
#define EVAL_PASSEDPAWN(dist)	(25 + 80/(dist))
#define EVAL_CONNECTEDPP	20

int Evaluator::score_pawns(Color side)
{
	int score = 0;

	/* 
	 * First look if pawn hash table probe was successful.
	 */
	if (pawnhashentry.is_valid() && pawnhashentry.get_phase() == phase) {
		passed_pawns[side] = pawnhashentry.get_passed(side);
		return pawnhashentry.get_score(side);
	} else {
		passed_pawns[side] = NULLBITBOARD;
	}

	
	/*
	 * Do normal pawn evaluation.
	 */

	Square sq;
	Bitboard pawns = board->get_pawns(side);

#ifdef EVAL_DOUBLEPAWNS
	/* Penalize doubled pawns */
	for (int f=FILEA; f<=FILEH; f++) {
		if ((pawns & Bitboard::file[f]).popcnt() > 1) {
			score += EVAL_DOUBLEPAWNS;
		}
	}
#endif

#ifdef EVAL_EIGHTPAWNS
	/* Penalize having eight pawns */
	if (side == myside  &&  pawns.popcnt() == 8) {
		score += EVAL_EIGHTPAWNS;
	}
#endif

	while (pawns) {
		sq = pawns.firstbit();
		pawns.clearbit(sq);

		/* Positional score */
		const Square idx = (side == WHITE) ? sq
			: SQUARE(XRANK(RNK(sq)),FIL(sq));
		if (phase == ENDGAME) {
			score += pawn_scores_endgame[idx];
		} else if (phase == MIDGAME) {
			score += pawn_scores_midgame[idx];
		} else {
			score += pawn_scores_opening[idx];
		}

#ifdef EVAL_PAWNRAMS
		/* Pawn rams */
		if (side == myside) {
			Square ram_sq = (side == WHITE) ? sq+8 : sq-8;
			if (board->get_pawns(XSIDE(side)).testbit(ram_sq)) {
				score += EVAL_PAWNRAMS;
			}
		}
#endif

#ifdef EVAL_ISOLATEDPAWN
		/* Isolated pawn? */
		if (! (Bitboard::isolated_pawn_mask[sq]
					& board->get_pawns(side))) {
			score += EVAL_ISOLATEDPAWN;
		}
#endif

#ifdef EVAL_PASSEDPAWN
		/* Passed pawn? */ 
		Bitboard pp_mask = Bitboard::passed_pawn_mask[side][sq];
		if (! (pp_mask & board->get_pawns(XSIDE(side)))) {
			passed_pawns[side].setbit(sq);

			/* Bonus for small distance to promotion rank. */
			const int rank8 = (side == WHITE) ? RANK8 : RANK1;
			const int dist = abs(rank8 - RNK(sq));
			ASSERT_DEBUG(dist > 0);
			score += EVAL_PASSEDPAWN(dist);

#ifdef EVAL_CONNECTEDPP
			/* Connected passed pawns. We look for passed pawns
			 * that are already determined and stored in 
			 * passed_pawns[]. This will always work, no matter
			 * in which order we process our pawns. */
			if (passed_pawns[side] 
					& Bitboard::connected_pawn_mask[sq]) {
				score += EVAL_CONNECTEDPP;
			}			
#endif
		}
#endif /* EVAL_PASSEDPAWN */
	}
	
	pawnhashentry.set_score(side, score);
	pawnhashentry.set_passed(side, passed_pawns[side]);

	return score;
}


/*
 * Knight evaluation.
 */

#define EVAL_KNIGHTMOBILITY	2
//#define EVAL_PINNEDKNIGHT	-30

int Evaluator::score_knights(Color side)
{
	int score = 0;

	Square sq;
	Bitboard knights = board->get_knights(side);
	while (knights) {
		sq = knights.firstbit();
		knights.clearbit(sq);

		/* Positional score */
		score += knight_scores[sq];

#ifdef EVAL_KNIGHTMOBILITY
		/* Simple mobility bonus */
		Bitboard ka = board->knight_attacks(sq)
				& ~board->get_pieces(side);
		score += ka.popcnt() * EVAL_KNIGHTMOBILITY;
#endif

#ifdef EVAL_PINNEDKNIGHT
		/* Pinned knight? */
		if (pinned_on_king[side].testbit(sq)) {
			score += EVAL_PINNEDKNIGHT;
		}
#endif
	}
	
	return score;
}


/* 
 * Bishop evaluation.
 */

#define EVAL_BISHOPMOBILITY	2
//#define EVAL_PINNEDBISHOP	-30
#define EVAL_BISHOPPAWN		25
#define EVAL_FIANCHETTOBISHOP	15

int Evaluator::score_bishops(Color side)
{
	int score = 0;

	Square sq;
	Bitboard bishops = board->get_bishops(side);
	while (bishops) {
		sq = bishops.firstbit();
		bishops.clearbit(sq);

#ifdef EVAL_BISHOPMOBILITY
		/* Simple mobility bonus */
		Bitboard ba = board->bishop_attacks(sq)
				& ~board->get_pieces(side);
		score += ba.popcnt() * EVAL_BISHOPMOBILITY;
#endif
		
#ifdef EVAL_PINNEDBISHOP
		/* Pinned bishop? */
		if (pinned_on_king[side].testbit(sq)) {
			score += EVAL_PINNEDBISHOP;
		}
#endif

#ifdef EVAL_BISHOPPAWN
		/* Bishop protected by pawn(s)? */
		Bitboard bp = board->pawn_captures(sq, XSIDE(side))
			& board->get_pawns(side);
		score += bp.popcnt() * EVAL_BISHOPPAWN;
#endif

#ifdef EVAL_FIANCHETTOBISHOP
		/* Fianchetto Bishop */
		if (       (side == WHITE && (sq == B2 || sq == G2))
			|| (side == BLACK && (sq == B7 || sq == G7))) {
			score += EVAL_FIANCHETTOBISHOP;
		}
#endif
	}
	
	return score;
}


/* 
 * Rook evaluation.
 */

#define EVAL_ROOKMOBILITY	2
#define EVAL_ROOKOPENFILE	10
#define EVAL_ROOKHALFOPENFILE	5
#define EVAL_ROOK7PAWNS7	20	// FIXME too high?
#define EVAL_ROOK7KING8		50	// FIXME too high?
#define EVAL_ROOKINFRONTPP	-15
#define EVAL_ROOKBEHINDPP	25
//#define EVAL_PINNEDROOK		-50

int Evaluator::score_rooks(Color side)
{
	int score = 0;

	const Color xside = XSIDE(side);
	const int rank7 = (side == WHITE) ? RANK7 : RANK2;
	const int rank8 = (side == WHITE) ? RANK8 : RANK1;

	Square sq;
	Bitboard rooks = board->get_rooks(side);
	while (rooks) {
		sq = rooks.firstbit();
		rooks.clearbit(sq);

#ifdef EVAL_ROOKMOBILITY
		/* Simple mobility bonus */
		Bitboard ra = board->rook_attacks(sq)
				& ~board->get_pieces(side);
		score += ra.popcnt() * EVAL_ROOKMOBILITY;
#endif
		
#if defined(EVAL_ROOKOPENFILE) && defined(EVAL_ROOKHALFOPENFILE)
		/* Rook on open/half-open file */
		if (! (Bitboard::file[FIL(sq)] & board->get_pawns(side)) ) {
			if (! (Bitboard::file[FIL(sq)]
						& board->get_pawns(xside)) ) {
				score += EVAL_ROOKOPENFILE;
			} else {
				score += EVAL_ROOKHALFOPENFILE;
			}
		}
#endif

#if defined(EVAL_ROOK7PAWNS7) || defined(EVAL_ROOK7KING8)
		/* Rook on 7th rank and ...*/
		if (phase != ENDGAME && RNK(sq) == rank7) {
#ifdef EVAL_ROOK7PAWNS7
			/* ... enemy pawns on 7th rank */
			if (board->get_pawns(XSIDE(side)) 
					& Bitboard::rank[rank7]) {
				score += EVAL_ROOK7PAWNS7;
			}
#endif

#ifdef EVAL_ROOK7KING8
			/* ... enemy king on 8th rank */
			if (RNK(board->get_king(XSIDE(side))) == rank8) {
				score += EVAL_ROOK7KING8;
			}
#endif
		}
#endif

#if defined(EVAL_ROOKINFRONTPP) || defined(EVAL_ROOKBEHINDPP)
		Bitboard pps = passed_pawns[side] & Bitboard::file[FIL(sq)];
		while (pps) {
			Square pp = pps.firstbit();
			pps.clearbit(pp);

#ifdef EVAL_ROOKINFRONTPP
			/* Rook in front of passed pawn */
			if (	(side == WHITE && RNK(sq) > RNK(pp))
			     || (side == BLACK && RNK(sq) < RNK(pp))) {
					score += EVAL_ROOKINFRONTPP;
			}
#endif

#ifdef EVAL_ROOKBEHINDPP
			/* Rook behind passed pawn */
			if (	(side == WHITE && RNK(sq) < RNK(pp))
			     || (side == BLACK && RNK(sq) > RNK(pp))) {
					score += EVAL_ROOKBEHINDPP;
			}
#endif
		}
#endif /* defined(EVAL_ROOKINFRONTPP) || defined(EVAL_ROOKBEHINDPP) */ 

#ifdef EVAL_PINNEDROOK
		/* Pinned rook? */
		if (pinned_on_king[side].testbit(sq)) {
			score += EVAL_PINNEDROOK;
		}
#endif
	}

	return score;
}


/*
 * Queen evaluation.
 */

//#define EVAL_QUEENNOTPRESENT	-40
#define EVAL_QUEENMOBILITY	1
//#define EVAL_QUEENNEARENEMYKING	5
//#define EVAL_PINNEDQUEEN	-90

int Evaluator::score_queens(Color side)
{
	int score = 0;

	Square sq;
	Bitboard queens = board->get_queens(side);
	
#ifdef EVAL_QUEENNOTPRESENT
	if (side == myside  &&  !queens) {
		score += EVAL_QUEENNOTPRESENT;
		return score;
	}
#endif
	
	while (queens) {
		sq = queens.firstbit();
		queens.clearbit(sq);
		
#ifdef EVAL_QUEENMOBILITY
		/* Simple mobility bonus */
		Bitboard qa = board->queen_attacks(sq)
				& ~board->get_pieces(side);
		score += qa.popcnt() * EVAL_QUEENMOBILITY;
#endif
		
#ifdef EVAL_PINNEDQUEEN
		/* Pinned queen? */
		if (pinned_on_king[side].testbit(sq)) {
			score += EVAL_PINNEDQUEEN;
		}
#endif

#ifdef EVAL_QUEENNEARENEMYKING
		/* Queen near enemy king */
		Square xking = board->get_king(XSIDE(side));
		unsigned int dist = sq_distance(sq, xking);
		if (dist < 5) {
			score += dist * EVAL_QUEENNEARENEMYKING;
		}
#endif
	}

	return score;
}


/* 
 * King evaluation.
 */

//#define EVAL_SQAROUNDKINGATKD	-4

int Evaluator::score_king(Color side)
{
	int score = 0;

	const Square kingsq = board->get_king(side);

	if (phase == ENDGAME) {
		score += king_scores_endgame[kingsq];
	} else {
		score += king_scores[kingsq];
	}

#ifdef EVAL_SQAROUNDKINGATKD
	/* Enemy pieces attacking squares around king */
	Bitboard attackers = NULLBITBOARD;
	Bitboard bb = Bitboard::attack_bb[KING][kingsq];
	while (bb) {
		Square sq = bb.firstbit();
		bb.clearbit(sq);

		attackers |= board->attackers(sq, XSIDE(side));
	}
	score += attackers.popcnt() * EVAL_SQAROUNDKINGATKD;
#endif

	return score;
}


/*
 * Evaluation of development (in opening phase).
 */

#define EVAL_MINORNOTDEV	-15
#define EVAL_EARLYROOKMOVE	-20
#define EVAL_EARLYQUEENMOVE	-25

#define EVAL_CASTLED		32
#define EVAL_CANCASTLE		16

int Evaluator::score_devel(Color side)
{
	int score = 0;

	if (phase != OPENING)
		return score;
	
#ifdef EVAL_MINORNOTDEV
	/* Penalize any unmoved knights/bishops. */
	Bitboard minor = board->get_knights(side) | board->get_bishops(side);
	while (minor) {
		Square sq = minor.firstbit();
		minor.clearbit(sq);

		if (board->get_pce_movecnt(sq) == 0) {
			score += EVAL_MINORNOTDEV;
		}
	}
#endif

#ifdef EVAL_EARLYROOKMOVE	
	/* Penalize early rook moves. */
	Bitboard rooks = board->get_rooks(side);
	while (rooks) {
		Square sq = rooks.firstbit();
		rooks.clearbit(sq);

		if (board->get_pce_movecnt(sq) > 0) {
			score += EVAL_EARLYROOKMOVE;
		}
	}
#endif

#ifdef EVAL_EARLYQUEENMOVE	
	/* Penalize early queen moves. */
	Bitboard queens = board->get_queens(side);
	while (queens) {
		Square sq = queens.firstbit();
		queens.clearbit(sq);

		if (board->get_pce_movecnt(sq) > 0) {
			score += EVAL_EARLYQUEENMOVE;
		}
	}
#endif
	
#if defined(EVAL_CASTLED) && defined(EVAL_CANCASTLE)
	/* Give a bonus for being castled, and a smaller bonus for
	 * still being available to. */
	if (board->has_castled[side]) {
		score += EVAL_CASTLED;
	} else if (board->flags & (side == WHITE ? WCASTLE : BCASTLE)) {
		score += EVAL_CANCASTLE;
	}
#endif

	return score;
}


/*
 * Evaluation of mixed-piece combinations.
 */

#define EVAL_QBCOMBO		15
#define EVAL_QRCOMBO		30

int Evaluator::score_combo(Color side)
{
	int score = 0;

	/* Look for bishop/queen and rook/queen combo. */
	Bitboard queens = board->get_queens(side);
	while (queens) {
		Square q = queens.firstbit();
		queens.clearbit(q);

#ifdef EVAL_QBCOMBO
		/* bishop/queen */
		Bitboard bq_bb = board->bishop_attacks(q)
				& board->get_bishops(side);
		while (bq_bb) {
			Square b = bq_bb.firstbit();
			bq_bb.clearbit(b);
			
			if (! (board->get_blocker() & Bitboard::ray_bb[q][b])) {
				score += EVAL_QBCOMBO;
			}
		}
#endif

#ifdef EVAL_QRCOMBO
		/* rook/queen */
		Bitboard rq_bb = board->rook_attacks(q)
				& board->get_rooks(side);
		while (rq_bb) {
			Square r = rq_bb.firstbit();
			rq_bb.clearbit(r);
			
			if (! (board->get_blocker() & Bitboard::ray_bb[q][r])) {
				score += EVAL_QRCOMBO;
			}
		}
#endif
	}

	return score;
}


/*
 * Control over board.
 */

const int Evaluator::control_score[] = {
	  1,  1,  1,  1,  1,  1,  1,  1,
	  1,  2,  2,  2,  2,  2,  2,  1,
	  1,  2,  3,  3,  3,  3,  2,  1,
	  1,  2,  3,  4,  4,  3,  2,  1,
	  1,  2,  3,  4,  4,  3,  2,  1,
	  1,  2,  3,  3,  3,  3,  2,  1,
	  1,  2,  2,  2,  2,  2,  2,  1,
	  1,  1,  1,  1,  1,  1,  1,  1
};

const unsigned int Evaluator::control_maxattackers[] = {
	  5,  5,  5,  5,  5,  5,  5,  5,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  5,  5,  7,  7,  7,  7,  5,  5,
	  5,  5,  7,  8,  8,  7,  5,  5,
	  5,  5,  7,  8,  8,  7,  5,  5,
	  5,  5,  7,  7,  7,  7,  5,  5,
	  5,  5,  5,  5,  5,  5,  5,  5,
	  5,  5,  5,  5,  5,  5,  5,  5
};

int Evaluator::score_control(Color side)
{
	int score = 0;

	for (Square sq = A1; sq <= H8; sq++) {
		Bitboard attackers = board->attackers(sq, side);
		unsigned int nr_attackers = attackers.popcnt();
		
		score += control_score[sq]
			* MIN(nr_attackers, control_maxattackers[sq]);
	}

	return score;
}
