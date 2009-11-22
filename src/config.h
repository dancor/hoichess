/* $Id: config.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/config.h
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
#ifndef CONFIG_H
#define CONFIG_H

/* Default size of main hash table */
#define DEFAULT_HASHSIZE	"32M"

/* Default size of pawn hash table */
#define DEFAULT_PAWNHASHSIZE	"4M"

/* Default size of evaluation cache */
#define DEFAULT_EVALCACHESIZE	"4M"

/* Default location of opening book */
#define DEFAULT_BOOK		"book.dat"

/* Use a single board in search tree, in connection with Board::unmake_move().
 * Otherwise, the board will be copied from node to node each time a move is
 * made. */
//#define USE_UNMAKE_MOVE

/* Use internal iterative deepening */
#define USE_IID

/* Use principal variation search */
#define USE_PVS

/* Use history table */
#define USE_HISTORY

/* Use killer heuristic */
#define USE_KILLER

/* Use null-move pruning */
#define USE_NULLMOVE

/* Use futility pruning */
#define USE_FUTILITYPRUNING

/* Use extended futility pruning */
#define USE_EXTENDED_FUTILITYPRUNING

/* Use razoring */
//#define USE_RAZORING

/* Use evaluation cache */
#define USE_EVALCACHE

/* Search extensions */
#define EXTEND_IN_CHECK
#define EXTEND_RECAPTURE

/* Maximum depth for full-width search */
#define MAXDEPTH		900

/* Maximum search tree depth (full-width + quiescence search) */
#define MAXPLY			1024

/* Maximum size of a Movelist */
#define MOVELIST_MAXSIZE	256

/* Collect statistics, e.g. hash table hits, cutoffs during search, etc. */
#define COLLECT_STATISTICS

/* Collect statistics about maximum number of moves stored in a movelist
 * (expensive!) */
//#define STATS_MOVELIST

#endif // CONFIG_H
