/* $Id: hash.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/hash.cc
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
#include "board.h"
#include "hash.h"
#include "move.h"

#include <stdio.h>


/*****************************************************************************
 * 
 * Member functions of class HashEntry.
 *
 *****************************************************************************/

/* TODO This should be inline, but the class definition of class Board
 * is not yet available in hash.h, thus get_hashkey() cannot be called. */
HashEntry::HashEntry(const Board & board, int score, Move move, int depth, 
		int type) 
{
	this->hashkey = board.get_hashkey();
	this->type = type;
	this->depth = depth;
	this->score = score;
	this->move = move;
}

/*****************************************************************************
 * 
 * Member functions of class HashTable.
 *
 *****************************************************************************/

HashTable::HashTable(unsigned long size)
{
	ASSERT(size > 0);

	table_size = size;
	table = new HashEntry[table_size];
	entries = 0;

	replacement_scheme = REPL_ALWAYS;
	
	reset_statistics();
}

HashTable::~HashTable()
{
	delete[] table;
}

void HashTable::clear()
{
	delete[] table;
	table = new HashEntry[table_size];
	entries = 0;

	reset_statistics();
}

bool HashTable::put(const HashEntry & entry)
{
	const unsigned long key = entry.hashkey % table_size;
	const HashEntry & e = table[key];

	if (e.type == HashEntry::NONE) {
		entries++;
	} else {
		switch (replacement_scheme) {
		case REPL_ALWAYS:
			/* Always replace. */
			break;
		case REPL_DEPTH:
			/* Replace when same depth or deeper. */
			if (e.depth > entry.depth) {
				return false;
			}
			break;
		default:
			BUG("replacement_scheme = %d", replacement_scheme);
		}

		if (e.hashkey != entry.hashkey) {
			STAT_INC(stat_collisions);
		}
	}

	table[key] = entry;
	return true;
}

bool HashTable::probe(const Board & board, HashEntry * entry)
{
	STAT_INC(stat_probes);

	const unsigned long key = board.get_hashkey() % table_size;
	const HashEntry & e = table[key];
	
	if (e.type == HashEntry::NONE || e.hashkey != board.get_hashkey()) {
		return false;
	}

	/* If this entry has a move, make sure it is
	 * valid for the given board position. */
	if (e.move) {
		if (!e.move.is_valid(board)) {
			STAT_INC(stat_collisions2);
			return false;
		}

		if (!e.move.is_legal(board)) {
			WARN("illegal move in hash table");
			return false;
		}
	}
	
	*entry = e;
	STAT_INC(stat_hits);
	return true;
}

void HashTable::set_replacement_scheme(enum replacement_schemes scheme)
{
	replacement_scheme = scheme;
}

void HashTable::set_replacement_scheme(const std::string & scheme)
{
	if (scheme == "always") {
		replacement_scheme = REPL_ALWAYS;
	} else if (scheme == "depth") {
		replacement_scheme = REPL_DEPTH;
	} else {
		fprintf(stderr, "Invalid hash replacement scheme: %s\n",
				scheme.c_str());
	}
}

void HashTable::print_info(FILE * fp) const
{
	fprintf(fp, "Hash table size: %lu entries (%.1f MiB)\n",
			table_size,
			(float) table_size * sizeof(HashEntry) / (1<<20));
	fprintf(fp, "Hash table usage: %lu entries (%lu%%)\n",
			entries,
			entries*100/table_size);

	const char * s;
	switch (replacement_scheme) {
	case REPL_ALWAYS:
		s = "always replace";
		break;
	case REPL_DEPTH:
		s = "same depth or deeper";
		break;
	default:
		BUG("replacement_scheme = %d", replacement_scheme);
	}
	fprintf(fp, "Hash table replacement scheme: %s\n", s);
}

void HashTable::print_statistics(FILE * fp) const
{
	fprintf(fp, "Hash table entries: %lu (%lu%% full)\n",
			entries, entries*100/table_size);

#ifdef COLLECT_STATISTICS
	if (stat_probes > 0) {
		fprintf(fp,
			"Hash table probes: %lu, hits: %lu/%lu (%lu%%/%lu%%)\n",
			stat_probes,
			stat_hits, stat_hits2,
			stat_hits*100/stat_probes, stat_hits2*100/stat_probes);
	}
	fprintf(fp, "Hash table collisions: %lu/%lu\n",
			stat_collisions, stat_collisions2);
#endif // COLLECT_STATISTICS
}

void HashTable::reset_statistics()
{
#ifdef COLLECT_STATISTICS
	stat_probes = 0;
	stat_hits = 0;
	stat_hits2 = 0;
	stat_collisions = 0;
	stat_collisions2 = 0;
#endif // COLLECT_STATISTICS
}

