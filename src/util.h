/* $Id: util.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/util.h
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
#ifndef UTIL_H
#define UTIL_H

#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>


extern Mutex stdout_mutex;

extern int atomic_printf(const char * fmt, ...);
extern int atomic_fprintf(FILE * fp, Mutex * mutex, const char * fmt, ...);


extern uint64_t random64();
extern std::string strprintf(const char * fmt, ...);

extern bool parse_size(const char * s, long * n);


extern FILE * logfp;
extern Mutex logfp_mutex;

extern void open_log(const char * logfile);
extern void close_log();
extern void log(const char * fmt, ...);
extern void vlog(const char * fmt, va_list args);


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))


/* 
 * reverse byte order
 */

#define GETBYTE(v, n) ((v) >> ((n)*8) & 0xff)
#define SETBYTE(v, n) ((v) << ((n)*8))

inline uint32_t reverse_byte_order(uint16_t a)
{
	return    SETBYTE(GETBYTE(a, 0), 1)
		| SETBYTE(GETBYTE(a, 1), 0);
}

inline uint32_t reverse_byte_order(uint32_t a)
{
	return    SETBYTE(GETBYTE(a, 0), 3)
		| SETBYTE(GETBYTE(a, 1), 2)
		| SETBYTE(GETBYTE(a, 2), 1)
		| SETBYTE(GETBYTE(a, 3), 0);
}

inline uint64_t reverse_byte_order(uint64_t a)
{
	return    SETBYTE(GETBYTE(a, 0), 7)
		| SETBYTE(GETBYTE(a, 1), 6)
		| SETBYTE(GETBYTE(a, 2), 5)
		| SETBYTE(GETBYTE(a, 3), 4)
		| SETBYTE(GETBYTE(a, 4), 3)
		| SETBYTE(GETBYTE(a, 5), 2)
		| SETBYTE(GETBYTE(a, 6), 1)
		| SETBYTE(GETBYTE(a, 7), 0);
}

#undef GETBYTE
#undef SETBYTE				     



#endif // UTIL_H
