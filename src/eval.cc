/* $Id: eval.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/eval.cc
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


Evaluator::Evaluator()
{
	pawnhashtable = NULL;
	evalcache = NULL;
	
	reset_statistics();
}

Evaluator::~Evaluator()
{
}


/*****************************************************************************
 *
 * Main evaluation functions.
 *
 *****************************************************************************/

/* TODO
 * - This could be different for Chess and Xiangqi...
 * - This should be configurable on runtime, because test tourneys have
 *   shown that it depends on available search time.
 */
#define EVAL_CUTOFF_MATERIAL	150
#define EVAL_CUTOFF_PHASE1	  5
#define EVAL_ENABLE_PHASE2

int Evaluator::eval(const Board & board, int alpha, int beta, Color _myside)
{
	myside = _myside;
	
	int score;

	/* TODO is_draw() still empty */
#if 0
	if (is_draw(board))
		return DRAW;
#endif
	
	const Color side = board.get_side();
	const Color xside = XSIDE(side);
	STAT_INC(stat_evals);

	
	/*
	 * material 
	 */
	
	score = material_balance(board.get_material(side),
				 board.get_material(xside));
	if (score >= beta + EVAL_CUTOFF_MATERIAL
			|| score <= alpha - EVAL_CUTOFF_MATERIAL) {
		return score;
	}
	
	
	/*
	 * do normal evaluation 
	 */
	
#ifdef USE_EVALCACHE
	if (evalcache && evalcache->probe(board, &score)) {
		return score;
	}
#endif
	
	setup(&board);

	
	/*
	 * phase 1 
	 */
	{
		STAT_INC(stat_evals_phase1);
		
		/* call plugins */
		int score1 = 0;
		for (int i=0; plugins[i].name != NULL; i++) {
			ASSERT_DEBUG(plugins[i].func != NULL);
			score1 += (this->*plugins[i].func)(side)
				- (this->*plugins[i].func)(xside);
		}
		score += score1;
#ifdef EVAL_CUTOFF_PHASE1
		if (score1 > EVAL_CUTOFF_PHASE1
				|| score1 < -EVAL_CUTOFF_PHASE1) {
			goto done;
		}
#endif
	}

#ifdef EVAL_ENABLE_PHASE2
	/*
	 * phase 2 
	 */
	{
		STAT_INC(stat_evals_phase2);

		/* call plugins2 */
		int score2 = 0;
		for (int i=0; plugins2[i].name != NULL; i++) {
			ASSERT_DEBUG(plugins2[i].func != NULL);
			score2 += (this->*plugins2[i].func)(side)
				- (this->*plugins2[i].func)(xside);
		}
		score += score2;
	}
#endif

	/*
	 * done 
	 */
	goto done;	// avoid warning about unused label
done:
	finish();

#ifdef USE_EVALCACHE
	if (evalcache) {
		evalcache->put(board, score);
	}
#endif
	
	return score;
}

void Evaluator::print_eval(const Board & board, Color _myside, FILE * fp)
{
	myside = _myside;
	setup(&board);
	
	fprintf(fp, "material: %d/%d\n", 
			board.material[WHITE], board.material[BLACK]);
	fprintf(fp, "material difference: %d\n",
			board.material[WHITE] - board.material[BLACK]);
	fprintf(fp, "material balance: %d\n",
			 material_balance(board.material[WHITE],
				 	  board.material[BLACK]));
	fprintf(fp, "phase: %u\n", phase);

	fprintf(fp, "draw: %s\n", is_draw(board) ? "yes" : "no");
	fprintf(fp, "myside: %s\n",
			(myside == WHITE ? "white"
		 		: (myside == BLACK ? "black" : "none")));

	fprintf(fp, "scoring plugins:\n");
	for (int i=0; plugins[i].name != NULL; i++) {
		ASSERT(plugins[i].func != NULL);
		fprintf(fp, "\t%s: %d/%d\n", plugins[i].name,
				(this->*plugins[i].func)(WHITE),
				(this->*plugins[i].func)(BLACK));
	}
	
	fprintf(fp, "scoring plugins2:\n");
	for (int i=0; plugins2[i].name != NULL; i++) {
		ASSERT(plugins2[i].func != NULL);
		fprintf(fp, "\t%s: %d/%d\n", plugins2[i].name,
				(this->*plugins2[i].func)(WHITE),
				(this->*plugins2[i].func)(BLACK));
	}
}


/*****************************************************************************
 *
 * Utility functions.
 *
 *****************************************************************************/

void Evaluator::reset_statistics()
{
#ifdef COLLECT_STATISTICS
	stat_evals = 0;
	stat_evals_phase1 = 0;
	stat_evals_phase2 = 0;
#endif

	if (pawnhashtable) {
		pawnhashtable->reset_statistics();
	}
	
#ifdef USE_EVALCACHE
	if (evalcache) {
		evalcache->reset_statistics();
	}
#endif
}

void Evaluator::print_statistics(FILE * fp) const
{
#ifdef COLLECT_STATISTICS
	fprintf(fp, "Evaluations: %lu/%lu/%lu\n",
			stat_evals, stat_evals_phase1, stat_evals_phase2);
#endif // COLLECT_STATISTICS

	if (pawnhashtable) {
		pawnhashtable->print_statistics(fp);
	}
	
#ifdef USE_EVALCACHE
	if (evalcache) {
		evalcache->print_statistics(fp);
	}
#endif
}


void Evaluator::set_pawnhashtable(PawnHashTable * pawnhashtable)
{
	this->pawnhashtable = pawnhashtable;
}

void Evaluator::set_evalcache(EvaluationCache * evalcache)
{
#ifdef USE_EVALCACHE
	this->evalcache = evalcache;
#else
	(void) evalcache;
	WARN("This version of %s has been compiled without"
			" evaluation cache support.\n", PROGNAME);
#endif
}


void Evaluator::set_param(const std::string& name, const std::string& value)
{
	/* avoid compiler warning about unused parameter */
	(void) name;
	(void) value;

	/* TODO ... */
}


