/* $Id: main.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/main.cc
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

#include <errno.h>
#include <getopt.h>
#ifndef WIN32
# include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compile.h"
#include "shell.h"
#include "util.h"


/*
 * Global variables, declared in common.h.
 */
unsigned int debug = 0;
unsigned int verbose = 0;
bool ansicolor = false;



static Shell * shell;

#ifndef WIN32
static void sigint_handler(int sig)
{
	(void) sig;
	shell->interrupt();
}
#endif

extern void init();
extern void fini();

static void print_version()
{
	printf("%s %s (%s)\n", PROGNAME, VERSION, COMPILE_DATE);
}

static void print_copyright()
{
	printf(
"Copyright (C) 2004-2007 %s %s\n"
"This program is free software and comes with ABSOLUTELY NO WARRANTY.\n"
"See the GNU General Public License for more details.\n",
			AUTHOR, AUTHOR_EMAIL);
}

static void print_info()
{
	printf("Platform: %s\n", PLATFORM);
	printf("Compiler: %s %s\n", COMPILER, COMPILER_VERSION);
}

static void usage(const char * argv0)
{
	print_version();
	print_copyright();
	
	printf("\nUsage: %s [options]\n\n", argv0);
	printf("Options:\n");
	printf("  -h | --help           Display usage information\n");
	printf("  -V | --version        Display version information\n");
	printf("  -v | --verbose[=N]    Increase verbosity\n");
	printf("  -d | --debug[=N]      Increase debug level\n");
	printf("  -L | --logfile FILE   Specify log file name (log will be appended)\n");
	printf("  -x | --xboard         Start in xboard mode\n");
	printf("       --book FILE      Specify file name of opening book (default: %s)\n", DEFAULT_BOOK);
	printf("       --nobook         Disable opening book\n");
	printf("       --hashsize SIZE  Set size of main hash table (default: %s)\n", DEFAULT_HASHSIZE);
	printf("       --pawnhashsize SIZE  Set size of pawn hash table (default: %s)\n", DEFAULT_PAWNHASHSIZE);
#ifdef USE_EVALCACHE
	printf("       --evalcache SIZE	Set size of evaluation cache (default: %s)\n", DEFAULT_EVALCACHESIZE);
#endif
	printf("       --rcfile FILE    Read initial commands from FILE\n");

	printf(
"\nThese are only the most general options. See documentation for a complete\n"
"list of supported command line options, and a detailed description of them.\n"
"\nSee http://www.hoicher.de/hoichess for more information and new releases.\n"
"Please report any bugs and suggestions to %s.\n",
			AUTHOR_EMAIL);
}

