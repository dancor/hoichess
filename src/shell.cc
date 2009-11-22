/* $Id: shell.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/shell.cc
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
#include "board.h"
#include "book.h"
#include "shell.h"

#include <errno.h> 
#ifdef HAVE_READLINE
# include <readline/readline.h>
# include <readline/history.h>
#endif
#include <stdio.h>
#include <string.h>


Shell::Shell()
{
	xboard = false;
	
	flag_force = false;
	flag_ponder = false;
	flag_showthinking = false;
	flag_analyze = false;
	flag_playboth = false;

	source.fp = NULL;

	myname = PROGNAME + std::string(" ") + VERSION;

	Clock clock(5);
	game = new Game(Board(), clock, clock);	
	book = NULL;
	hashtable = NULL;
	pawnhashtable = NULL;
	evalcache = NULL;
	search = new Search(this);
}

Shell::~Shell()
{
	delete search;
	delete hashtable;
	delete pawnhashtable;
	delete evalcache;
	delete book;
	delete game;
}

void Shell::main(const char * filename)
{
	source_t mainsource;
	if (filename != NULL) {
		mainsource.name = filename;
		mainsource.line = 0;
		mainsource.fp = fopen(filename, "r");
		if (mainsource.fp == NULL) {
			fprintf(stderr, "Cannot open %s for reading: %s\n",
					filename, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
		mainsource.name = "(stdin)";
		mainsource.line = 0;
		mainsource.fp = stdin;
	}
	if (sources.size() != 0) {
		sources.pop_front();
		sources.push_front(mainsource);
	} else {
		source = mainsource;
	}
	
	cmd_new();

	if (!xboard && source.fp == stdin) {
		printf("\nType \"help\" for a list of available commands.\n\n");
	}
	
	quit = false;
	while (!quit) {
		stop = false;
		
		if (game->is_over()) {
			/* nothing */
		} else if (flag_playboth) {
			/* let engine make a move */
			myside = game->get_side();
			engine_move();
			continue;
		} else if (!flag_force && game->get_side() == myside) {
			/* let engine make a move */
			engine_move();
			continue;
		} else if (flag_ponder && !flag_force && game->is_running()) {
			/* start search in background */
			DBG(1, "starting background search...");
			search->start_thread(*game, Search::PONDER, myside);
		} else if (flag_analyze) {
			/* start search in background */
			DBG(1, "starting background search...");
			search->start_thread(*game, Search::ANALYZE, NO_COLOR);
		}

		input();
	}

	search->stop_thread();
}

/* 
 * This function will be called by the SIGINT-handler
 * in main.cc, so keep it short.
 */
void Shell::interrupt()
{
	if (!xboard) {
		printf("Interrupt\n");
	}
	search->interrupt();
	stop = true;

	flag_playboth = false;
}

/*
 * Set the opening book. If bookfile is NULL, disable opening book.
 */
void Shell::set_book(const char * bookfile)
{
	if (bookfile) {
		try {
			delete book;
			book = new Book(bookfile);
			printf("Opening book: %s\n", bookfile);
		} catch (BookException & e) {
			if (!xboard) {
				printf("%s", e.get_msg().c_str());
			}
			printf("Failed to open opening book.\n");
			book = NULL;
		}
	} else {
		printf("Opening book disabled.\n");
		delete book;
		book = NULL;
	}

	search->set_book(book);
}

/*
 * Set the size of the hash table in bytes. 0 disables hash table.
 */
void Shell::set_hashsize(unsigned long size)
{
	search->stop_thread();
	
	unsigned long entries = size / sizeof(HashEntry);
	if (entries > 0) {
		delete hashtable;
		hashtable = new HashTable(entries);
		hashtable->print_info();
	} else {
		delete hashtable;
		hashtable = NULL;
		printf("Hash table disabled.\n");
	}

	search->set_hashtable(hashtable);
}

/*
 * Set the size of the pawn hash table in bytes. 0 disables pawn hash table.
 */
void Shell::set_pawnhashsize(unsigned long size)
{
	search->stop_thread();
	
	unsigned long entries = size / sizeof(PawnHashEntry);
	if (entries > 0) {
		delete pawnhashtable;
		pawnhashtable = new PawnHashTable(entries);
		pawnhashtable->print_info();
	} else {
		delete pawnhashtable;
		pawnhashtable = NULL;
		printf("Pawn hash table disabled.\n");
	}

	search->set_pawnhashtable(pawnhashtable);
}

