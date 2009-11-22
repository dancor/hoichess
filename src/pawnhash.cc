/* $Id: pawnhash.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/pawnhash.cc
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
#include "pawnhash.h"

#include <stdio.h>



/*****************************************************************************
 * 
 * Member functions of class PawnHashTable.
 *
 *****************************************************************************/

PawnHashTable::PawnHashTable(unsigned long size)
{
	ASSERT(size > 0);

	table_size = size;
	table = new PawnHashEntry[table_size];
	entries = 0;

	reset_statistics();
}

PawnHashTable::~PawnHashTable()
{
	delete[] table;
}

void PawnHashTable::clear()
{
	delete[] table;
	table = new PawnHashEntry[table_size];
	entries = 0;

	reset_statistics();
}

bool PawnHashTable::put(const PawnHashEntry & entry)
{
	const unsigned long key = entry.hashkey % table_size;
	const PawnHashEntry & e = table[key];

	if (e.phase == -1) {
		entries++;
	} else {
		/* Always replace. */
		if (e.hashkey != entry.hashkey) {
			STAT_INC(stat_collisions);
		}
	}

	table[key] = entry;
	return true;
}

bool PawnHashTable::probe(Hashkey hashkey, PawnHashEntry * entry)
{
	STAT_INC(stat_probes);

	const unsigned long key = hashkey % table_size;
	const PawnHashEntry & e = table[key];
	
	if (e.phase == -1 || e.hashkey != hashkey) {
		entry->phase = -1;
		return false;
	}

	*entry = e;
	STAT_INC(stat_hits);
	return true;
}

void PawnHashTable::print_info(FILE * fp) const
{
	fprintf(fp, "Pawn hash table size: %lu entries (%.1f MiB)\n",
			table_size,
			(float) table_size * sizeof(PawnHashEntry) / (1<<20));
	fprintf(fp, "Pawn hash table usage: %lu entries (%lu%%)\n",
			entries,
			entries*100/table_size);
}

void PawnHashTable::print_statistics(FILE * fp) const
{
	fprintf(fp, "Pawn hash table entries: %lu (%lu%% full)\n",
			entries, entries*100/table_size);

#ifdef COLLECT_STATISTICS
	if (stat_probes > 0) {
		fprintf(fp,
			"Pawn hash table probes: %lu, hits: %lu/%lu"
			" (%lu%%/%lu%%)\n",
			stat_probes,
			stat_hits, stat_hits2,
			stat_hits*100/stat_probes, stat_hits2*100/stat_probes);
	}
	fprintf(fp, "Pawn hash table collisions: %lu\n",
			stat_collisions);
#endif // COLLECT_STATISTICS
}

void PawnHashTable::reset_statistics()
{
#ifdef COLLECT_STATISTICS
	stat_probes = 0;
	stat_hits = 0;
	stat_hits2 = 0;
	stat_collisions = 0;
#endif // COLLECT_STATISTICS
}
