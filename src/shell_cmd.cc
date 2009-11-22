/* $Id: shell_cmd.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/shell_cmd.cc
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
#include "shell.h"
#include "bench.h"
#include "pgn.h"

#include <errno.h>
#include <stdio.h>

#include <sstream>

struct Shell::command Shell::commands[] = {
	/* xboard protocol commands */
	{ "xboard",	&Shell::cmd_xboard,	false,	""	},
	{ "protover",	&Shell::cmd_protover,	false,	""	},
	{ "accepted",	&Shell::cmd_accepted,	false,	""	},
	{ "rejected",	&Shell::cmd_rejected,	false,	""	},
	{ "new",	&Shell::cmd_new,	false,	"Start a new game" },
	{ "variant",	&Shell::cmd_variant,	false,  ""	},
	{ "quit",	&Shell::cmd_quit,	false,	"Quit" },
	{ "random",	NULL,			true,	""	},
	{ "force",	&Shell::cmd_force,	false,	"Let engine make no moves at all" },
	{ "go",		&Shell::cmd_go,		false,	"Switch sides, let computer make next move" },
	{ "playother",	NULL,			false,	""	},
	{ "white",	NULL,			false,	""	},
	{ "black",	NULL,			false,	""	},
	{ "level",	&Shell::cmd_level,	false,	""	},
	{ "st",		&Shell::cmd_st,		false,	""	},
	{ "sd",		&Shell::cmd_sd,		false,	""	},
	{ "time",	&Shell::cmd_time,	false,	""	},
	{ "otim",	&Shell::cmd_otim,	false,	""	},
	{ "usermove",	NULL,			false,	""	},
	{ "?",		NULL,			false,	""	},
	{ "ping",	&Shell::cmd_ping,	false,	""	},
	{ "draw",	NULL,			false,	""	},
	{ "result",	NULL,			true,	""	},
	{ "setboard",	&Shell::cmd_setboard,	false,	""	},
	{ "edit",	NULL,			false,	""	},
	{ "hint",	NULL,			false,	""	},
	{ "bk",		&Shell::cmd_bk,		false,	""	},
	{ "undo",	&Shell::cmd_undo,	false,	""	},
	{ "remove",	&Shell::cmd_remove,	false,	""	},
	{ "hard",	&Shell::cmd_hard,	false,	"Turn on pondering (thinking on opponent's time)" },
	{ "easy",	&Shell::cmd_easy,	false,	"Turn off pondering" },
	{ "post",	&Shell::cmd_post,	false,	"Show thinking output" },
	{ "nopost",	&Shell::cmd_nopost,	false,	"Hide thinking output" },
	{ "analyze",	&Shell::cmd_analyze,	false,	"Enter analysis mode" },
	{ "exit",	&Shell::cmd_exit,	false,	"Leave analysis mode" },
	{ "name",	NULL,			true,	""	},
	{ "rating",	NULL,			false,	""	},
	{ "ics",	NULL,			false,	""	},
	{ "computer",	NULL,			true,	""	},
	{ "pause",	NULL,			false,	""	},
	{ "resume",	NULL,			false,	""	},
	{ ".",		NULL,			true,	""	},
	
	/* own commands */
	{ "verbose",	&Shell::cmd_verbose,	false,	""	},
	{ "debug",	&Shell::cmd_debug,	false,	""	},
	{ "ignore",	&Shell::cmd_ignore,	false,	""	},
	{ "obey",	&Shell::cmd_obey,	false,  ""	},
	{ "help",	&Shell::cmd_help,	false,	""	},
	{ "source",	&Shell::cmd_source,	false,	""	},
	{ "echo",	&Shell::cmd_echo,	false,	""	},
	{ "show",	&Shell::cmd_show,	false,	""	},
	{ "solve",	&Shell::cmd_solve,	false,	""	},
	{ "bench",	&Shell::cmd_bench,	false,	""	},
	{ "book",	&Shell::cmd_book,	false,	""	},
	{ "hash",	&Shell::cmd_hash,	false,	""	},
	{ "pawnhash",	&Shell::cmd_pawnhash,	false,	""	},
	{ "evalcache",	&Shell::cmd_evalcache,	false,	""	},
	{ "set",	&Shell::cmd_set,	false,	""	},
	{ "get",	&Shell::cmd_get,	false,	""	},
	{ "playboth",	&Shell::cmd_playboth,	false,	""	},
	{ "loadgame",	&Shell::cmd_loadgame,	false,	""	},
	{ "savegame",	&Shell::cmd_savegame,	false,	""	},
	{ "redo",	&Shell::cmd_redo,	false,	""	},
	
	{ NULL, NULL, false, NULL }
};


