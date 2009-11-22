/* $Id: eval.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/xiangqi/eval.h
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
#ifndef EVAL_H
#define EVAL_H

#include "common.h"
#include "board.h"
#include "evalcache.h"
#include "pawnhash.h"


/* score values */
#define INFTY		100000
#define MATE		 90000
#define DRAW		     0


class Evaluator
{
      public:
	enum game_phase {
		OPENING,
		MIDGAME,
		ENDGAME
	};

      private:
	static const struct score_plugin plugins[];
	static const struct score_plugin plugins2[];

      private:
	PawnHashTable * pawnhashtable;
	EvaluationCache * evalcache;

#ifdef COLLECT_STATISTICS
	unsigned long stat_evals;
	unsigned long stat_evals_phase1;
	unsigned long stat_evals_phase2;
#endif

      private:
	const Board * board;
	unsigned int phase;
	Color myside;
	PawnHashEntry pawnhashentry;
	//Bitboard passed_pawns[2];
	//Bitboard pinned_on_king[2];
	
      public:
	Evaluator();
	~Evaluator();

      public:
	int eval(const Board & board, int alpha, int beta, Color myside);
	void print_eval(const Board & board, Color myside, FILE * fp = stdout);

      public:
	void reset_statistics();
	void print_statistics(FILE * fp = stdout) const;
	void set_pawnhashtable(PawnHashTable * pawnhashtable);
	void set_evalcache(EvaluationCache * evalcache);
	void set_param(const std::string& name, const std::string& value);
	
      private:
	void setup(const Board * board);
	void finish();
	
      public:
	static bool is_draw(const Board & board);
	static int material_balance(int mat_side, int mat_xside);
	static unsigned int get_phase(const Board & board);

      private:
	static const int positional_scores[7][90];

      private:
	int score_positional(Color side);
};

struct score_plugin {
	const char * name;
	int (Evaluator::* func)(Color);
};

#endif // EVAL_H
