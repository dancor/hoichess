/* $Id: game.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/game.cc
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
#include "game.h"


GameEntry::GameEntry(const Board & board, Move mov,
		const Clock & wclock, const Clock & bclock,
		const MoveAttributes & attr)
{
	this->board = board;
	this->move = mov;
	this->clock[WHITE] = wclock;
	this->clock[BLACK] = bclock;
	this->attr = attr;
}


/*
 * Create a new game, starting from the position described by board
 * and assign a clock to each side.
 */
Game::Game(const Board & board, const Clock & wclock, const Clock & bclock)
{
	initial_board = current_board = board;
	initial_clock[WHITE] = old_clock[WHITE] = current_clock[WHITE] = wclock;
	initial_clock[BLACK] = old_clock[BLACK] = current_clock[BLACK] = bclock;
	
	running = false;
	result = OPEN;
	nr_hashkeys = 0;
}

bool Game::is_over() const
{
	return result != OPEN;
}

bool Game::is_running() const
{
	return running;
}

int Game::get_result() const
{
	return result;
}

std::string Game::get_result_str() const
{
	return result_str;
}

std::string Game::get_result_comment() const
{
	return result_comment;
}

Board Game::get_board() const
{
	return current_board; 
}

int Game::get_side() const
{
	return current_board.get_side();
}

Clock Game::get_clock() const
{
	return current_clock[get_side()];
}

Clock Game::get_clock(Color side) const
{
	ASSERT(side == WHITE || side == BLACK);
	return current_clock[side];
}

void Game::start()
{
	current_clock[get_side()].start();
	running = true;
}

void Game::make_move(Move mov, const GameEntry::MoveAttributes & move_attr)
{
	ASSERT(mov.is_valid(current_board));
	ASSERT(mov.is_legal(current_board));

	/* stop clock of side that has played the current move */
	current_clock[get_side()].stop();

	/* make sure we don't put any running clocks in game history */
	old_clock[WHITE].stop();
	old_clock[BLACK].stop();

	/* save current position */
	GameEntry entry(current_board, mov, old_clock[WHITE], old_clock[BLACK],
			move_attr);
	entries.push_back(entry);
	
	/* update current position with new move and clocks */
	current_board.make_move(mov);
	old_clock[WHITE] = current_clock[WHITE];
	old_clock[BLACK] = current_clock[BLACK];

	/* check result and start clock of side to move */
	check_result();
	if (!result) {
		current_clock[get_side()].start();
		running = true;
	} else {
		running = false;
	}

	update_hashkeys();
	undone_entries.clear();
}

bool Game::undo_move()
{
	if (entries.size() == 0) {
		running = false;
		return false;
	}

	/* get undo information */
	const GameEntry last = entries.back();
	entries.pop_back();
	
	/* save last state */
	old_clock[WHITE].stop();
	old_clock[BLACK].stop();
	GameEntry undone(current_board,
			last.get_move(),
			old_clock[WHITE], old_clock[BLACK],
			last.get_attr());
	undone_entries.push_front(undone);

	/* load last state */
	current_board = last.get_board();
	old_clock[WHITE] = current_clock[WHITE] = last.get_clock(WHITE);
	old_clock[BLACK] = current_clock[BLACK] = last.get_clock(BLACK);
	current_clock[current_board.get_side()].start();
	
	/* set result to open */
	running = true;
	result = OPEN;
	
	update_hashkeys();
	
	return true;
}

/*
 * FIXME This does not restore clocks correctly.
 */
Move Game::redo_move()
{
	if (undone_entries.size() == 0) {
		return NO_MOVE;
	}

	/* get undo information */
	GameEntry undone = undone_entries.front();
	undone_entries.pop_front();

	/* stop clock of side that has played the current move */
	current_clock[get_side()].stop();

	/* make sure we don't put any running clocks in game history */
	old_clock[WHITE].stop();
	old_clock[BLACK].stop();

	/* save current position */
	GameEntry entry(current_board,
			undone.get_move(),
			old_clock[WHITE], old_clock[BLACK],
			undone.get_attr());
	entries.push_back(entry);
	
	/* update current position with new move and clocks */
	current_board = undone.get_board();
	current_clock[WHITE] = old_clock[WHITE] = undone.get_clock(WHITE);
	current_clock[BLACK] = old_clock[BLACK] = undone.get_clock(BLACK);

	/* check result and start clock of side to move */
	check_result();
	if (!result) {
		current_clock[get_side()].start();
		running = true;
	} else {
		running = false;
	}

	update_hashkeys();
	
	return undone.get_move();
}