/*
 * Set the size of the evaluation cache in bytes. 0 disables evaluation cache.
 */
void Shell::set_evalcachesize(unsigned long size)
{
	search->stop_thread();
	
#ifdef USE_EVALCACHE
	unsigned long entries = size / EvaluationCache::SIZEOF_ENTRY;
	if (entries > 0) {
		delete evalcache;
		evalcache = new EvaluationCache(entries);
		evalcache->print_info();
	} else {
		delete evalcache;
		evalcache = NULL;
		printf("Evaluation cache disabled.\n");
	}

	search->set_evalcache(evalcache);
#else
	(void) size;
	WARN("This version of %s has been compiled without"
			" evaluation cache support.\n", PROGNAME);
#endif
}

/*
 * Set the engine's name.
 */
void Shell::set_myname(const char * name)
{
	if (name) {
		myname = name;
	} else {
		myname = PROGNAME + std::string(" ") + VERSION;
	}
}

/*
 * Set/unset xboard mode.
 */
void Shell::set_xboard(bool x)
{
	xboard = x;
	if (xboard) {
		setbuf(stdout, NULL);
		printf("\n"); 
	}
}


void Shell::source_file(FILE * fp, const char * name)
{
	ASSERT(fp != NULL);

	/* save current source */
	sources.push_back(source);

	/* open new source */
	source.name = name;
	source.line = 0;
	source.fp = fp;
}

void Shell::input()
{
	cmd_args.clear();
	
	if (xboard) {
		DBG(3, "waiting for input...");
	}
	
	char * line = get_line(get_prompt().c_str());
	if (line == NULL) {
		quit = true;
		return;
	}
	
	/* Strip trailing \n */
	if (line[strlen(line)-1] == '\n') {
		line[strlen(line)-1] = '\0';
	}

	DBG(1, "%s:%d: line=\"%s\"", source.name.c_str(), source.line, line);

	/* Tokenize the input. */
	const char * delim = " \t\n";
	char * strtok_r_buf;
	for (char * p = strtok_r(line, delim, &strtok_r_buf); 
			p != NULL;
			p = strtok_r(NULL, delim, &strtok_r_buf)) {
		//DBG(3, "p = \"%s\"", p);
		cmd_args.push_back(p);
	}

	free(line);
	
	/* Ignore empty commands and comments. */
	if (cmd_args.size() == 0 || cmd_args[0][0] == '#') {
		return;
	}
	
	/* Walk through the list of registered commands. */
	for (int i=0; commands[i].name != NULL; i++) {
		if (cmd_args[0] != commands[i].name)
			continue;
		
		/* If a function is registered, and the command is not
		 * to be ignores, call the corresponding function. */ 
		if (commands[i].ignore) {
			return;
		} else if (commands[i].func != NULL) { 
			(this->*commands[i].func)();
			return;
		} else {
			printf("Error (command not implemented): %s\n",
					cmd_args[0].c_str());
			return;
		}
	}

	/* The input wasn't recognized as a command,
	 * so check if it is a move. */
	if (cmd_args.size() == 1) {
		input_move(cmd_args[0]);
		return;
	}

	printf("Error (unknown command): %s\n", cmd_args[0].c_str());
	return;
}

char * Shell::get_line(const char * prompt)
{
again:
	char * line;

	if (source.fp == NULL) {
		return NULL;
	}

	int fd = fileno(source.fp);
	if (!xboard && fd == 0 && isatty(0)) {
#ifdef HAVE_READLINE
		line = get_line_readline(prompt);
#else
		line = get_line_fgets(source.fp, prompt);
#endif
	} else {
		line = get_line_fgets(source.fp, NULL);
		
	}	
	source.line++;
	
	if (line == NULL && sources.size() > 0) {
		fclose(source.fp);
		source = sources.back();
		sources.pop_back();
		goto again;
	}
		
	return line;
}

char * Shell::get_line_fgets(FILE * fp, const char * prompt)
{
	ASSERT(fp != NULL);
	if (prompt) {
		printf("%s", prompt);
	}

	char buf[1024];
	char * p = fgets(buf, sizeof(buf)-1, fp);
	if (p != NULL) {
		char * ret = (char *) malloc((strlen(p)+1) * sizeof(char));
		strcpy(ret, p);
		return ret;
	} else {
		return NULL;
	}
}

