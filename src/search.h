/* $Id: search.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/search.h
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
#ifndef SEARCH_H
#define SEARCH_H

#include "common.h"
#include "board.h"
#include "book.h"
#include "clock.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "historytable.h"
#include "move.h"
#include "movelist.h"
#include "shell.h"
#include "thread.h"
#include "tree.h"

/* forward declaration */
class Shell;

class Search
{
      public:
	enum search_modes_e { MOVE, ANALYZE, PONDER };
	enum params_time_e {
		PARAM_TIME_FL = 0x01,
		PARAM_TIME_IC = 0x02,
		PARAM_TIME_NB = 0x04,
		PARAM_TIME_FH = 0x08
	};
		
      private:
	Shell * shell;
	
	/* external data structures */
	Evaluator * evaluator;
	HashTable * hashtable;
	HistoryTable * histtable[2];
	Book * book;
	
	/* information about game */
	const Game * game;
	Clock * clock;

	/* tree */
	Tree tree;
	int rootdepth;
	int maxdepth;

	unsigned int maxplyreached;
	unsigned int maxplyreached_quiesce;
	
	/* search mode */
	int mode;
	Color myside;
	
	/* parameters */
	unsigned long param_time;
	
	/* thread/control stuff */
	Mutex start_mutex;
	Mutex main_mutex;
	Thread * thread;
	bool stop;
	
	/* required for time control and basic statistics */
	unsigned long nodes;
	unsigned long nodes_quiesce;
	unsigned long next_timecheck;
	unsigned long next_update;
	
#ifdef COLLECT_STATISTICS
	/* extended statistics */
	unsigned long stat_cut;
	unsigned long stat_nullcut;
	unsigned long stat_futcut;
	unsigned long stat_xfutcut;
	unsigned long stat_razcut;
	unsigned long stat_moves_sum;
	unsigned long stat_moves_cnt;
	unsigned long stat_moves_sum_quiesce;
	unsigned long stat_moves_cnt_quiesce;
#endif

	/* overall statistics */
	unsigned long ostat_knodes;
	unsigned long ostat_csecs;
	unsigned int  ostat_depth_sum;
	unsigned int  ostat_depth_cnt;

	/* flags */
	bool showthinking;
	
      public:
	Search(Shell * shell);
	~Search();
	
      public:
	void start(Game game, int mode, Color myside = NO_COLOR);
	void start(const Board & board, const Clock & clock, int mode);
	void start_thread(Game game, int mode, Color myside = NO_COLOR);
	void stop_thread();
	static void * thread_main(void * arg);

	void interrupt();
	Move get_best();
	void set_book(Book * book);
	void set_depthlimit(unsigned int depth);
	void set_hashtable(HashTable * hashtable);
	void set_pawnhashtable(PawnHashTable * pawnhashtable);
	void set_evalcache(EvaluationCache * evalcache);
	void set_showthinking(bool x);
	Evaluator * get_evaluator() const;
	void set_param(const std::string& name, const std::string& value);
	
      private:
	void main();
	void iterate();
	int search_root(unsigned int ply, int depth, int alpha, int beta);
	int search(unsigned int ply, int depth, int extend,
			int alpha, int beta);
	int quiescence_search(unsigned int ply, int alpha, int beta);

	bool is_draw();

      private:
	void check_time();
	void reset_statistics();
	void print_statistics();
	void update_overall_statistics();
	void print_overall_statistics();
	void print_header();
	void print_thinking(unsigned int depth);
	void print_thinking_terminal(unsigned int depth);
	void print_thinking_xboard(unsigned int depth);
	void print_result(unsigned int depth, int score, char c);
	void print_result_terminal(unsigned int depth, int score, char c);
	void print_result_xboard(unsigned int depth, int score, char c);
	std::string get_best_line(unsigned int depth, char c) const;
	void clear_line();
};

#endif // SEARCH_H
