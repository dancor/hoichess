/* $Id: search.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/search.cc
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
#include "search.h"

#include <stdio.h>


/*****************************************************************************
 *
 * Constructor / Destructor
 *
 *****************************************************************************/

Search::Search(Shell * shell)
{
	this->shell = shell;

	evaluator = new Evaluator();
	hashtable = NULL;
	histtable[WHITE] = new HistoryTable();
	histtable[BLACK] = new HistoryTable();

	game = NULL;
	clock = NULL;
	
	maxdepth = MAXDEPTH;

	param_time = 0;
	
	thread = NULL;
	stop = false;

	ostat_knodes = 0;
	ostat_csecs = 0;
	ostat_depth_sum = 0;
	ostat_depth_cnt = 0;
	
	showthinking = false;
}

Search::~Search()
{
	delete evaluator;
	delete histtable[WHITE];
	delete histtable[BLACK]; 
}


/* After how many nodes (full-width search only) should we check time?
 * Checking time (i.e. calling gettimeofday()) at every node would cause a
 * lot of overhead due to system calls.
 * This value works very well on a Pentium III 800 MHz. If you have a
 * considerably slower machine, try to set this a bit lower. */
#define TIMECHECK_INTERVAL 1000


/*****************************************************************************
 * 
 * These functions will be called by the shell
 * to set up and control the search.
 *
 *****************************************************************************/

void Search::start(Game _game, int _mode, Color _myside)
{
	DBG(2, "locking start_mutex");
	start_mutex.lock();
	DBG(2, "locked start_mutex");

	ASSERT(!thread);
	ASSERT(game == NULL);
	game = &_game;
	mode = _mode;
	myside = _myside;

	stop = false;
	main();
	stop = false;

	game = NULL;
	
	DBG(2, "unlocking start_mutex");
	start_mutex.unlock();
	DBG(2, "unlocked start_mutex");
}

void Search::start(const Board & board, const Clock & clock, int mode)
{
	Game game(board, clock, clock);
	game.start();
	start(game, mode, NO_COLOR);
}

void Search::start_thread(Game _game, int _mode, Color _myside)
{
	DBG(2, "locking start_mutex");
	start_mutex.lock();
	DBG(2, "locked start_mutex");

	if (!thread) {
		DBG(2, "starting thread");
		ASSERT(game == NULL);
		game = new Game(_game);
		mode = _mode;
		myside = _myside;
		stop = false;
		thread = new Thread(thread_main);
		thread->start(this);
	} else {
		DBG(2, "thread already running");
	}
	
	DBG(2, "unlocking start_mutex");
	start_mutex.unlock();
	DBG(2, "unlocked start_mutex");
}

void Search::stop_thread()
{
	DBG(2, "locking start_mutex");
	start_mutex.lock();
	DBG(2, "locked start_mutex");
	
	if (thread) {
		interrupt();
		DBG(2, "waiting for thread to terminate");
		thread->wait();
		DBG(2, "thread has terminated");
		delete thread;
		thread = NULL;
		delete game;
		game = NULL;
	} else {
		DBG(2, "thread not running");
	}
	
	DBG(2, "unlocking start_mutex");
	start_mutex.unlock();
	DBG(2, "unlocked start_mutex");
}

void * Search::thread_main(void * arg)
{
	Search * search = (Search *) arg;
	search->main();
	return NULL;
}

void Search::interrupt()
{
	DBG(2, "interrupt");
	stop = true;
}

Move Search::get_best()
{
	DBG(2, "locking main_mutex");
	main_mutex.lock();
	DBG(2, "locked main_mutex");
	
	Move tmp = tree[0]->get_best();
	
	DBG(2, "unlocking main_mutex");
	main_mutex.unlock();
	DBG(2, "unlocked main_mutex");

	return tmp;
}