#define CMD_REQUIRE_ARGS(n) do {					\
	if (cmd_args.size() < (n)+1) {					\
		printf("Error: Command requires %d argument%s.\n",	\
				(n), ((n) == 1 ? "" : "s"));		\
		return;							\
	}								\
} while(0)



void Shell::cmd_xboard()
{
	set_xboard(true);
}

void Shell::cmd_protover()
{
	CMD_REQUIRE_ARGS(1);

	std::ostringstream ss;
	ss << "feature";
	ss << " myname=\"" << myname << "\"";

#if defined(HOICHESS)
	ss << " variants=\"normal\"";
#elif defined(HOIXIANGQI)
	ss << " variants=\"xiangqi\"";
#else
# error "neither HOICHESS nor HOIXIANGQI defined"
#endif
	
	ss << " ping=1 setboard=1 time=1 sigint=0 colors=0";

#if defined(HAVE_PTHREAD) || defined(WIN32)
	ss << " analyze=1";
#else
	ss << " analyze=0";
#endif

	ss << " name=1";
	ss << " done=1\n";

	atomic_printf("%s", ss.str().c_str());
}

void Shell::cmd_accepted()
{
	if (!xboard) {
		printf("yeah!\n");
	}
}

void Shell::cmd_rejected()
{
	CMD_REQUIRE_ARGS(1);
	
	printf("tellusererror Feature `%s' was rejected, expect problems\n",
			cmd_args[1].c_str());
}

void Shell::cmd_new()
{
	search->stop_thread();
	
	if (!game->set_board(opening_fen())) {
		BUG("Failed to set up standard opening position");
	}

	if (!flag_analyze) {
		flag_force = false;
		myside = BLACK;
	}
}

