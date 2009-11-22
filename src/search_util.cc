/* $Id: search_util.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/search_util.cc
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
#include "clock.h"
#include "search.h"

#include <stdio.h>

#include <sstream>


/*****************************************************************************
 *
 * Search statistics.
 *
 *****************************************************************************/

void Search::print_statistics()
{
	int csecs = clock->get_elapsed_time();
	unsigned long nodes_total = nodes + nodes_quiesce;
	
	unsigned int perc_fw = nodes_total > 0
		? 100 * nodes / nodes_total
		: 0;
	unsigned int perc_quiesce = nodes_total > 0
		? 100 * nodes_quiesce / nodes_total
		: 0;
	printf("Nodes searched: %ld"
			" (full-width: %lu (%u%%), quiescence: %lu (%u%%))\n",
			nodes_total,
			nodes, perc_fw,
			nodes_quiesce, perc_quiesce);

	if (csecs >= 6000) {
		int mins = csecs / 6000;
		int secs = csecs % 6000 / 100;
		printf("Search time: %d:%02d", mins, secs);
	} else {
		printf("Search time: %.2fs", (float) csecs / 100);
	}
	printf(" (%.0fk nodes/s)\n",
			nodes_total / ((float) csecs / 100) / 1000);

#ifdef COLLECT_STATISTICS
	printf("Cutoffs: beta: %ld, null: %ld, fut: %ld/%ld, razor: %ld\n",
			stat_cut, stat_nullcut,
			stat_futcut, stat_xfutcut, stat_razcut);
	
	if (stat_moves_cnt > 0) {
		printf("Average branching factor in full-width search: %.2f\n",
				(float) stat_moves_sum / stat_moves_cnt);
	}
	if (stat_moves_cnt_quiesce > 0) {
		printf("Average branching factor in quiescence search: %.2f\n",
				(float) stat_moves_sum_quiesce
				/ stat_moves_cnt_quiesce);
	}
#endif // COLLECT_STATISTICS
	
	if (hashtable) {
		hashtable->print_statistics();
	}
	evaluator->print_statistics();
}

void Search::reset_statistics()
{
	nodes = 0;
	nodes_quiesce = 0;
#ifdef COLLECT_STATISTICS
	stat_cut = 0;
	stat_nullcut = 0;
	stat_futcut = 0;
	stat_xfutcut = 0;
	stat_razcut = 0;
	stat_moves_sum = 0;
	stat_moves_cnt = 0;
	stat_moves_sum_quiesce = 0;
	stat_moves_cnt_quiesce = 0;
#endif // COLLECT_STATISTICS

	if (hashtable) {
		hashtable->reset_statistics();
	}
	evaluator->reset_statistics();
}

void Search::update_overall_statistics()
{
	ostat_knodes += (nodes + nodes_quiesce) / 1000;
	ostat_csecs += clock->get_elapsed_time();

	if (rootdepth > 0) {
		ostat_depth_sum += rootdepth;
		ostat_depth_cnt++;
	}
}

void Search::print_overall_statistics()
{
	if (ostat_csecs > 0) {
		printf("Average nps: %.0fk\n",
				(float) ostat_knodes / ostat_csecs * 100);
	}

	if (ostat_depth_cnt > 0) {
		printf("Average search depth: %d\n",
				ostat_depth_sum / ostat_depth_cnt);
	}
}


/*****************************************************************************
 *
 * Thinking output.
 *
 *****************************************************************************/

void Search::print_header()
{
	printf("Depth   Ply    Time   Score      Nodes  Principal-Variation\n");
}

/*
 * Print thinking output.
 */
void Search::print_thinking(unsigned int depth)
{
	unsigned long csecs = clock->get_elapsed_time();
	next_update = csecs + 500;

	if (verbose < 3 && csecs < 300) {
		return;
	}
	
	if (shell->xboard) {
		print_thinking_xboard(depth);
	} else {
		print_thinking_terminal(depth);
	}
}

/*
 * Print thinking output to terminal:
 *
 *  9?    4.15            1109710  1. e4 (1/20)                                 
 */
void Search::print_thinking_terminal(unsigned int depth)
{	
	unsigned long csecs = clock->get_elapsed_time();
	int nodes_total = nodes+nodes_quiesce;
	int i = tree[0]->get_current_move_no();
	int n = tree[0]->get_movelist_size();
	Move mov = tree[0]->get_current_move();
	const Board & board = tree.get_rootboard();

	std::string s;
	char buf[128];
	
	/* depth */
	snprintf(buf, sizeof(buf), "%2d?   %2d/%2d", depth, maxplyreached+1, maxplyreached_quiesce+1);
	s += buf;
	
	/* search time */
	if (csecs >= 6000) {
		int mins = csecs / 6000;
		int secs = csecs % 6000 / 100;
		snprintf(buf, sizeof(buf), "  %3d:%02d", mins, secs);
	} else {
		snprintf(buf, sizeof(buf), "  %6.2f", (float) csecs / 100);
	}
	s += buf;

	/* nodes */
	snprintf(buf, sizeof(buf), "          %9d  ", nodes_total);
	s += buf;

	/* current move */
	snprintf(buf, sizeof(buf), "%d. %s%s", board.get_moveno(),
			board.get_side() == WHITE ? "" : "... ",
			mov.san(board).c_str());
	s += buf;

	/* number of current move / total number of moves */
	snprintf(buf, sizeof(buf), " (%d/%d)", i+1, n);
	s += buf;

	clear_line();
	atomic_printf("%s\r", s.c_str());
	fflush(stdout);
}

/*
 * In xboard mode, when analyzing, we print out the stat01 line:
 *
 * stat01: time nodes ply mvleft mvtot mvname
 */
