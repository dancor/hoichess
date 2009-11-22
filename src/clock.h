/* $Id: clock.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/clock.h
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
#ifndef CLOCK_H
#define CLOCK_H

#include "common.h"

#ifdef WIN32
# include "gettimeofday.h"
#else
# include <sys/time.h>
#endif

class Clock
{
      private:
	enum clock_mode { NONE, CONV, INCR, SUDDENDEATH, EXACT } mode;
	bool running;

	struct timeval start_tv;
	
	unsigned int elapsed;
	unsigned int limit;
	unsigned int hard_limit;
	int remaining_time;
	unsigned int remaining_moves;
	
	unsigned int base_time;
	unsigned int base_moves;
	unsigned int increment;

      public:
	Clock();
	Clock(unsigned int secs);
	Clock(unsigned int moves, unsigned int secs, unsigned int inc);
	~Clock() {}

      public:
	void start();
	unsigned int stop();
	void turn_back();

      private:
	void update() const;
	void do_update();
	
      public:
	void allocate_time();
	void allocate_more_time(const char * reason = NULL);

      private:
	void check_limit();

      public:
	bool timeout() const;
	unsigned int get_limit() const;
	unsigned int get_elapsed_time() const;
	void set_remaining_time(unsigned int csecs);
	void print(FILE * fp = stdout) const;
};

#endif // CLOCK_H
