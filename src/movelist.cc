/* $Id: movelist.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/movelist.cc
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
#include "move.h"
#include "movelist.h"

Movelist::Movelist()
{
	nextin = 0;
	nextout = 0;
}

Movelist::~Movelist()
{}

void Movelist::filter_illegal(const Board & board)
{
	for (unsigned int i=0; i<size(); i++) {
		if (!move[i].is_legal(board)) {
			swap(i, size()-1);
			nextin--;
			i--;
		}
	}

	nextout = 0;
}

#ifdef STATS_MOVELIST
std::map<int, int> Movelist::maxsize;

void Movelist::print_stats()
{
	printf("maximum movelist size statistics:\n");
	printf("size\tcount\n");
	for (std::map<int, int>::iterator it = maxsize.begin();
			it != maxsize.end(); it++) {
		printf("%d\t%d\n", it->first, it->second);
	}
}
#endif
