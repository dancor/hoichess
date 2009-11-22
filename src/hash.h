/* $Id: hash.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/hash.h
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
#ifndef HASH_H
#define HASH_H

#include "common.h"
#include "board.h"
#include "move.h"
#include "util.h"


typedef uint64_t Hashkey;
#define NULLHASHKEY	((uint64_t) 0)

/*****************************************************************************
 *
 * Class HashEntry
 *
 *****************************************************************************/

class HashEntry
{
	friend class HashTable;

      public:
	enum hashentry_type { NONE, EXACT, ALPHA, BETA, QUIESCE };

      private:
	Hashkey hashkey;
	unsigned short type;
	unsigned short depth;
	int score;
	Move move;	

      public:
	FORCEINLINE HashEntry();
	HashEntry(const Board & board, int score, Move move, int depth,
			int type);
	FORCEINLINE ~HashEntry() {}

      public:
	inline unsigned int get_depth() const;
	inline int get_score() const;
	inline int get_type() const;
	inline Move get_move() const;
};

inline HashEntry::HashEntry()
{
	type = NONE;
}

inline unsigned int HashEntry::get_depth() const
{
	return depth;
}

inline int HashEntry::get_score() const
{
	return score;
}

inline int HashEntry::get_type() const
{
	return type; 
}

inline Move HashEntry::get_move() const
{
	return move;
}


/*****************************************************************************
 *
 * Class HashEntry
 *
 *****************************************************************************/

class HashTable
{
      public:
	enum replacement_schemes { REPL_ALWAYS, REPL_DEPTH };

      private:
	unsigned long table_size;
	HashEntry * table;

	unsigned long entries;
	enum replacement_schemes replacement_scheme;
	
#ifdef COLLECT_STATISTICS
	unsigned long stat_probes;
	unsigned long stat_hits;
	unsigned long stat_hits2;
	unsigned long stat_collisions;
	unsigned long stat_collisions2;
#endif

      public:
	HashTable(unsigned long size);
	~HashTable();

      public:
	void clear();
	bool put(const HashEntry & entry);
	bool probe(const Board & board, HashEntry * entry);	
	inline void incr_hits2();
	void set_replacement_scheme(enum replacement_schemes scheme);
	void set_replacement_scheme(const std::string & scheme);

	void print_info(FILE * fp = stdout) const;
	void print_statistics(FILE * fp = stdout) const;
	void reset_statistics();
};

inline void HashTable::incr_hits2()
{
	STAT_INC(stat_hits2);
}

#endif // HASH_H