void Shell::cmd_variant()
{
	CMD_REQUIRE_ARGS(1);

	const std::string& v = cmd_args[1];

#if defined(HOICHESS)
	if (0) {
		/* for standard chess, the 'variant' command is never sent */
#elif defined(HOIXIANGQI)
	if (v == "xiangqi") {
		/* used by Winboard_F by H.G. Muller */
#else
# error "neither HOICHESS nor HOIXIANGQI defined"
#endif
	} else {
		printf("Error (variant not supported): %s\n", v.c_str());
	}
}

void Shell::cmd_quit()
{
	quit = true;
}

void Shell::cmd_force()
{
	search->stop_thread();
	flag_force = true;
	flag_playboth = false;
	myside = NO_COLOR;
}

void Shell::cmd_go()
{
	search->stop_thread();
	flag_force = false;
	flag_playboth = false;
	myside = game->get_side();
}

void Shell::cmd_level()
{
	CMD_REQUIRE_ARGS(3);
	
	int moves;
	if (sscanf(cmd_args[1].c_str(), "%d", &moves) != 1) {
		moves = -1;
	}

	/* Argument might be '5' (= 5 minutes) or '0:30' (= 30 seconds) */
	int mins, secs;
	if (sscanf(cmd_args[2].c_str(), "%d:%d", &mins, &secs) != 2) {
		secs = 0;
		if (sscanf(cmd_args[2].c_str(), "%d", &mins) != 1) {
			mins = -1;
		}
	}
	secs = mins * 60 + secs;
	
	int inc;
	if (sscanf(cmd_args[3].c_str(), "%d", &inc) != 1) {
		inc = -1;
	}

	if (moves < 0 || secs <= 0 || inc < 0) {
		printf("Illegal argument to command `level': %s %s %s\n",
				cmd_args[1].c_str(), 
				cmd_args[2].c_str(),
				cmd_args[3].c_str());
		return;
	}

	Clock clock(moves, secs, inc);
	game->set_clocks(clock, clock);
}

void Shell::cmd_st()
{
	CMD_REQUIRE_ARGS(1);
	
	int secs = atoi(cmd_args[1].c_str());
	if (secs > 0) {
		Clock clock(secs);
		game->set_clocks(clock, clock);
	} else {
		printf("Illegal time value.\n");
	}
}

void Shell::cmd_sd()
{
	CMD_REQUIRE_ARGS(1);
	
	unsigned int depth = atoi(cmd_args[1].c_str());
	if (depth == 0) {
		printf("Unlimited search depth.\n");
	} else {
		printf("Search depth limited to %u ply.\n", depth);
	}
	search->set_depthlimit(depth);
}

void Shell::cmd_time()
{
	CMD_REQUIRE_ARGS(1);

	unsigned int csecs;
	if (sscanf(cmd_args[1].c_str(), "%u", &csecs) != 1) {
		printf("Illegal argument to command 'time': %s\n",
				cmd_args[1].c_str());
		return;
	}

	if (myside != NO_COLOR) {
		Clock clock = game->get_clock(myside);
		clock.set_remaining_time(csecs);
		game->set_clock(myside, clock);
	}
}

void Shell::cmd_otim()
{
	CMD_REQUIRE_ARGS(1);

	unsigned int csecs;
	if (sscanf(cmd_args[1].c_str(), "%u", &csecs) != 1) {
		printf("Illegal argument to command 'otim': %s\n",
				cmd_args[1].c_str());
		return;
	}

	if (myside != NO_COLOR) {
		Clock clock = game->get_clock(XSIDE(myside));
		clock.set_remaining_time(csecs);
		game->set_clock(XSIDE(myside), clock);
	}
}

void Shell::cmd_ping()
{
	CMD_REQUIRE_ARGS(1);
	
	atomic_printf("pong %s\n", cmd_args[1].c_str());
}

void Shell::cmd_setboard()
{
	search->stop_thread();
	
	CMD_REQUIRE_ARGS(6);
	
	std::string fen = cmd_args[1] + " " + cmd_args[2] + " "
		+ cmd_args[3] + " " + cmd_args[4] + " "
		+ cmd_args[5] + " " + cmd_args[6];
	
	if (!game->set_board(fen.c_str())) {
		if (xboard) {
			printf("tellusererror Illegal position\n");
		} else {
			printf("Error (illegal position): %s\n",
					fen.c_str());
		}
		return;
	}
	
	print_result();
}

void Shell::cmd_bk()
{
	BookEntry entry;
	if (book && book->lookup(game->get_board(), &entry)) {
		entry.print(game->get_board());
	} else {
		printf(" Nothing found in book\n");
	}
	
	/* Must finish with an empty line */
	printf("\n");
}

void Shell::cmd_undo()
{
	search->stop_thread();
	if (cmd_args.size() == 2 && cmd_args[1] == "all") {
		while (game->undo_move()) {}		
	} else {
		if (!game->undo_move()) {
			if (!xboard) {
				printf("No move to be undone.\n");
			}
		}
	}
}

void Shell::cmd_remove()
{
	search->stop_thread();
	game->undo_move();
	game->undo_move();
}

void Shell::cmd_hard()
{
	flag_ponder = true;
}

void Shell::cmd_easy()
{
	search->stop_thread();
	flag_ponder = false;
}

void Shell::cmd_post()
{
	flag_showthinking = true;
	search->set_showthinking(true);
}

void Shell::cmd_nopost()
{
	flag_showthinking = false;
	search->set_showthinking(false);
}

void Shell::cmd_analyze()
{
	search->stop_thread();
	cmd_force();
	cmd_post();
	flag_analyze = true;
}

void Shell::cmd_exit()
{
	search->stop_thread();
	flag_analyze = false;
}

void Shell::cmd_verbose()
{
	if (cmd_args.size() == 2) {
		unsigned int tmp;
		if (sscanf(cmd_args[1].c_str(), "%d", &tmp) == 1) {
			verbose = tmp;
			printf("Verbosity set to %d.\n", verbose);
		}
	} else {
		printf("Verbosity set to %d.\n", verbose);
	}
}

void Shell::cmd_debug()
{
	if(cmd_args.size() == 2) {
		unsigned int tmp;
		if (sscanf(cmd_args[1].c_str(), "%d", &tmp) == 1) {
			debug = tmp;
			printf("Debug level set to %d.\n", debug);
		}
	} else {
		printf("Debug level set to %d.\n", debug);
	}
}

void Shell::cmd_ignore()
{
	CMD_REQUIRE_ARGS(1);

	for (int i=0; commands[i].name != NULL; i++) {
		if (cmd_args[1] != commands[i].name)
			continue;

		commands[i].ignore = true;
		if (!xboard) {
			printf("Ignoring command '%s' from now on.\n",
					cmd_args[1].c_str());
		}
		return;
	}

	printf("Error (unknown command): %s\n", cmd_args[1].c_str());
}

void Shell::cmd_obey()
{
	CMD_REQUIRE_ARGS(1);

	for (int i=0; commands[i].name != NULL; i++) {
		if (cmd_args[1] != commands[i].name)
			continue;

		commands[i].ignore = false;
		if (!xboard) {
			printf("Won't ignore command '%s' anymore.\n",
					cmd_args[1].c_str());
		}
		return;
	}

	printf("Error (unknown command): %s\n", cmd_args[1].c_str());
}	

void Shell::cmd_help()
{
	printf("Available commands:\n");

	printf("\t<move>\t\tPlay move (coordinate notation or SAN)\n");
	for (int i=0; commands[i].name != NULL; i++) {
		if (commands[i].func == NULL || commands[i].ignore)
			continue;

		printf("\t%s\t\t%s\n", commands[i].name, commands[i].usage);
	}
}

void Shell::cmd_source()
{
	CMD_REQUIRE_ARGS(1);
	const char * filename = cmd_args[1].c_str();

	FILE * fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s for reading: %s\n",
				filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	source_file(fp, filename);
}

void Shell::cmd_echo()
{
	std::string s;
	for (unsigned int i=1; i<cmd_args.size(); i++) {
		if (i > 1) {
			s += " ";
		}
		s += cmd_args[i];
	}
	atomic_printf("%s\n", s.c_str());
}

void Shell::cmd_show()
{
	CMD_REQUIRE_ARGS(1);
	std::string param = cmd_args[1];
	
	const Board & board = game->get_board();
	
	if (param == "board") {
		board.print();
	} else if (param == "fen") {
		printf("%s\n", board.get_fen().c_str());
	} else if (param == "moves" || param == "captures"
			|| param == "noncaptures" || param == "escapes") {
		Movelist moves;
		if (param == "moves") {
			board.generate_moves(&moves);
		} else if (param == "captures") {
			board.generate_captures(&moves);
		} else if (param == "noncaptures") {
			board.generate_noncaptures(&moves);
		} else if (param == "escapes") {
			board.generate_escapes(&moves);
		} else {
			BUG("param == %s", param.c_str());
		}
		moves.filter_illegal(board);
		
		unsigned int j=1;
		for (unsigned int i=0; i<moves.size(); i++) {
			printf("%s\t", moves[i].san(board).c_str());
			j++;
			if (j == 8) {
				printf("\n");
				j=1;
			}
		}
		if (j != 1 && j != 8)
			printf("\n");
	} else if (param == "eval") {
		Evaluator eval;
		printf("Symmetric evaluation:\n");
		eval.print_eval(board, NO_COLOR);
		printf("--------------------------------------------------\n");
		printf("Evaluation if I would play white:\n");
		eval.print_eval(board, WHITE);
		printf("--------------------------------------------------\n");
		printf("Evaluation if I would play black:\n");
		eval.print_eval(board, BLACK);
	} else if (param == "clocks") {
		printf("[White]\n"); game->get_clock(WHITE).print();
		printf("\n[Black]\n"); game->get_clock(BLACK).print();
	} else if (param == "game") {
		game->print(stdout);
	} else if (param == "pgn") {
		game->write_pgn(stdout);
	} else {
		printf("Usage: show {board|fen}\n");
		printf("       show {moves|captures|noncaptures|escapes}\n");
		printf("       show eval\n");
		printf("       show clocks\n");
		printf("       show game\n");
		printf("       show pgn\n");
	}
}

void Shell::cmd_solve()
{
	search->stop_thread();
	
	CMD_REQUIRE_ARGS(1);
	const char * filename = cmd_args[1].c_str();
	
	FILE * fp;
	if ((fp = fopen(filename, "r")) == NULL) {
		printf("Cannot open %s: %s\n", filename, strerror(errno));
		return;
	}

	log("solve: %s\n", filename);

	int right = 0;
	int wrong = 0;
	int total = 0;
	int skipped = 0;
	
	char buf[1024];
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		printf("--------------------------------------------------\n");
		
		/* Parse EPD, print FEN and board */
		EPD epd(buf);
		Board board(epd.get_fen().c_str());
		printf("[%s] %s\n", 
				epd.get1("id").c_str(),
				epd.get_fen().c_str());
		printf("\n");
		board.print_small();
		printf("\n");

		/* Get the list of best moves */
		std::list<std::string> bms = epd.get("bm");
		std::list<std::string>::const_iterator it;
		if (bms.size() == 0) {
			printf("No best move associated to this position,"
					" skipping.\n");
			skipped++;
			continue;
		}
		
		/* Print the list of best moves */
		printf("Best move:");
		for (it = bms.begin(); it != bms.end(); it++)
			printf(" %s", it->c_str());
		printf("\n");

		/* Start search */
		if (hashtable) {
			printf("Clearing hash table.\n");
			hashtable->clear();
		}
		if (pawnhashtable) {
			printf("Clearing pawn hash table.\n");
			pawnhashtable->clear();
		}
		if (evalcache) {
			printf("Clearing evaluation cache.\n");
			evalcache->clear();
		}
		printf("Thinking...\n");
		search->start(board, game->get_clock(), Search::MOVE);

		Move mov = search->get_best();
		
		/* Look if our move is among the best */
		bool correct = false;
		for (it = bms.begin(); it != bms.end(); it++) {
			if (*it == mov.san(board)) {
				right++;
				correct = true;
				break;
			}
		}
		if (it == bms.end()) {
			wrong++;
			correct = false;
		}
		
		printf("My move: %s (%s)\n", mov.san(board).c_str(),
				correct ? "correct" : "incorrect");
		log("position: %s\tmove: %s\t(%s)\n",
				epd.get1("id").c_str(),
				mov.san(board).c_str(),
				correct ? "correct" : "incorrect");
		
		total = right + wrong;
		printf("Correct: %d of %d (%d%%), skipped: %d\n",
				right, total, 100 * right / total, skipped);

		if (stop) {
			break;
		}
	}	

	log("solve finished: correct %d/%d\n", right, total);
			
	fclose(fp);
}