void Search::print_thinking_xboard(unsigned int depth)
{
	if (mode != ANALYZE) {
		return;
	}

	unsigned long csecs = clock->get_elapsed_time();
	int nodes_total = nodes+nodes_quiesce;
	int i = tree[0]->get_current_move_no();
	int n = tree[0]->get_movelist_size();
	Move mov = tree[0]->get_current_move();
	const Board & board = tree.get_rootboard();

	atomic_printf("stat01: %d %d %d %d %d %s\n",
			csecs,
			nodes_total,
			depth,
			n - i - 1,
			n,
			mov.san(board).c_str());
}

/*
 * Print search result.
 *
 * TODO Replace 'char c' by an enum type.
 */
void Search::print_result(unsigned int depth, int score, char c)
{
	int csecs = clock->get_elapsed_time();

	if (!shell->xboard && verbose < 2 && csecs < 100 && score < MATE) {
		return;
	}
	
	if (shell->xboard) {
		if (verbose < 2 && c == ' ') {
			return;
		}
		print_result_xboard(depth, score, c);
	} else {
		if (verbose < 2 && depth < 2 && c == ' ') {
			return;
		}
		print_result_terminal(depth, score, c);
	}
}

/*
 * Print search result to terminal.
 * 
 *  7.    1.16    0.25     261225  1. e4 e5 2. Nf3 Nf6 3. Nxe5 Bd6 4. d4
 */
void Search::print_result_terminal(unsigned int depth, int score, char c)
{
	int csecs = clock->get_elapsed_time();
	int nodes_total = nodes+nodes_quiesce;
	
	std::string s;
	char buf[128];
	
	/* depth and result type */
	snprintf(buf, sizeof(buf), "%2d%c   %2d/%2d", depth, c, maxplyreached+1, maxplyreached_quiesce+1);
	s += buf;
	
	/* search time */
	if (csecs >= 6000) {
		int mins = csecs / 6000;
		int secs = csecs % 6000 / 100;
		snprintf(buf, sizeof(buf), "  %3d:%02d", mins, secs);
	} else {
		snprintf(buf, sizeof(buf), "  %6.2f", (float) csecs / 100);
	}
	s += buf;

	/* score and nodes */
	snprintf(buf, sizeof(buf), " %7.2f  %9d  ",
			(float) score / 100,
			nodes_total);
	s += buf;

	/* pv line */
	s += get_best_line(depth, c);
	
	clear_line();
	atomic_printf("%s\n", s.c_str());
	fflush(stdout);
}

/*
 * Print search result in xboard mode:
 * 
 * ply score time nodes pv
 */
void Search::print_result_xboard(unsigned int depth, int score, char c)
{
	(void) c;

	int csecs = clock->get_elapsed_time();
	int nodes_total = nodes+nodes_quiesce;
	
	atomic_printf("%d %d %d %d %s\n",
			depth,
			score,
			csecs,
			nodes_total,
			get_best_line(depth, c).c_str());
}

/*
 * Return the line of best moves starting the root of the search tree.
 */
std::string Search::get_best_line(unsigned int depth, char c) const
{
	std::ostringstream ss;
	
	Board board = tree.get_rootboard();
	Move mov = tree[0]->get_best();
	
	ss << board.get_moveno() << ". ";
	if (board.get_side() == BLACK) {
		ss << "... ";
	}
	ss << mov.san(board);

	if (c == '+') {
		ss << "!!";
	} else if (c == '-') {
		ss << "??";
	}
	
	depth--;
	int ply = 0;
	
	/* Get the remaining moves from the hash table. */
	if (!hashtable) {
		return ss.str();
	}

	while (depth-- > 0) {
		board.make_move(mov);
		HashEntry entry;
		if (!hashtable->probe(board, &entry)) {
			break;
		}
		
		mov = entry.get_move();
		if (!mov) {
			break;
		}

		if (board.get_side() == WHITE) {
			ss << " " << board.get_moveno() << ". ";
		} else {
			ss << " ";
		}

		ss << mov.san(board);
		
		/* Limit output length to avoid xboard buffer overflow. */
		if (++ply >= 30) {
			ss << " [...]";
			break;
		}
	}

	return ss.str();
}

void Search::clear_line()
{
	std::string s(79, ' ');
	atomic_printf("\r%s\r", s.c_str());
}


/*****************************************************************************
 *
 * These functions will be called by the shell to configure and control
 * the search.
 *
 *****************************************************************************/

void Search::set_book(Book * book)
{
	this->book = book;
}

void Search::set_depthlimit(unsigned int depth)
{
	if (depth > 0 && depth < MAXDEPTH) {
		maxdepth = depth;
	} else {
		maxdepth = MAXDEPTH;
	}
}

void Search::set_hashtable(HashTable * hashtable)
{
	this->hashtable = hashtable;
}

void Search::set_pawnhashtable(PawnHashTable * pawnhashtable)
{
	evaluator->set_pawnhashtable(pawnhashtable);
}

void Search::set_evalcache(EvaluationCache * evalcache)
{
	evaluator->set_evalcache(evalcache);
}

void Search::set_showthinking(bool x)
{
	showthinking = x;
}

Evaluator * Search::get_evaluator() const
{
	return evaluator;
}

void Search::set_param(const std::string& name, const std::string& value)
{
	if (name == "time") {
		unsigned long v;
		if (sscanf(value.c_str(), "%lx", &v) == 1) {
			param_time = v;
			printf("param_time = 0x%lx\n", param_time);
		} else {
			printf("Illegal argument for parameter '%s': '%s'\n",
					name.c_str(), value.c_str());
		}
	}
}
