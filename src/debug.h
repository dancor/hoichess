/* $Id: debug.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/debug.h
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
#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>



#ifdef DEBUG
# ifndef DEBUGLEVEL
#  define DEBUGLEVEL 1
# endif
#else // !DEBUG
# define DEBUGLEVEL 0
#endif // !DEBUG



/* We cannot include util.h or thread.h, since they may use something
 * that is declared inside this file. */
class Mutex;
extern Mutex stdout_mutex;


/*
 * Macros like this one here would be nice, but are only supported by gcc.
 *

#define BUG(fmt, arg...) do {	\
	fprintf(stderr, "BUG: %s:%d: %s: " fmt "\n",			      \
			__FILE__, __LINE__, __PRETTY_FUNCTION__, ##arg);      \
	exit(EXIT_FAILURE);	\
} while (0) 

 *
 * The following trick however, is standard C++ and must work
 * with any compiler. It is much more flexible anyway.
 */

class __debug_helper {
	FILE * fp;
	Mutex * mutex;
	const char * file;
	int line;
	const char * function;
      public:
	inline __debug_helper(FILE * fp, Mutex * mutex,
			const char * file, int line, const char * function)
		: fp(fp), mutex(mutex),
		file(file), line(line), function(function)
	{ }
			
	inline ~__debug_helper() { }

      public:
	void __dbg(unsigned int level, const char * fmt, ...);
	void __warn(const char * fmt, ...);
	void __bug(const char * fmt, ...) NORETURN;
};


#define DBG __debug_helper(stdout, &stdout_mutex,	\
		__FILE__, __LINE__, __PRETTY_FUNCTION__).__dbg

#define WARN __debug_helper(stdout, &stdout_mutex,	\
		__FILE__, __LINE__, __PRETTY_FUNCTION__).__warn

#define BUG __debug_helper(stderr, NULL,	\
		__FILE__, __LINE__, __PRETTY_FUNCTION__).__bug


#define ASSERT(expr) do {	\
	if (!(expr)) {		\
		BUG("assertion `%s' failed", #expr);	\
	}	\
} while (0)

#ifdef DEBUG
# define ASSERT_DEBUG(expr) ASSERT(expr)
#else
# define ASSERT_DEBUG(expr)
#endif


void debug_print_compiletime_config();
void debug_print_storagesizes();


#endif // DEBUG_H