void Shell::cmd_bench()
{
	search->stop_thread();
	
	CMD_REQUIRE_ARGS(1);
	const std::string type = cmd_args[1];
	
	if (type == "movegen") {
		Bench bench;
		bench.bench_movegen();
	} else if (type == "evaluator") {
		Bench bench;
		bench.bench_evaluator();
	} else if (type == "makemove") {
		Bench bench;
		bench.bench_makemove();
	} else {
		printf("Usage: bench movegen\n");
		printf("       bench evaluator\n");
		printf("       bench makemove\n");
		return;
	}
}

void Shell::cmd_book()
{
	CMD_REQUIRE_ARGS(1);
	const std::string param = cmd_args[1];

	if (param == "close" || param == "off") {
		delete book;
		book = NULL;
	} else if (param == "open") {
		CMD_REQUIRE_ARGS(2);
		const char * file = cmd_args[2].c_str();
		set_book(file);
	} else if (param == "create") {
		CMD_REQUIRE_ARGS(5);
		const char * destfile = cmd_args[2].c_str();
		const char * srcfile = cmd_args[3].c_str();
		int depth;
		if (sscanf(cmd_args[4].c_str(), "%d", &depth) != 1) {
			printf("Error: argument <depth> must be non-negative"
					" integer\n");
			return;
		}
		int min_move_count;
		if (sscanf(cmd_args[5].c_str(), "%d", &min_move_count) != 1) {
			printf("Error: argument <min_move_count> must be"
					" non-negative integer\n");
			return;
		}

		printf("Creating opening book `%s' from `%s' ...\n",
				destfile, srcfile);
		Book::create_from_pgn(destfile, srcfile, depth, min_move_count);
	} else {
		printf("Usage: book close\n");
		printf("       book open <bookfile>\n");
		printf("       book create <bookfile> <pgnfile> <depth>"
							" <min_move_count>\n");
	}
}

