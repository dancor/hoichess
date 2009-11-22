/* $Id: clock.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/clock.cc
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
#include "clock.h"
#include "util.h"

Clock::Clock()
{
	mode = NONE;
	running = false;
	elapsed = 0;
	limit = 0;
	hard_limit = 0;
}

Clock::Clock(unsigned int secs)
{
	mode = EXACT;
	running = false;
	elapsed = 0;
	limit = secs * 100;
	hard_limit = limit;
}

Clock::Clock(unsigned int moves, unsigned int secs, unsigned int inc)
{
	ASSERT(secs > 0);
	
	base_time = secs * 100;
	base_moves = moves;
	increment = inc * 100;

	if (moves == 0 && inc == 0) {
		mode = SUDDENDEATH;
	} else if (inc == 0) {
		mode = CONV;
	} else {
		mode = INCR;
	}

	remaining_time = base_time;
	remaining_moves = base_moves;
	
	running = false;
	elapsed = 0;
	limit = 0;
	hard_limit = 0;
}

void Clock::start()
{
	if (running)
		return;
	
	gettimeofday(&start_tv, NULL);
	elapsed = 0;
	running = true;
}

unsigned int Clock::stop()
{
	update();
	running = false;

	switch (mode) {
	case NONE:
	case EXACT:
		break;
	case CONV:
		remaining_time -= elapsed;
		remaining_moves--;
		if (remaining_moves == 0) {
			remaining_time += base_time;
			remaining_moves = base_moves;
		}
		break;
	case INCR:
		remaining_time -= elapsed;
		remaining_time += increment;
		break;
	case SUDDENDEATH:
		remaining_time -= elapsed;
		break;
	}
	
	return elapsed;
}	

void Clock::turn_back()
{
	if (!running)
		return;

	gettimeofday(&start_tv, NULL);
	elapsed = 0;
}

void Clock::update() const
{
	/* We want to update the clock status even
	 * from const methods like print().
	 */
	const_cast<Clock *>(this)->do_update();
}

void Clock::do_update()
{
	if (!running)
		return;
	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	elapsed = (tv.tv_sec - start_tv.tv_sec) * 100
		+ (tv.tv_usec - start_tv.tv_usec) / 10000;
}

void Clock::allocate_time()
{
	switch (mode) {
	case NONE:
		return;
	case EXACT:
		/* limit and hard_limit already set in constructor */
		return;
	case CONV:
		ASSERT(remaining_moves > 0);
		limit = remaining_time / remaining_moves;
		break;		
	case INCR:
		limit = remaining_time / 20 + increment;
		break;
	case SUDDENDEATH:
		limit = remaining_time / 40;
		break;
	}

	if (remaining_time <= 0) {
		limit = 0;
		hard_limit = 0;
		if (verbose) {
			atomic_printf("No time remaining!\n");
		}
		return;
	}
	
	hard_limit = 2 * limit;
	if (hard_limit > (unsigned) remaining_time) {
		hard_limit = remaining_time;
	}

	check_limit();
	
	if (verbose) {
		atomic_printf("Search time allocation: %.2f sec"
				" (max. %.2f sec)\n",
				(float) limit/100,
				(float) hard_limit/100);
	}
}

void Clock::allocate_more_time(const char * reason)
{
	if (mode == NONE || mode == EXACT) {
		return;
	}

	unsigned int old_limit = limit;

	/* TODO Only a first try... */
	limit = old_limit + (hard_limit - old_limit) / 3;
	
	check_limit();
	
	if (verbose) {
		atomic_printf("Search time extended from %.2f sec"
				" to %.2f sec (reason: %s)\n",
				(float) old_limit/100,
				(float) limit/100,
				reason ? reason : "??");
	}
}

void Clock::check_limit()
{
	if (mode == NONE || mode == EXACT) {
		return;
	}
	
	if (limit > hard_limit) {
		limit = hard_limit;
	}
	
	/* Short on time? */
	if ((float) remaining_time / base_time < 0.1f) {
		limit = (int) (limit * 0.8f);
	}
	
	/* Very short on time? */
	if ((float) remaining_time / base_time < 0.05f) {
		limit = (int) (limit * 0.8f);
	}
	
//	if (limit == 0) {
//		limit = 1;
//	}
}

bool Clock::timeout() const
{
	if (mode == NONE) {
		return false;
	}
	
	if (!running) {
		WARN("Clock::timeout() called with stopped clock");
	}
	
	update();
	return (elapsed >= limit);
}

unsigned int Clock::get_limit() const
{
	return limit;
}

unsigned int Clock::get_elapsed_time() const
{
	update();
	return elapsed;
}

void Clock::set_remaining_time(unsigned int csecs)
{
	remaining_time = csecs;
}

void Clock::print(FILE * fp) const
{
	switch (mode) {
	case NONE:
		fprintf(fp, "Clock mode: none\n");
		break;
	case CONV:
		fprintf(fp, "Clock mode: conventional\n");
		fprintf(fp, "Remaining time: %.2f sec, remaining moves: %d\n",
				(float) remaining_time/100,
				remaining_moves);
		break;
	case INCR:
		fprintf(fp, "Clock mode: incremental\n");
		fprintf(fp, "Remaining time: %.2f sec, increment: %.2fs\n",
				(float) remaining_time/100,
				(float) increment/100);
		break;		
	case SUDDENDEATH:
		fprintf(fp, "Clock mode: sudden death\n");
		fprintf(fp, "Remaining time: %.2f sec\n",
				(float) remaining_time/100);
		break;
	case EXACT:
		fprintf(fp, "Clock mode: exact\n");
		fprintf(fp, "Time per move: %.2f sec\n",
				(float) limit/100);
		break;
	}

	update();
	if (running) {
		fprintf(fp, "Clock is running, elapsed time: %.2fs\n",
				(float) elapsed/100);
	}
}
