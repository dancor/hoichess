/* $Id: common.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/common.h
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
#ifndef COMMON_H
#define COMMON_H

#include "version.h"
#include "config.h"

#if defined(HOICHESS)
# define PROGNAME	"HoiChess"
#elif defined(HOIXIANGQI)
# define PROGNAME	"HoiXiangqi"
#else
# error "neither HOICHESS nor HOIXIANGQI is defined"
#endif

#define AUTHOR		"Holger Ruckdeschel"
#define AUTHOR_EMAIL	"<holger@hoicher.de>"


/* Get uint64_t and friends */
# include <inttypes.h>


/*
 * Compiler specific definitions
 */

#ifdef __GNUC__
# define FORCEINLINE inline __attribute__((always_inline))
#else
# define FORCEINLINE inline
#endif

#ifdef __GNUC__
# define NORETURN __attribute__((noreturn))
#else
# define NORETURN
#endif

#ifndef __GNUC__
# define __PRETTY_FUNCTION__ __FUNCTION__
#endif



/*
 * Platform specific definitions
 */

#ifdef WIN32

#include <windows.h>

#include <io.h>
#define isatty _isatty

#ifndef __MINGW32__
# include "snprintf.h"
#endif

#include "strtok_r.h"

#endif /* WIN32 */



/*
 * Global variables
 */

/* These are defined in main.cc */
extern unsigned int debug;
extern unsigned int verbose;
extern bool ansicolor;

/* This one is defined in uint64_table.cc */
extern uint64_t uint64_table[];
extern unsigned int uint64_table_size;


/*
 * Include some frequently used stuff
 */

#include "debug.h"
#include "util.h"


/*
 * Miscellaneous
 */

#ifdef COLLECT_STATISTICS
# define STAT_INC(x) ((x)++)
#else
# define STAT_INC(x)
#endif

#endif // COMMON_H