void Shell::cmd_hash()
{
	CMD_REQUIRE_ARGS(1);
	const std::string param = cmd_args[1];

	if (param == "clear") {
		if (hashtable) {
			search->stop_thread();
			hashtable->clear();
			printf("Hash table cleared.\n");
		} else {
			printf("Error: hash table is disabled\n");
		}
	} else if (param == "size") {
	 	CMD_REQUIRE_ARGS(2);
		const char * s = cmd_args[2].c_str();
		long size;
		if (!parse_size(s, &size) || size < 0) {
			printf("Illegal value for hash table size: %s\n",
					s);
			return;
		}
		search->stop_thread();
		set_hashsize((unsigned) size);
	} else if (param == "off") {
		search->stop_thread();
		set_hashsize(0);
	} else if (param == "info") {
		if (hashtable) {
			hashtable->print_info();
		} else {
			printf("Hash table is disabled.\n");
		}
	} else if (param == "stats") {
		if (hashtable) {
			hashtable->print_statistics();
		} else {
			printf("Error: hash table is disabled\n");
		}
	} else if (param == "replace") {
		CMD_REQUIRE_ARGS(2);
		if (hashtable) {
			hashtable->set_replacement_scheme(cmd_args[2]);
		} else {
			printf("Error: hash table is disabled\n");
		}
	} else {
		printf("Usage: hash clear\n");
		printf("       hash size <size>\n");
		printf("       hash off\n");
		printf("       hash info\n");
		printf("       hash stats\n");
	}
}

