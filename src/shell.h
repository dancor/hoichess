/* $Id: shell.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/shell.h
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
#ifndef SHELL_H
#define SHELL_H

#include "common.h"
#include "board.h"
#include "book.h"
#include "clock.h"
#include "game.h"
#include "hash.h"
#include "search.h"
#include "basic.h"

#include <string>
#include <sstream>
#include <vector>

/* forward declaration */
class Search;

class Shell
{
      private:
	typedef struct {
		std::string name;
		unsigned int line;
		FILE * fp;
	} source_t;
	
      public:
	/* Public, because also accessed from Search. */
	bool xboard;
	
      private:
	bool flag_force;
	bool flag_ponder;
	bool flag_showthinking;
	bool flag_analyze;
	bool flag_playboth;
	
	source_t source;
	std::list<source_t> sources;

	std::string myname;
	
	Game * game;
	Book * book;
	HashTable * hashtable;
	PawnHashTable * pawnhashtable;
	EvaluationCache * evalcache;
	Search * search;
	Color myside;

	bool quit;
	
	/* Some commands (e.g. solve and bench) call search from within a loop.
	 * interrupt() sets this flag to abort those commands. */
	bool stop;

	std::vector<std::string> cmd_args;

      public:
	Shell();
	~Shell();

      public:
	void main(const char * filename = NULL);
	void interrupt();

      public:
	void set_book(const char * bookfile);
	void set_hashsize(unsigned long size);
	void set_pawnhashsize(unsigned long size);
	void set_evalcachesize(unsigned long size);
	void set_myname(const char * name);
	void set_xboard(bool x);

      public:
	void source_file(FILE * fp, const char * name);
      private:
	void input();
	char * get_line(const char * prompt);
	char * get_line_fgets(FILE * fp, const char * prompt);	
#ifdef HAVE_READLINE
	char * get_line_readline(const char * prompt);
#endif
	std::string get_prompt();
	void input_move(std::string input);
	void user_move(Move mov);
	void engine_move();
	void print_result();

      private:
	/* FIXME this should either be const, or not static */
	static struct command {
		const char * name;
		void (Shell::* func) (void);
		bool ignore;
		const char * usage;
	} commands[];

      private:
	void cmd_xboard();
	void cmd_protover();
	void cmd_accepted();
	void cmd_rejected();
	void cmd_new();
	void cmd_variant();
	void cmd_quit();
	void cmd_force();
	void cmd_go();
	void cmd_level();
	void cmd_st();
	void cmd_sd();
	void cmd_time();
	void cmd_otim();
	void cmd_ping();
	void cmd_setboard();
	void cmd_bk();
	void cmd_undo();
	void cmd_remove();
	void cmd_hard();
	void cmd_easy();
	void cmd_post();
	void cmd_nopost();
	void cmd_analyze();
	void cmd_exit();
	
	void cmd_verbose();
	void cmd_debug();
	void cmd_ignore();
	void cmd_obey();
	void cmd_help();
	void cmd_source();
	void cmd_echo();
	void cmd_show();
	void cmd_solve();
	void cmd_bench();
	void cmd_book();
	void cmd_hash();
	void cmd_pawnhash();
	void cmd_evalcache();
	void cmd_set();
	void cmd_get();
	void cmd_playboth();
	void cmd_loadgame();
	void cmd_savegame();
	void cmd_redo();
};

#endif // SHELL_H
