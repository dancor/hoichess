/* $Id: compile.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/compile.h
 *
 * Copyright (C) 2004-2006 Holger Ruckdeschel <holger@hoicher.de>
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
#ifndef COMPILE_H
#define COMPILE_H

#define COMPILE_DATE	__DATE__ " " __TIME__

#if defined(__GNUC__)
# define COMPILER		"GCC"
# define COMPILER_VERSION	 __VERSION__
#elif defined(__INTEL_COMPILER)
# define COMPILER		"ICC"
# define COMPILER_VERSION	"unknown"
#else
# define COMPILER		"unknown"
# define COMPILER_VERSION	"unknown"
#endif

#if defined(__unix__)
# define PLATFORM		"unix"
#elif defined(WIN32) && defined(__MINGW32__)
# define PLATFORM		"mingw32"
#elif defined(WIN32)
# define PLATFORM		"win32"
#else
# define PLATFORM		"unknown"
#endif	

#endif /* COMPILE_H */