void Shell::cmd_pawnhash()
{
	CMD_REQUIRE_ARGS(1);
	const std::string param = cmd_args[1];

	if (param == "clear") {
		if (pawnhashtable) {
			search->stop_thread();
			pawnhashtable->clear();
			printf("Pawn hash table cleared.\n");
		} else {
			printf("Error: pawn hash table is disabled\n");
		}
	} else if (param == "size") {
	 	CMD_REQUIRE_ARGS(2);
		const char * s = cmd_args[2].c_str();
		long size;
		if (!parse_size(s, &size) || size < 0) {
			printf("Illegal value for pawn hash table size: %s\n",
					s);
			return;
		}
		search->stop_thread();
		set_pawnhashsize((unsigned) size);
	} else if (param == "off") {
		search->stop_thread();
		set_pawnhashsize(0);
	} else if (param == "info") {
		if (pawnhashtable) {
			pawnhashtable->print_info();
		} else {
			printf("Pawn hash table is disabled.\n");
		}
	} else if (param == "stats") {
		if (pawnhashtable) {
			pawnhashtable->print_statistics();
		} else {
			printf("Error: Pawn hash table is disabled\n");
		}
	} else {
		printf("Usage: pawnhash clear\n");
		printf("       pawnhash size <size>\n");
		printf("       pawnhash off\n");
		printf("       pawnhash info\n");
		printf("       pawnhash stats\n");
	}
}

