/* $Id: game.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/game.h
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
#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "board.h"
#include "clock.h"
#include "basic.h"

#include <list>

class GameEntry
{
      public:
	class MoveAttributes {
		friend class GameEntry;

	      public:
		bool computer;
		bool bookmove;

	      private:
		MoveAttributes()
		{}

	      public:
		MoveAttributes(bool computer, bool bookmove)
			: computer(computer), bookmove(bookmove)
		{}
	};
	
      private:
	Board board;		/* position */
	Move move;		/* move that was played in this position */
	Clock clock[2];		/* clocks at beginning */
	MoveAttributes attr;	/* more information about this->move */

      public:
	GameEntry(const Board & board, Move mov, const Clock & wclock,
			const Clock & bclock, 
			const MoveAttributes & attr);
	~GameEntry() {}

      public:
	Board get_board() const
	{ return board; }

	Color get_side() const
	{ return board.get_side(); }
	
	Move get_move() const
	{ return move; }
	
	Clock get_clock(int side) const
	{ return clock[side]; }

	MoveAttributes get_attr() const
	{ return attr; }
};

class Game
{
      public:
	enum game_results { OPEN = 0,
		WHITEMATES, BLACKMATES,
#if defined(HOICHESS)
		STALEMATE,
#elif defined(HOIXIANGQI)
		WHITESTALEMATES, BLACKSTALEMATES,
#else
# error "neither HOICHESS nor HOIXIANGQI defined"
#endif
		RULE50, REPS3,
		MATERIAL,
		ILLEGAL };
	
      private:
	Board initial_board;	/* initial position */
	Clock initial_clock[2];	/* initial clocks */
	
	Board current_board;	/* current position */
	Clock current_clock[2];	/* current clocks, one of them may be running */
	Clock old_clock[2];	/* clocks at beginning of current position */
	std::list<GameEntry> entries;	/* past positions (excl. current) */
	std::list<GameEntry> undone_entries;
	bool running;
	
	Hashkey hashkeys[100];
	unsigned int nr_hashkeys;
	
	int result;
	std::string result_str;
	std::string result_comment;

      public:
	Game(const Board & board, const Clock & wclock, const Clock & bclock);
	~Game() {}

      public:
	bool is_over() const;
	bool is_running() const;
	int get_result() const;
	std::string get_result_str() const;
	std::string get_result_comment() const;
	Board get_board() const;
	int get_side() const;
	Clock get_clock() const;
	Clock get_clock(Color side) const;
	
	void start();
	void make_move(Move mov, const GameEntry::MoveAttributes & move_attr);
	bool undo_move();
	Move redo_move();
	bool set_board(const std::string& fen);
	void set_board(const Board & board);
	void set_clock(const Clock & clock);
	void set_clock(Color side, const Clock & clock);
	void set_clocks(const Clock & wclock, const Clock & bclock);

	int repetitions(const Board & board) const;
	int repetitions_search(const Board & board) const;
	int last_bookmove() const;

      private:
	void update_hashkeys();
	void check_result();

      public:
	void print(FILE * fp = stdout) const;
	void write_pgn(FILE * fp = stdout) const;
};

#endif // GAME_H