void Search::main()
{
	DBG(2, "locking main_mutex");
	main_mutex.lock();
	DBG(2, "locked main_mutex");

	if (mode != MOVE) {
		clock = new Clock();
		clock->start();
	} else {
		clock = new Clock(game->get_clock());
	}		
	next_timecheck = TIMECHECK_INTERVAL;
	next_update = 0;

	reset_statistics();

	histtable[WHITE]->reset();
	histtable[BLACK]->reset();
	tree.clear_killer();

	/* Initialize the root node. set_root() already assigns one legal
	 * move as best, in case search terminates without choosing a move. */
	tree.set_root(game->get_board());
	rootdepth = 0;
	const Board & rootboard = tree.get_rootboard();
	int rooteval = evaluator->eval(rootboard, -INFTY, INFTY, myside);
	
	if (verbose >= 2) {
		printf("===================================================\n");
		printf("Root node evaluation: %.2f\n",
				(float) rooteval / 100);
		evaluator->print_eval(rootboard, myside);
		printf("---------------------------------------------------\n");
		clock->print();
		printf("===================================================\n");
	} else if (verbose >= 1) {
		printf("Root node evaluation: %.2f\n",
				(float) rooteval / 100);
	}
	
	if (mode == MOVE) {
		clock->allocate_time();

		/* Extend search time when in check. */
		if ((param_time & PARAM_TIME_IC) && rootboard.in_check()) {
			clock->allocate_more_time("in check at root node");
		}
		
		/* Extend search time when near book. */
		unsigned int lbm = game->last_bookmove();
		if ((param_time & PARAM_TIME_NB) && book != NULL
				&& lbm > 0 && lbm <= 3) {
			clock->allocate_more_time("near book");
		}
	}

	if (showthinking && !shell->xboard) {
		print_header();
	}

	/* If there is only one move, don't waste any time searching it. */
	ASSERT(tree[0]->get_movelist_size() > 0);
	if (tree[0]->get_movelist_size() == 1 && mode == MOVE) {
		if (showthinking) {
			print_result(0, rooteval, '.');
		}
	} else {
		iterate();
	}

	if (verbose || (showthinking && !shell->xboard)) {
		print_statistics();
		update_overall_statistics();
		print_overall_statistics();
	}
	
	delete clock;
	
	DBG(2, "unlocking main_mutex");
	main_mutex.unlock();
	DBG(2, "unlocked main_mutex");
}

/*****************************************************************************
 *
 * Tree Search Functions.
 *
 *****************************************************************************/

/* Aspiration window for iterative deepening */
#define WINDOW	50

void Search::iterate()
{
	int score;
	int alpha = -INFTY;
	int beta = INFTY;
	
	for (rootdepth = 1; rootdepth <= maxdepth; rootdepth++) {
		maxplyreached = 0;
		maxplyreached_quiesce = 0;

		/* Search the root node. */
		score = search_root(0, rootdepth, alpha, beta);
		if (stop) {
			break;
		}
		
		/* If search failed low, re-search with a wider window */
		if (score <= alpha && score > -MATE) {
			if (showthinking) {
				print_result(rootdepth, score, '-');
			}

			/* If a fail-low happens after we have searched for a
			 * considerable amount of time, add some extra time. */
			if ((param_time & PARAM_TIME_FL)
					&& clock->get_elapsed_time()
							> clock->get_limit()/2
					&& mode == MOVE) {
				clock->allocate_more_time("fail low");
			}
			
			alpha = -INFTY;
			beta = beta+1;
			score = search_root(0, rootdepth, alpha, beta);
			if (stop) {
				break;
			}
		}
		
		/* Adjust window for next iteration */
		if (score >= beta && score < MATE) {
			if (showthinking) {
				print_result(rootdepth, score, '+');
			}

			/* If a fail-high happens after we have searched for a
			 * considerable amount of time, add some extra time. */
			if ((param_time & PARAM_TIME_FH)
					&& clock->get_elapsed_time()
							> clock->get_limit()/2
					&& mode == MOVE) {
				clock->allocate_more_time("fail high");
			}

			alpha = alpha-1;
			beta = INFTY;
		} else {
			if (showthinking) {
				print_result(rootdepth, score, '.');
			}
			
			alpha = score - WINDOW;
			beta = score + WINDOW;
		}
		
		/* Check if we found a mate. */
		if (score >= MATE || score <= -MATE) {
			break;
		}
	}
}

