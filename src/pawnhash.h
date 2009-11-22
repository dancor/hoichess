/* $Id: pawnhash.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/pawnhash.h
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
#ifndef PAWNHASH_H
#define PAWNHASH_H

#include "common.h"
#include "board.h"
#include "hash.h"
#include "move.h"
#include "util.h"


/*****************************************************************************
 *
 * Class PawnHashEntry
 *
 *****************************************************************************/

class PawnHashEntry
{
	friend class PawnHashTable;

      private:
	Hashkey hashkey;
	int phase;
	int score[2];
#ifdef HOICHESS
	Bitboard passed[2];
#endif // HOICHESS

      public:
	FORCEINLINE PawnHashEntry();
	FORCEINLINE ~PawnHashEntry() {}

      public:
	inline bool is_valid() const;
	inline void set_invalid();
	inline Hashkey get_hashkey() const;
	inline void set_hashkey(Hashkey hashkey);
	inline unsigned int get_phase() const;
	inline void set_phase(unsigned int phase);
	inline int get_score(Color side) const;
	inline void set_score(Color side, int score);
#ifdef HOICHESS
	inline Bitboard get_passed(Color side) const;
	inline void set_passed(Color side, Bitboard bb);
#endif // HOICHESS
};

inline PawnHashEntry::PawnHashEntry()
{
	phase = -1;
}

inline bool PawnHashEntry::is_valid() const
{
	return (phase != -1);
}

inline void PawnHashEntry::set_invalid()
{
	phase = -1;
}

inline Hashkey PawnHashEntry::get_hashkey() const
{
	return hashkey;
}

inline void PawnHashEntry::set_hashkey(Hashkey hashkey)
{
	this->hashkey = hashkey;
}

inline unsigned int PawnHashEntry::get_phase() const
{
	ASSERT_DEBUG(phase >= 0);	
	return phase;
}

inline void PawnHashEntry::set_phase(unsigned int phase) 
{
	this->phase = phase;
}

inline int PawnHashEntry::get_score(Color side) const
{
	return score[side];
}

inline void PawnHashEntry::set_score(Color side, int score)
{
	this->score[side] = score;
}

#ifdef HOICHESS
inline Bitboard PawnHashEntry::get_passed(Color side) const
{
	return passed[side];
}

inline void PawnHashEntry::set_passed(Color side, Bitboard bb)
{
	this->passed[side] = bb;
}
#endif // HOICHESS

/*****************************************************************************
 *
 * Class PawnHashTable
 *
 *****************************************************************************/

class PawnHashTable
{
      private:
	unsigned long table_size;
	PawnHashEntry * table;

	unsigned long entries;

#ifdef COLLECT_STATISTICS
	unsigned long stat_probes;
	unsigned long stat_hits;
	unsigned long stat_hits2;
	unsigned long stat_collisions;
#endif // COLLECT_STATISTICS

      public:
	PawnHashTable(unsigned long size);
	~PawnHashTable();

      public:
	void clear();
	bool put(const PawnHashEntry & entry);
	bool probe(Hashkey hashkey, PawnHashEntry * entry);	
	inline void incr_hits2();

	void print_info(FILE * fp = stdout) const;
	void print_statistics(FILE * fp = stdout) const;
	void reset_statistics();
};

inline void PawnHashTable::incr_hits2()
{
	STAT_INC(stat_hits2);
}

#endif // HASHPAWN_H
