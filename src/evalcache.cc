/* $Id: evalcache.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/evalcache.cc
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

#include "common.h"
#include "evalcache.h"

#include <limits.h>

EvaluationCache::EvaluationCache(unsigned long size)
{
	ASSERT(size > 0);

	cache_size = size;
	cache = new struct cacheentry[cache_size];
	for (unsigned long i = 0; i < cache_size; i++) {
		cache[i].score = INT_MIN;
	}
	entries = 0;

	reset_statistics();
}

EvaluationCache::~EvaluationCache()
{
	delete[] cache;
}

void EvaluationCache::clear()
{
	delete[] cache;
	cache = new struct cacheentry[cache_size];
	for (unsigned long i = 0; i < cache_size; i++) {
		cache[i].score = INT_MIN;
	}
	entries = 0;

	reset_statistics();
}

bool EvaluationCache::put(const Board & board, int score)
{
	const Hashkey hashkey = board.get_hashkey_noside();
	const unsigned long key = hashkey % cache_size;

	/* We use an always replace strategy. */
	if (cache[key].score == INT_MIN) {
		entries++;
	} else if (cache[key].hashkey != hashkey) {
		STAT_INC(stat_collisions);
	}

	cache[key].hashkey = hashkey;
	cache[key].score = (board.get_side() == WHITE) ? score : -score;
	return true;
}

bool EvaluationCache::probe(const Board & board, int * score)
{
	ASSERT(score != NULL);
	STAT_INC(stat_probes);
	
	const Hashkey hashkey = board.get_hashkey_noside();
	const unsigned long key = hashkey % cache_size;

	if (cache[key].score == INT_MIN) {
		return false;
	} else if (cache[key].hashkey != hashkey) {
		return false;
	}

	*score = (board.get_side() == WHITE)
		? cache[key].score : -cache[key].score;
	STAT_INC(stat_hits);
	return true;
}

void EvaluationCache::print_info(FILE * fp) const
{
	fprintf(fp, "Evaluation cache size: %lu entries (%.1f MiB)\n",
			cache_size,
			(float) cache_size * SIZEOF_ENTRY / (1<<20));
	fprintf(fp, "Evaluation cache usage: %lu entries (%lu%%)\n",
			entries,
			entries*100/cache_size);
}

void EvaluationCache::print_statistics(FILE * fp) const
{
	fprintf(fp, "Evaluation cache entries: %lu (%lu%% full)\n",
			entries, entries*100/cache_size);

#ifdef COLLECT_STATISTICS
	if (stat_probes > 0) {
		fprintf(fp,
			"Evaluation cache probes: %lu, hits: %lu (%lu%%)\n",
			stat_probes,
			stat_hits,
			stat_hits*100/stat_probes);
	}
	fprintf(fp, "Evaluation cache collisions: %lu\n",
			stat_collisions);
#endif // COLLECT_STATISTICS
}

void EvaluationCache::reset_statistics()
{
#ifdef COLLECT_STATISTICS
	stat_probes = 0;
	stat_hits = 0;
	stat_collisions = 0;
#endif // COLLECT_STATISTICS
}