#ifdef HAVE_READLINE
char * Shell::get_line_readline(const char * prompt)
{
	char * line = readline(prompt);
	if (line && *line) {
		add_history(line);
	}
	return line;
}
#endif

std::string Shell::get_prompt()
{
	const char * a1 = ansicolor ? "\033[1m" : "";
	const char * a2 = ansicolor ? "\033[0m" : "";
		
	if (game->is_over()) {
		return strprintf("%s(game over):%s ", a1, a2);
	}

	std::string s = a1;

	if (flag_analyze) {
		s += "(analyze mode) ";
	} else if (flag_ponder && !flag_force && game->is_running()) {
		s += "(pondering) ";
	}
	
	s += strprintf("%s (%d): ",
			game->get_board().get_side() == WHITE 
					? "White" : "Black",
			game->get_board().get_moveno());

	s += a2;
	
	if (flag_analyze || (flag_ponder && flag_showthinking)) {
		s += "\n";
	}

	return s;
}

void Shell::input_move(std::string input)
{
	if (game->is_over()) {
		printf("Illegal move (game over): %s\n", input.c_str());
		return;
	} else if (game->get_side() == myside) {
		printf("Illegal move (it's my turn): %s\n", input.c_str());
		return;
	}

	Board board = game->get_board();
	Move mov = board.parse_move(input);
	if (mov) {
		search->stop_thread();
		user_move(mov);
	} else {
		printf("Illegal move: %s\n", input.c_str());
	}
}

void Shell::user_move(Move mov)
{
	Board board = game->get_board();
	
	if (!mov.is_valid(board)) {
		BUG("user_move() called with invalid move: %s",
				mov.str().c_str());
	} else if (!mov.is_legal(board)) {
		BUG("user_move() called with illegal move: %s",
				mov.str().c_str());
	}
	
	std::string san = mov.san(board);

	GameEntry::MoveAttributes move_attr(false, false);
	game->make_move(mov, move_attr);

	if (!xboard) {
		printf("\n");
		game->get_board().print(stdout, mov);
		printf("\nYour move was: %s\n\n", san.c_str());
	}
	//game->get_board().print(stdout, mov);

	DBG(1, "user move: %s", san.c_str());

	DBG(1, "result = %d", game->get_result());
	if (game->get_result()) {
		print_result();
	}
}

void Shell::engine_move()
{
	if (!xboard) {
		 printf("Thinking...\n");
	}

	/* Turn back the current side's clock, so we can think the full amount
	 * of time, even if we've switched from human to engine. */
	Clock clock = game->get_clock();
	clock.turn_back();
	game->set_clock(clock);

	game->start();
	
	Board board = game->get_board();
	Move mov;
	bool bookmove;

	BookEntry bookentry;
	if (book && book->lookup(board, &bookentry)) {
		mov = bookentry.choose();
		bookmove = true;
		if (!mov.is_valid(board)) {
			BUG("book returned invalid move: %s",
					mov.str().c_str());
		} else if (!mov.is_legal(board)) {
			BUG("book returned illegal move: %s",
					mov.str().c_str());
		}
	} else {
		DBG(1, "starting search...");
		search->start(*game, Search::MOVE, myside);
		DBG(1, "search terminated");
		mov = search->get_best();
		bookmove = false;
		if (!mov.is_valid(board)) {
			BUG("search returned invalid move: %s",
					mov.str().c_str());
		} else if (!mov.is_legal(board)) {
			BUG("search returned illegal move: %s",
					mov.str().c_str());
		}

	}

	std::string san = mov.san(board);
	
	GameEntry::MoveAttributes move_attr(true, bookmove);
	game->make_move(mov, move_attr);

	if (xboard) {
		std::string str = mov.str();
		atomic_printf("move %s\n", str.c_str());
	} else {
		printf("\n");
		game->get_board().print(stdout, mov);
		printf("\nMy move is: %s\n\n", san.c_str());
	}
	//game->get_board().print(stdout, mov);
	
	DBG(1, "engine move: %s", san.c_str());

	DBG(1, "result = %d", game->get_result());
	if (game->get_result()) {
		print_result();
	}
}

void Shell::print_result()
{
	atomic_printf("%s {%s}\n",
			game->get_result_str().c_str(),
			game->get_result_comment().c_str());
}