/*
 * Set up a new position based on the given board. Take the clocks as
 * they were at the _beginning_ of the old game.
 */
void Game::set_board(const Board & board)
{
	ASSERT(board.is_valid());
	ASSERT(board.is_legal());

	current_board = board;
	old_clock[WHITE] = current_clock[WHITE] = initial_clock[WHITE];
	old_clock[BLACK] = current_clock[BLACK] = initial_clock[BLACK];
	
	entries.clear();

	running = false;
	check_result();

	update_hashkeys();
}

bool Game::set_board(const std::string& fen)
{
	Board board;
	if (!board.parse_fen(fen)) {
		entries.clear();
		running = false;
		result = ILLEGAL;
		return false;
	}
	
	set_board(board);
	return true;
}

/*
 * Replace the current clock of the side to move.
 */
void Game::set_clock(const Clock & clock)
{
	old_clock[get_side()] = current_clock[get_side()] = clock;

	if (running) {
		current_clock[get_side()].start();
	}
}

void Game::set_clock(Color side, const Clock & clock)
{
	ASSERT(side == WHITE || side == BLACK);
	old_clock[side] = current_clock[side] = clock;

	if (running) {
		current_clock[side].start();
	}
}

/*
 * Replace the current _and_initial_ clocks of both sides by new ones.
 */
void Game::set_clocks(const Clock & wclock, const Clock & bclock)
{
	initial_clock[WHITE] = old_clock[WHITE] = current_clock[WHITE] = wclock;
	initial_clock[BLACK] = old_clock[BLACK] = current_clock[BLACK] = bclock;

	if (running) {
		current_clock[current_board.get_side()].start();
	}
}

/*
 * Count how often this board appeared in the game history,
 * _not_ including the current position.
 */
int Game::repetitions(const Board & board) const
{
	int rep = 0;
	
	for (std::list<GameEntry>::const_reverse_iterator it = entries.rbegin();
			it != entries.rend(); it++) {
		if (it->get_board() == board)
			rep++;
	}

	return rep;
}

/*
 * Count how often this board appeared in the game history,
 * _not_ including the current position.
 * 
 * This doesn't compare actual Board objects, but uses a list
 * of hash keys. This is intended to be called during search.
 * It is much faster than repetitions(), but might theoretically
 * make mistakes (due to hash key collisions).
 */
int Game::repetitions_search(const Board & board) const
{
	/* We could also test if entries.empty() ... */
	if (nr_hashkeys == 0) {
		return 0;
	}
	
	const Hashkey hk = board.get_hashkey();
	int rep = 0;

	for (unsigned int i = (board.get_side() == entries.back().get_side())
			? 0 : 1; i<nr_hashkeys; i+=2) {
		if (hk == hashkeys[i]) {
			rep++;
			if (rep >= 2)
				break;
		}				
	}

	return rep;
}

/*
 * Count the number of (full-)moves of the current player since his last
 * book move. If the last move was out of book, this number is 1.
 */
int Game::last_bookmove() const
{
	unsigned int n = 0;
	for (std::list<GameEntry>::const_reverse_iterator it = entries.rbegin();
			it != entries.rend(); it++) {
		if (it->get_board().get_side() != get_side()) {
			continue;
		}
		
		n++;
		if (it->get_attr().bookmove) {
			break;
		}
	}

#ifdef DEBUG
	printf("last bookmove: %d moves ago\n", n);
#endif
	return n;
}


/*
 * Update the list of hash keys that is used by repetitions_search().
 * Store max. 100 hash keys (50 move rule) and only up to an irreversible
 * move (pawn, capture or castling).
 */
void Game::update_hashkeys()
{
	nr_hashkeys = 0;
	for (std::list<GameEntry>::reverse_iterator it = entries.rbegin();
			it != entries.rend(); it++) {
		Move mov = it->get_move();
#ifdef HOICHESS
		if (mov.ptype() == PAWN || mov.is_capture() || mov.is_castle())
			break;
#endif

		hashkeys[nr_hashkeys++] = it->get_board().get_hashkey();
		if (nr_hashkeys == 100)
			break;
	}

#ifdef DEBUG
	printf("positions in game history: %d\n", entries.size());
	printf("hashkeys stored: %d\n", nr_hashkeys);
	printf("repetitions: %d/%d\n", repetitions(current_board),
			repetitions_search(current_board));
#endif
}

