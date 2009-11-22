/* $Id: movelist.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/movelist.h
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
#ifndef MOVELIST_H
#define MOVELIST_H

#include "common.h"
#include "board.h"
#include "move.h"

#ifdef STATS_MOVELIST
# include <map>
#endif

class Movelist
{
      private:
	Move move[MOVELIST_MAXSIZE];
	int score[MOVELIST_MAXSIZE];
	
	unsigned int nextin;
	unsigned int nextout;
	
      public:
	Movelist();
	~Movelist();

      public:
	inline unsigned int size() const;
	inline void clear();
	inline void add(Move mov);
	inline Move & operator[](unsigned int i);
	inline Move operator[](unsigned int i) const;

	inline int get_score(unsigned int i) const;
	inline void set_score(unsigned int i, int s);
	
	inline void swap(int i, int j);
	void filter_illegal(const Board & board);

#ifdef STATS_MOVELIST
      private:
	static std::map<int, int> maxsize;
      public:
	static void print_stats();
#endif
};


inline unsigned int Movelist::size() const
{
	return nextin;
}
	
inline void Movelist::clear() 
{
#ifdef STATS_MOVELIST
	maxsize[nextin]++;
#endif
		
	nextin = 0;
	nextout = 0;
}

inline void Movelist::add(Move mov) 
{
	if (nextin >= MOVELIST_MAXSIZE) {
		WARN("movelist is full, move not added:");
		mov.print();		
		return;
	}
	
	move[nextin] = mov;
	score[nextin] = 0;
	nextin++;
}

inline Move & Movelist::operator[](unsigned int i)
{
	ASSERT_DEBUG(i < size());
	return move[i]; 
}

inline Move Movelist::operator[](unsigned int i) const
{
	ASSERT_DEBUG(i < size());
	return move[i]; 
}

inline int Movelist::get_score(unsigned int i) const
{
	return score[i];
}

inline void Movelist::set_score(unsigned int i, int s)
{
	score[i] = s;
}

inline void Movelist::swap(int i, int j)
{
	Move tmp_move = move[i];
	move[i] = move[j];
	move[j] = tmp_move;

	int tmp_score = score[i];
	score[i] = score[j];
	score[j] = tmp_score;
}

#endif // MOVELIST_H
