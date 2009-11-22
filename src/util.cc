/* $Id: util.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/util.cc
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
#include "util.h"
#include "signal.h"
#include "thread.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


Mutex stdout_mutex;

int atomic_printf(const char * fmt, ...)
{
	stdout_mutex.lock();
	
	va_list args;
	va_start(args, fmt);
	int ret = vfprintf(stdout, fmt, args);
	va_end(args);
	
	stdout_mutex.unlock();
	return ret;
}

int atomic_fprintf(FILE * fp, Mutex * mutex, const char * fmt, ...)
{
	ASSERT(mutex != NULL);
	mutex->lock();
	
	va_list args;
	va_start(args, fmt);
	int ret = vfprintf(fp, fmt, args);
	va_end(args);
	
	mutex->unlock();
	return ret;
}


/*
 * Win32 rand() returns only a 15 bit random number. We just take this
 * function on all systems - it is only used during initialization, so
 * it is not performance critical.
 *
 * Thanks to Bruce Moreland for this snippet,
 * and to Jim Ablett who brought it to me :-).
 */
uint64_t random64()
{
	uint64_t tmp = rand();
	tmp ^= ((uint64_t) rand() << 15);
	tmp ^= ((uint64_t) rand() << 30);
	tmp ^= ((uint64_t) rand() << 45);
	tmp ^= ((uint64_t) rand() << 60);
	return tmp;
}

#if 0
uint64_t random64()
{
	uint64_t tmp = random();
	tmp <<= 32;
	tmp |= random();
	return tmp;
}
#endif

std::string strprintf(const char * fmt, ...)
{
	char buf[65536]; // should be enough

	va_list args;
	va_start(args, fmt);
	int ret = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	ASSERT(ret >= 0);
	if ((unsigned) ret >= sizeof(buf)) {
		WARN("buffer overflow, output truncated");
	}

	return std::string(buf);
}


bool parse_size(const char * s, long * n)
{
	ASSERT(s != NULL);
	ASSERT(s != NULL);

	long tmp;
	
	if (s[strlen(s)-1] == 'M' && sscanf(s, "%ldM", &tmp) == 1) {
		*n = tmp * (1<<20);
		return true;
	} else if (s[strlen(s)-1] == 'K' && sscanf(s, "%ldK", &tmp) == 1) {
		*n = tmp * (1<<10);
		return true;
	} else if (sscanf(s, "%ld", &tmp) == 1) {
		*n = tmp;
		return true;
	} else {
		return false;
	}
}



FILE * logfp = NULL;
Mutex logfp_mutex;

void open_log(const char * logfile)
{
	logfp = fopen(logfile, "a");
	if (!logfp) {
		fprintf(stderr, "Cannot open logfile %s: %s\n",
				logfile, strerror(errno));
		exit(EXIT_FAILURE);
	}
	setbuf(logfp, NULL);

	time_t t;
	time(&t);
	/* Note that ctime() appends '\n' */
	fprintf(logfp, "---- Log for %s %s opened on %s",
			PROGNAME, VERSION, ctime(&t));
}

void close_log()
{
	if (!logfp)
		return;

	time_t t;
	time(&t);
	/* Note that ctime() appends '\n' */
	fprintf(logfp, "---- Log for %s %s closed on %s",
			PROGNAME, VERSION, ctime(&t));
	fclose(logfp);
}


void log(const char * fmt, ...)
{
	if (!logfp) {
		return;
	}
	
	va_list args;
	va_start(args, fmt);
	logfp_mutex.lock();
	vfprintf(logfp, fmt, args);
	logfp_mutex.unlock();
	va_end(args);
}

void vlog(const char * fmt, va_list args)
{
	if (!logfp) {
		return;
	}

	logfp_mutex.lock();
	vfprintf(logfp, fmt, args);
	logfp_mutex.unlock();
}
	

