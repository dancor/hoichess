/* $Id: bench.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/bench.h
 *
 * Copyright (C) 2005 Holger Ruckdeschel <holger@hoicher.de>
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
#ifndef BENCH_H
#define BENCH_H

#include "common.h"
#include "board.h"

class Bench
{
      private:
	typedef void (Board::* movegen_t) (Movelist *) const;
		
      private:
	static const char * fens[];	 

      public:
	Bench();
	~Bench();

      public:
	void bench_movegen();
	void bench_evaluator();
	void bench_makemove();

      private:
	unsigned int bench_movegen(movegen_t movgen, const char * movegen_name);
};

#endif // BENCH_H
