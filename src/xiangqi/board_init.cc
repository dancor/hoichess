/* $Id: board_init.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/board_init.cc
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
#include "util.h"


Hashkey Board::hashkeys[2][7][90];
Hashkey Board::hash_side;


void Board::init()
{
	unsigned int k = 0;
	
	/* Generate hash keys */
	for (int c=0; c<2; c++) {
		for (int p=0; p<7; p++) {
			for (int s=0; s<90; s++) {
				//hashkeys[c][p][s] = random64();
				hashkeys[c][p][s] = uint64_table[k++];
			}
		}
	}
	
	//hash_side = random64();
	hash_side = uint64_table[k++];

	/* uint64_table should have contained as many values as we needed */
	ASSERT(k <= uint64_table_size);
}