void Shell::cmd_evalcache()
{
	CMD_REQUIRE_ARGS(1);
	const std::string param = cmd_args[1];

#ifdef USE_EVALCACHE
	if (param == "clear") {
		if (evalcache) {
			search->stop_thread();
			evalcache->clear();
			printf("Evaluation cache cleared.\n");
		} else {
			printf("Error: evaluation cache is disabled\n");
		}
	} else if (param == "size") {
	 	CMD_REQUIRE_ARGS(2);
		const char * s = cmd_args[2].c_str();
		long size;
		if (!parse_size(s, &size) || size < 0) {
			printf("Illegal value for evaluation cache size: %s\n",
					s);
			return;
		}
		search->stop_thread();
		set_evalcachesize((unsigned) size);
	} else if (param == "off") {
		search->stop_thread();
		set_evalcachesize(0);
	} else if (param == "info") {
		if (evalcache) {
			evalcache->print_info();
		} else {
			printf("Evaluation cache is disabled.\n");
		}
	} else if (param == "stats") {
		if (evalcache) {
			evalcache->print_statistics();
		} else {
			printf("Error: evaluation cache is disabled\n");
		}
	} else {
		printf("Usage: evalcache clear\n");
		printf("       evalcache size <size>\n");
		printf("       evalcache off\n");
		printf("       evalcache info\n");
		printf("       evalcache stats\n");
	}
#else
	printf("Error: This version of %s has been compiled without"
				" evaluation cache support.\n", PROGNAME);
#endif
}

void Shell::cmd_set()
{
	CMD_REQUIRE_ARGS(1);

	if (cmd_args[1] == "myname") {
		CMD_REQUIRE_ARGS(2);
		set_myname(cmd_args[2].c_str());
		printf("myname set to %s\n", cmd_args[2].c_str());
	} else if (cmd_args[1] == "searchparam") {
		CMD_REQUIRE_ARGS(3);
		const std::string& name = cmd_args[2];
		const std::string& value = cmd_args[3];
		search->set_param(name, value);
	} else if (cmd_args[1] == "evalparam") {
		CMD_REQUIRE_ARGS(3);
		const std::string& name = cmd_args[2];
		const std::string& value = cmd_args[3];
		search->get_evaluator()->set_param(name, value);
	} else {
		printf("Illegal argument to command 'set': '%s'\n",
				cmd_args[1].c_str());
	}
}

void Shell::cmd_get()
{
	CMD_REQUIRE_ARGS(1);

	if (cmd_args[1] == "myname") {
		printf("myname = %s\n", myname.c_str());
#if 0
	} else if (cmd_args[1] == "searchparam") {
		//unsigned long features = search->get_features();
		//printf("search_features = 0x%lx\n", features);
#endif
	} else {
		printf("Illegal argument to command 'get': '%s'\n",
				cmd_args[1].c_str());
	}
}

void Shell::cmd_playboth()
{
	search->stop_thread();
	flag_playboth = true;
	flag_force = false;
}

void Shell::cmd_loadgame()
{
	CMD_REQUIRE_ARGS(1);
	const char * pgnfile  = cmd_args[1].c_str();

	FILE* fp = fopen(pgnfile, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s for reading: %s\n",
				pgnfile, strerror(errno));
		return;
	}

	PGN pgn;
	pgn.parse(fp);

	fclose(fp);
	
	Game g(pgn.get_opening(), Clock(), Clock());
	std::list<Move> moves = pgn.get_moves();
	for (std::list<Move>::const_iterator 
			it = moves.begin();
			it != moves.end();
			it++) {
		GameEntry::MoveAttributes move_attr(false, false);
		g.make_move(*it, move_attr);
	}

	printf("--- begin read game ---\n");
	g.write_pgn(stdout);
	printf("--- end read game ---\n");

	search->stop_thread();
	*game = g;
}

void Shell::cmd_savegame()
{
	CMD_REQUIRE_ARGS(1);
	const char * pgnfile  = cmd_args[1].c_str();

	FILE* fp = fopen(pgnfile, "w");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open %s for writing: %s\n",
				pgnfile, strerror(errno));
		return;
	}
	
	game->write_pgn(fp);
	
	fclose(fp);
}

void Shell::cmd_redo()
{
	search->stop_thread();
	
	Move mov;
	if (cmd_args.size() == 2 && cmd_args[1] == "all") {
		Move mov0;
		do {
			mov = mov0;
			mov0 = game->redo_move();
		} while (mov0);

		if (!mov) {
			if (!xboard) {
				printf("No move to be redone.\n");
			}
			return;
		}
	} else {
		mov = game->redo_move();
		if (!mov) {
			if (!xboard) {
				printf("No move to be redone.\n");
			}
			return;
		}
	}

	if (!xboard) {
		game->get_board().print(stdout, mov);
		if (game->get_result()) {
			print_result();
		}
	}
}
