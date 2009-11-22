/* $Id: evalcache.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/evalhash.h
 *
 * Copyright (C) 2005, 2006 Holger Ruckdeschel <holger@hoicher.de>
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
#ifndef EVALCACHE_H
#define EVALCACHE_H

#include "board.h"


/*****************************************************************************
 *
 * Class EvaluationCache
 *
 *****************************************************************************/

class EvaluationCache
{
      private:
	/* A cache slot is empty if score == INT_MIN. */
	struct cacheentry {
		Hashkey hashkey;
		int score;
	};
	
      public:
	static const size_t SIZEOF_ENTRY = sizeof(struct cacheentry);
      
      private:
	unsigned long cache_size;
	struct cacheentry * cache;
	
	unsigned long entries;

#ifdef COLLECT_STATISTICS
	unsigned long stat_probes;
	unsigned long stat_hits;
	unsigned long stat_collisions;
#endif

      public:
	EvaluationCache(unsigned long size);
	~EvaluationCache();

      public:
	void clear();
	bool put(const Board & board, int score);
	bool probe(const Board & board, int * score);	

	void print_info(FILE * fp = stdout) const;
	void print_statistics(FILE * fp = stdout) const;
	void reset_statistics();
};

#endif // EVALCACHE_H