/*
 * Check if the game has ended by rule.
 */
void Game::check_result()
{
	const Board & board = get_board();
	if (board.is_mate()) {
		if (board.get_side() == WHITE) {
			result = BLACKMATES;
			result_str = "0-1";
			result_comment = "Black mates";
		} else {
			result = WHITEMATES;
			result_str = "1-0";
			result_comment = "White mates";
		}
		
	} else if (board.is_stalemate()) {
#if defined(HOICHESS)
		result = STALEMATE;
		result_str = "1/2-1/2";
		result_comment = "Stalemate";
#elif defined(HOIXIANGQI)
		if (board.get_side() == WHITE) {
			result = BLACKSTALEMATES;
			result_str = "0-1";
			result_comment = "Black stalemates";
		} else {
			result = WHITESTALEMATES;
			result_str = "1-0";
			result_comment = "White stalemates";
		}
#else
# error "neither HOICHESS nor HOIXIANGQI defined"
#endif
	} else if (board.get_movecnt50() == 100) {
		result = RULE50;
		result_str = "1/2-1/2";
		result_comment = "50 move rule";
	} else if (repetitions(board) >= 2) {
		result = REPS3;
		result_str = "1/2-1/2";
		result_comment = "3 repetitions";
	} else if (board.is_material_draw()) {
		result = MATERIAL;
		result_str = "1/2-1/2";
		result_comment = "Insufficient material";
	} else {
		result = OPEN;
		result_str = "*";
		result_comment = "Open";
	}
}

void Game::print(FILE * fp) const
{
	fprintf(fp, "Positions in game history: %d\n", entries.size());
	for (std::list<GameEntry>::const_iterator it = entries.begin();
			it != entries.end(); it++) {
		fprintf(fp, "---------------------------------------------\n");
		it->get_board().print_small(fp);
		fprintf(fp, "\n");
		
		fprintf(fp, "White clock:\n");
		it->get_clock(WHITE).print(fp);
		fprintf(fp, "Black clock:\n");
		it->get_clock(BLACK).print(fp);
		fprintf(fp, "\n");
		
		fprintf(fp, "Move played: %s\n",
				it->get_move().san(it->get_board()).c_str());
		fprintf(fp, "Move attributes:");
		if (it->get_attr().computer) fprintf(fp, " computer");
		if (it->get_attr().bookmove) fprintf(fp, " bookmove");
		fprintf(fp, "\n\n");
	}
}

void Game::write_pgn(FILE * fp) const
{
	/* standard tags (seven tag roster) */
	fprintf(fp, "[Event \"unknown\"]\n");
	fprintf(fp, "[Site \"unknown\"]\n");
	fprintf(fp, "[Date \"unknown\"]\n");
	fprintf(fp, "[Round \"unknown\"]\n");
	fprintf(fp, "[White \"unknown\"]\n");
	fprintf(fp, "[Black \"unknown\"]\n");
	fprintf(fp, "[Result \"%s\"]\n", get_result_str().c_str());

	/* additional tag: FEN if non-standard starting position */
	std::string ofen = 
		(entries.size() > 0)
		? entries.begin()->get_board().get_fen()
		: current_board.get_fen();
	if (ofen != opening_fen()) {
		fprintf(fp, "[FEN \"%s\"]\n", ofen.c_str());
	}
	fprintf(fp, "\n");
	
	/* moves */
	unsigned int i = 1;
	unsigned int nchars = 0;
	for (std::list<GameEntry>::const_iterator it = entries.begin();
			it != entries.end(); it++) {
		std::string s = it->get_move().san(it->get_board());

		if (it->get_board().get_side() == WHITE) {
			nchars += fprintf(fp, "%d. %s ", i, s.c_str());
		} else {
			nchars += fprintf(fp, "%s ", s.c_str());
			i++;
		}

		if (nchars > 70) {
			fprintf(fp, "\n");
			nchars = 0;
		}
	}
	fprintf(fp, "\n");

	/* result */
	fprintf(fp, "%s {%s}\n",
			get_result_str().c_str(),
			get_result_comment().c_str());
}