int Search::search_root(unsigned int ply, int depth, int alpha, int beta)
{
	ASSERT_DEBUG(tree.get_current_ply() == ply);
	ASSERT_DEBUG(ply == 0);

//	DBG(3, "depth=%d, alpha=%d, beta=%d\n", depth, alpha, beta);
	
	Node * node = tree[ply];
	
	int score;
	int moves = 0;
#ifdef USE_PVS
	bool first = true;
#endif
	
	for (Move mov = node->first(); mov; mov = node->next()) {
		tree.make_move(mov);
		if (!tree.get_board().is_legal()) {
			BUG("illegal move at root node: %s", mov.str().c_str());
		}
		moves++;

		if (showthinking) {
			print_thinking(depth);
		}

#ifdef USE_PVS
		/* Search the current move. We use a standard
		 * principal variation search here. */
		if (first) {
			score = -search(ply+1, depth-1, 0, -beta, -alpha);
			first = false;
		} else {
			score = -search(ply+1, depth-1, 0, -alpha-1, -alpha);
			if (score > alpha && score < beta) {
				score = -search(ply+1, depth-1, 0, 
						-beta, -alpha);
			}			
		}
#else
		/* Search the current move. We use a pure
		 * alpha-beta search here. */
		score = -search(ply+1, depth-1, 0, -beta, -alpha);
#endif

		tree.unmake_move();

		if (stop) {
			return alpha;
		}
		
		node->set_current_score(score);
		
		if (score > alpha) {
			alpha = score;
			node->set_best(mov);
			if (showthinking) {
				print_result(depth, score, ' ');
			}
			if (score >= beta) {
				STAT_INC(stat_cut);
				break;
			}
		}
	}
	
	/* Update history table */
#ifdef USE_HISTORY
	histtable[tree.get_board().get_side()]->add(node->get_best());
#endif

#ifdef COLLECT_STATISTICS
	stat_moves_sum += moves;
	stat_moves_cnt++;
#endif

	if (!shell->xboard) {
		clear_line();
	}

//	DBG(3, "return alpha=%d\n", alpha);
	return alpha;
}