int main(int argc, char ** argv)
{
	const char * opt_logfile = NULL;
	const char * opt_xboard = "auto";
	const char * opt_hashsize = DEFAULT_HASHSIZE;
	const char * opt_bookfile = DEFAULT_BOOK;
	const char * opt_name = NULL;
	const char * opt_rcfile = NULL;
	const char * opt_color = "auto";
	const char * opt_evalcache = DEFAULT_EVALCACHESIZE;
	const char * opt_pawnhashsize = DEFAULT_PAWNHASHSIZE;

	/* Most Unix platforms have color terminals. But on Win32 systems,
	 * ANSI color is normally not available. */
#ifdef WIN32
	opt_color = "off";
#endif
	

	bool xboard_mode = false;
	
	/*
	 * Parse command line.
	 */
	
	const char * short_opts = "hVvdL:x::";
	struct option long_options[] = {
		{ "help", 0, 0, 'h' },
		{ "version", 0, 0, 'V' },
		{ "verbose", 2, 0, 'v' },
		{ "debug", 2, 0, 'd' },
		{ "logfile", 1, 0, 'L' },
		{ "xboard", 2, 0, 'x' },
		{ "hashsize", 1, 0, 128 },
		{ "book", 1, 0, 129 },
		{ "nobook", 0, 0, 130 },
		{ "name", 1, 0, 131 },
		{ "rcfile", 1, 0, 132 },
		{ "script", 1, 0, 240 },		// deprecated
		{ "color", 1, 0, 133 },
		{ "evalcache", 1, 0, 134 },
		{ "pawnhashsize", 1, 0, 135 },
		
		{ 0, 0, 0, 0 }
	};

	int c;
	while ((c = getopt_long(argc, argv, short_opts, long_options,
					NULL)) != -1) {
		switch (c) {
		case 'h': /* --help */
			usage(argv[0]);
			exit(0);
		case 'V': /* --version */
			print_version();
			exit(0);
		case 'v': /* --verbose */
			if (optarg) {
				verbose = atoi(optarg);
			} else {
				verbose++;
			}
			break;
		case 'd': /* --debug */
			if (optarg) {
				debug = atoi(optarg);
			} else {
				debug++;
			}
			break;
		case 'L': /* --logfile */
			opt_logfile = optarg;
			break;
		case 'x': /* --xboard */
			if (!optarg) {
				xboard_mode = true;
			}
			opt_xboard = optarg;
			break;
		case 128: /* --hashsize */
			opt_hashsize = optarg;
			break;
		case 129: /* --book */
			opt_bookfile = optarg;
			break;
		case 130: /* --nobook */
			opt_bookfile = NULL;
			break;
		case 131: /* --name */
			opt_name = optarg;
			break;
		case 132: /* --rcfile */
			opt_rcfile = optarg;
			break;
		case 240: /* --script (deprecated) */
			opt_rcfile = optarg;
			printf("Warning: option --script is deprecated."
					" Use --rcfile.\n");
			break;
		case 133: /* --color */
			opt_color = optarg;
			break;
		case 134: /* --evalcache */
			opt_evalcache = optarg;
			break;
		case 135: /* --pawnhashsize */
			opt_pawnhashsize = optarg;
			break;
			
		case '?':
			usage(argv[0]);
			exit(1);
		default:
			BUG("getopt_long() returned %d", c);
		}
	}

	
	print_version();
	print_copyright();
	printf("\n");
	print_info();
	printf("\n");

	if (debug) {
		debug_print_compiletime_config();
		debug_print_storagesizes();
		printf("\n");
	}

	if (verbose || debug) {
		printf("Verbosity set to %u.\n", verbose);
		printf("Debug level set to %u.\n", debug);
		printf("\n");
	}
	
	/*
	 * Do things depending on command line options.
	 */

	if (opt_logfile) {
		printf("Logging to %s\n", opt_logfile);
		open_log(opt_logfile);
	}

	if (opt_xboard) {
		if (		   strcmp(opt_xboard, "true") == 0
				|| strcmp(opt_xboard, "on") == 0
				|| strcmp(opt_xboard, "yes") == 0) {
			xboard_mode = true;
		} else if (	   strcmp(opt_xboard, "false") == 0
				|| strcmp(opt_xboard, "off") == 0
				|| strcmp(opt_xboard, "no") == 0) {
			xboard_mode = false;
		} else if (strcmp(opt_xboard, "auto") == 0) {
			if (isatty(1)) {
				xboard_mode = false;
			} else {
				xboard_mode = true;
				printf("stdout is not a terminal, enabling"
					" xboard mode.\n");
			}
		} else {
			fprintf(stderr, 
				"Illegal argument to option --xboard: %s\n",
				opt_xboard);
			exit(EXIT_FAILURE);
		}
	}

	if (opt_color) {
		if (		   strcmp(opt_color, "true") == 0
				|| strcmp(opt_color, "on") == 0
				|| strcmp(opt_color, "yes") == 0) {
			ansicolor = true;
		} else if (	   strcmp(opt_color, "false") == 0
				|| strcmp(opt_color, "off") == 0
				|| strcmp(opt_color, "no") == 0) {
			ansicolor = false;
		} else if (strcmp(opt_color, "auto") == 0) {
			if (isatty(1)) {
				ansicolor = true;
			} else {
				ansicolor = false;
			}
		} else {
			fprintf(stderr, 
				"Illegal argument to option --color: %s\n",
				opt_color);
			exit(EXIT_FAILURE);
		}

	}

	/*
	 * Initialize everything and start shell.
	 */

	init();
	shell = new Shell();
	shell->set_myname(opt_name);
	shell->set_xboard(xboard_mode);

	/* opening book */
	shell->set_book(opt_bookfile);
	
	/* main hash size */
	if (opt_hashsize) {
		long size = 0;
		if (!parse_size(opt_hashsize, &size) || size < 0) {
			printf("Error: Illegal value for hash table size: %s\n",
					opt_hashsize);
			exit(EXIT_FAILURE);
		}

		shell->set_hashsize(size);
	}
	
	/* pawh hash size */
	if (opt_pawnhashsize) {
		long size = 0;
		if (!parse_size(opt_pawnhashsize, &size) || size < 0) {
			printf("Error: Illegal value for pawn hash size: %s\n",
					opt_pawnhashsize);
			exit(EXIT_FAILURE);
		}

		shell->set_pawnhashsize(size);
	}

#ifdef USE_EVALCACHE
	/* evalation cache size */
	if (opt_evalcache) {
		long size = 0;
		if (!parse_size(opt_evalcache, &size) || size < 0) {
			printf("Error: Illegal value for evaluation"
					" cache size: %s\n",
					opt_evalcache);
			exit(EXIT_FAILURE);
		}

		shell->set_evalcachesize(size);
	}
#endif

	/* read script file if given on command line */
	if (opt_rcfile) {
		FILE * fp = fopen(opt_rcfile, "r");
		if (fp == NULL) {
			fprintf(stderr, "Cannot open %s for reading: %s\n",
					opt_rcfile, strerror(errno));
			exit(EXIT_FAILURE);
		}
		shell->source_file(fp, opt_rcfile);
	}
	
	/* read hoichess.rc; note that commands here are executed _before_
	 * commands in --script file, because the last opened source is
	 * read at as first... */
#if defined(HOICHESS)
	const char * rcfile = "hoichess.rc";
#elif defined(HOIXIANGQI)
	const char * rcfile = "hoixiangqi.rc";
#endif
	FILE * fp = fopen(rcfile, "r");
	if (fp != NULL) {
		shell->source_file(fp, rcfile);
	}

#ifndef WIN32
	signal(SIGINT, sigint_handler);	
#endif 
	shell->main();

	/*
	 * Clean up.
	 */

	delete shell;

	close_log();
	
	fini();
	
	return 0;
}

