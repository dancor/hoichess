/* $Id: historytable.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/historytable.h
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
#ifndef HISTORYTABLE_H
#define HISTORYTABLE_H

#include "basic.h"

#include <string.h>

class HistoryTable
{
      private:
	unsigned long table[BOARDSIZE][BOARDSIZE];

      public:
	inline HistoryTable();
	inline ~HistoryTable();

      public:
	inline void reset();
	inline void add(Move mov);
	inline unsigned long get(Move mov);
};


inline HistoryTable::HistoryTable()
{
	reset();
}
	
inline HistoryTable::~HistoryTable()
{}

inline void HistoryTable::reset()
{
	memset(table, 0, sizeof(table));
}
	
inline void HistoryTable::add(Move mov)
{
	ASSERT_DEBUG(mov.from() >= 0  &&  mov.from() < BOARDSIZE);
	ASSERT_DEBUG(mov.to()   >= 0  &&  mov.to()   < BOARDSIZE);

	table[mov.from()][mov.to()]++;
}
	
inline unsigned long HistoryTable::get(Move mov)
{
	ASSERT_DEBUG(mov.from() >= 0  &&  mov.from() < BOARDSIZE);
	ASSERT_DEBUG(mov.to()   >= 0  &&  mov.to()   < BOARDSIZE);

	return table[mov.from()][mov.to()];
}

#endif // HISTORYTABLE_H