int Search::search(unsigned int ply, int depth, int extend, int alpha, int beta)
{
	ASSERT_DEBUG(tree.get_current_ply() == ply);
	Node * node = tree[ply];

	/* If maximum search depth is reached, begin quiescence search. */
	if (depth <= 0) {
		return quiescence_search(ply, alpha, beta);
	}

	if (is_draw()) {
		return DRAW;
	}

	int save_alpha = alpha;
	int score;
	int moves = 0;
#ifdef USE_PVS
	bool first = true;
#endif
	
	nodes++;
	
	if (ply > maxplyreached) {
		maxplyreached = ply;
	}

	/*
	 * First look into the hash table if this
	 * position has already been search before.
	 * 
	 * TODO This surely needs some verification
	 */
	HashEntry hashentry;
	if (hashtable && hashtable->probe(tree.get_board(), &hashentry)) {
		if (hashentry.get_depth() >= (unsigned) depth) {
			score = hashentry.get_score();
			switch (hashentry.get_type()) {
			case HashEntry::EXACT:
				hashtable->incr_hits2();
				return score;
			case HashEntry::ALPHA:
				if (score <= alpha) {
					hashtable->incr_hits2();
					return score;
				}
				break;
#if 0
			case HashEntry::BETA:
				if (score >= beta) {
					hashtable->incr_hits2();
					return score;
				}
				break;
#endif
			}
		}

		node->set_hashmv(hashentry.get_move());
	}

#ifdef USE_NULLMOVE
	/* Null-move forward pruning */
	bool null_ok = !(tree[ply-1]->get_played_move().is_null())
		    && !(Evaluator::get_phase(tree.get_board())
				   == Evaluator::ENDGAME);
	if (!node->in_check() && null_ok) {
		tree.make_move(Move::null());
		score = -search(ply+1, depth-2-1, 0, -beta, -beta+1);
		tree.unmake_move();
		if (score >= beta) {
			STAT_INC(stat_nullcut);
			return beta;
		}
	}

#endif
	
#ifdef USE_IID
	/*
	 * Internal iterative deepening:
	 *
	 * If we don't have a move from the hash table, search 
	 * this node with a shallower depth to get a good move
	 * to search first.
	 */
	if (!node->get_hashmv() && depth > 2) {
		search(ply, depth-2, 0, alpha, beta);
		node->set_hashmv(node->get_best());
	}
#endif

	node->set_type(Node::FULLWIDTH);
#ifdef USE_HISTORY
	node->set_historytable(histtable[tree.get_board().get_side()]);
#endif	

	/*
	 * Futility pruning, extended futility pruning, and razoring.
	 */
#ifdef USE_RAZORING
	if (depth == 3  &&  (node->material_balance() + 900 <= alpha)) {
		STAT_INC(stat_razcut);
		depth--;
	}
#endif // USE_RAZORING

#ifdef USE_FUTILITYPRUNING
	bool fprune = false;
	if (depth == 1  &&  (node->material_balance() + 300 <= alpha)) {
		fprune = true;
#ifdef USE_EXTENDED_FUTILITYPRUNING
	} else if (depth == 2  &&  (node->material_balance() + 500 <= alpha)) {
		STAT_INC(stat_xfutcut);
		fprune = true;
		depth--;
#endif // USE_EXTENDED_FUTILITYPRUNING
	}
#endif // USE_FUTILITYPRUNING

	/*
	 * Search extensions.
	 */
	if (extend == 0) {
#ifdef EXTEND_IN_CHECK
		if (node->in_check()) {
			extend++;
		}
#endif
#ifdef EXTEND_RECAPTURE
		if (ply >= 3) {
			int mb = node->material_balance();
			int mb1 = -tree[ply-1]->material_balance();
			int mb2 = tree[ply-2]->material_balance();
			int mb3 = -tree[ply-3]->material_balance();
			if (abs(mb - mb1) >= 900 && abs(mb - mb2) >= 900
					&& abs(mb - mb3) >= 900) {
				extend++;
			}
		}
#endif
	} else {
		depth += extend;
		extend = 0;
	}

	/*
	 * Search all successor moves.
	 */
	for (Move mov = node->first(); mov; mov = node->next()) {
		Node * cnode = tree.make_move(mov);
		if (!tree.get_board().is_legal()) {
			tree.unmake_move();
			continue;
		}
		moves++;

#ifdef USE_FUTILITYPRUNING
		/* Futility pruning */
		if (fprune && !node->in_check() && !cnode->in_check()
				&& !mov.is_capture()
#ifdef HOICHESS
				&& !mov.is_enpassant()
				&& !mov.is_promotion()
#endif // HOICHESS
				) {
			STAT_INC(stat_futcut);
			tree.unmake_move();
			continue;
		}
#endif // USE_FUTILITYPRUNING
		
#ifdef USE_PVS
		/* Search the current move. We use a standard
		 * principal variation search here. */
		if (first) {
			score = -search(ply+1, depth-1, extend, -beta, -alpha);
			first = false;
		} else {
			score = -search(ply+1, depth-1, extend,
					-alpha-1, -alpha);
			if (score > alpha && score < beta) {
				score = -search(ply+1, depth-1, extend,
						-beta, -alpha);
			}			
		}
#else
		/* Search the current move. We use a pure
		 * alpha-beta search here. */
		score = -search(ply+1, depth-1, extend, -beta, -alpha);
#endif

		tree.unmake_move();

		if (nodes >= next_timecheck) {
			check_time();
			next_timecheck = nodes + TIMECHECK_INTERVAL;
		}

		if (stop) {
			return alpha;
		}

		if (score > alpha) {
			alpha = score;
			node->set_best(mov);
			if (score >= beta) {
				STAT_INC(stat_cut);
				break;
			}
		}
	}

	/* Test for checkmate or stalemate */
#if defined(HOICHESS)
	if (moves == 0) {
		if (node->in_check()) {
			alpha = -INFTY + ply;
		} else {
			alpha = DRAW;
		}
	}
#elif defined(HOIXIANGQI)
	if (moves == 0) {
		alpha = -INFTY + ply;
	}
#else
# error "neither HOICHESS nor HOIXIANGQI is defined"
#endif

	/* Save search result in hash table. */
	if (hashtable && alpha > -MATE && alpha < MATE) {
		int scoretype;
		if (alpha >= beta) {
			scoretype = HashEntry::BETA;
		} else if (alpha <= save_alpha) {
			scoretype = HashEntry::ALPHA;
		} else {
			scoretype = HashEntry::EXACT;
		}
		hashentry = HashEntry(tree.get_board(), alpha, node->get_best(),
				depth, scoretype);
		hashtable->put(hashentry);
	}
	
#ifdef USE_HISTORY
	/* Update history table */
	if (node->get_best()) {
		histtable[tree.get_board().get_side()]->add(node->get_best());
	}
#endif

#ifdef USE_KILLER
	/* Update killer */
	if (node->get_best() != node->get_hashmv()
			&& !(node->get_best().is_capture())
#ifdef HOICHESS
			&& !(node->get_best().is_enpassant())
			&& !(node->get_best().is_promotion())
#endif // HOICHESS
			) {
		node->add_killer(node->get_best());
	}
#endif // USE_KILLER

#ifdef COLLECT_STATISTICS
	stat_moves_sum += moves;
	stat_moves_cnt++;
#endif
	
	return alpha;
}

int Search::quiescence_search(unsigned int ply, int alpha, int beta)
{
	ASSERT_DEBUG(tree.get_current_ply() == ply);
	Node * node = tree[ply];

	int score;
	int moves = 0;
	
	nodes_quiesce++;

	if (ply > maxplyreached_quiesce) {
		maxplyreached_quiesce = ply;
	}

	/*
	 * First look into the hash table if this
	 * position has already been search before.
	 *
	 * TODO This surely needs some verification
	 */
	HashEntry hashentry;
	if (hashtable && hashtable->probe(tree.get_board(), &hashentry)) {
		score = hashentry.get_score();
		switch (hashentry.get_type()) {
		case HashEntry::EXACT:
		case HashEntry::QUIESCE:
			hashtable->incr_hits2();
			return score;
		case HashEntry::ALPHA:
			if (score <= alpha) {
				hashtable->incr_hits2();
				return score;
			}
			break;
#if 0
		case HashEntry::BETA:
			if (score >= beta) {
				hashtable->incr_hits2();
				return score;
			}
			break;
#endif
		}
	}
	
	/*
	 * Evaluate board position.
	 *
	 * This score will be returned if none
	 * of the moves is better than alpha.
	 */
	score = evaluator->eval(tree.get_board(), alpha, beta, myside);
	if (score >= beta) {
		/* TODO Is this really a good idea? */
		return score;
	} else if (score > alpha) {
		alpha = score;
	}

	/* MAXPLY is an absolute depth limit */
	if (ply == MAXPLY-1) {
		WARN("reached maximum tree depth: %d", ply);
		return score;
	}
	
	node->set_type(Node::QUIESCE);
	
	for (Move mov = node->first(); mov; mov = node->next()) {
		tree.make_move(mov);
		if (!tree.get_board().is_legal()) {
			tree.unmake_move();
			continue;
		}
		moves++;
		
		score = -quiescence_search(ply+1, -beta, -alpha);

		tree.unmake_move();

		if (score > alpha) {
			alpha = score;
			node->set_best(mov);
			if (score >= beta) {
				STAT_INC(stat_cut);
				break;
			}
		}
	}

	/* Test for checkmate */
	if (moves == 0 && node->in_check())
		alpha = -INFTY + ply;
	
	/* Save search result in hash table. */
	if (hashtable && alpha > -MATE && alpha < MATE) {
		hashentry = HashEntry(tree.get_board(), alpha, node->get_best(),
				0, HashEntry::QUIESCE);
		hashtable->put(hashentry);
	}

#ifdef COLLECT_STATISTICS
	stat_moves_sum_quiesce += moves;
	stat_moves_cnt_quiesce++;
#endif
	
	return alpha;
}

/* 
 * Return true if the current position is a draw (draw by rule).
 */
bool Search::is_draw()
{
	const Board & board = tree.get_board();
	
	/* 50 move rule */
	if (board.get_movecnt50() >= 100) {
		return true;
	}
	
	/* Draw due to insufficient material */
	if (board.is_material_draw()) {
		return true;
	}
	
	/* Look for repetitions in the tree */
	int rep = 1;
	for (int ply=tree.get_current_ply()-1; ply>=0; ply--) {
		if (tree[ply]->get_hashkey() == board.get_hashkey()) {
			rep++;
		}
		
		if (rep >= 2) {
			return true;
		}
	}

	/* Look for repetitions in the game history */
	rep += game->repetitions_search(board);
	if (rep >= 2) {
		return true; 
	}
	
	return false;
}


void Search::check_time()
{
	if (clock->timeout() && mode == MOVE) {
		stop = true;
	}

	if (showthinking  &&  clock->get_elapsed_time() >= next_update) {
		print_thinking(rootdepth);
	}
}


