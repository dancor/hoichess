/* $Id: pgn.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/pgn.cc
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
#include "game.h"
#include "pgn.h"
#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>


//#define DEBUG_PGN_PARSE_1
//#define DEBUG_PGN_PARSE_2


PGN::PGN()
{
}

bool PGN::parse(FILE * fp)
{
	ASSERT(fp != NULL);

	char buf[1024];
	char * strtok_r_buf;

	/* First, dischard everything up to the next tag */
	while (!feof(fp)) {
		if (fgets(buf, sizeof(buf), fp) == NULL) {
			return false;
		}

		if (buf[0] == '[') {
			break;
		}
	}

	/* Read all tags until the next empty line.
	 * One tag line is already in buf. */
	while (!feof(fp)) {
#ifdef DEBUG_PGN_PARSE_1
		printf("%s", buf);
#endif		
		char * p = buf;

		/* skip whitespace */
		while (*p && *p == ' ') {
			p++;
		}
		
		if (buf[0] != '[') {
			break;
		}

		/* skip '[' */
		p++;
		
		/* skip whitespace */
		while (*p && *p == ' ') {
			p++;
		}
		
		char * q;

		q = strtok_r(p, " \"]\n\r", &strtok_r_buf);
#ifdef DEBUG_PGN_PARSE_1
		printf("q = '%s'\n", q);
#endif
		if (!q) {
			return false;
		}
		std::string tag = q;

		q = strtok_r(NULL, "\"\n\r", &strtok_r_buf);
#ifdef DEBUG_PGN_PARSE_1
		printf("q = '%s'\n", q);
#endif
		if (!q) {
			return false;
		}
		std::string val = q;

#ifdef DEBUG_PGN_PARSE_2
		printf("tag: '%s', value '%s'\n", tag.c_str(), val.c_str());
#endif
		tags[tag] = val;

		/* Read next tag line */
		if (fgets(buf, sizeof(buf), fp) == NULL) {
			return false;
		}
	}

	if (tags["FEN"] != "") {
		if (!opening.parse_fen(tags["FEN"])) {
			return false;
		}
	} else {
		if (!opening.parse_fen(opening_fen())) {
			BUG("Failed to set up standard opening position");
		}
	}

	/*
	 * Now read the list of moves, make them on board and put them
	 * into this->moves.
	 */
	Board board = opening;
	std::string tok;
	while (!feof(fp)) {
		char * p = get_movetext_token(fp, buf, sizeof(buf));
		if (!p || !*p) {
			continue;
		}
		tok = p;
		
		if (tok[tok.length()-1] == '.') {
			/* move number */
#ifdef DEBUG_PGN_PARSE_2
			printf("move number: %s\n", tok.c_str());
#endif
		} else if (tok[0] == '{') {
			/* comment */
			std::string comment = tok;
#ifdef DEBUG_PGN_PARSE_2
			printf("comment: %s\n", comment.c_str());
#endif
		} else if (tok[0] == '$') {
			/* `numeric annotation glyph' */
#ifdef DEBUG_PGN_PARSE_2
			printf("nag: %s\n", tok.c_str());
#endif
		} else if (tok == "1-0" || tok == "0-1" || tok == "1/2-1/2"
				|| tok == "*") {
			/* result */
#ifdef DEBUG_PGN_PARSE_2
			printf("result: %s\n", tok.c_str());
#endif

			/* PGN finished, just read rest of line */
			//strm.getline(buf, sizeof(buf));
#ifdef DEBUG_PGN_PARSE_1
			printf("%s\n", buf);
#endif
			return true;
		} else {
			/* move */
#ifdef DEBUG_PGN_PARSE_2
			printf("move: %s\n", tok.c_str());
#endif
			
			Move mov = board.parse_move_1(tok);
			if (!mov) {
				if (debug) {
					/* TODO Perhaps we should print some
					 * more information here (e.g. tags) */
					printf("Invalid or illegal move in"
						" PGN: %s\n",
						tok.c_str());
				}
				return false;
			}
			moves.push_back(mov);
			board.make_move(mov);
		}
	}

	return true;
}

char * PGN::get_movetext_token(FILE * fp, char * buf, size_t bufsize)
{
	ASSERT(fp != NULL);
	char * p = buf;

	bool comment = false;
		
	while (!feof(fp) && p != buf + bufsize-1) {
		int c = fgetc(fp);
		if (c == EOF) {
			break;
		} else if (c == '\n' || c == '\r') {
			c = ' ';
		}

		if (comment) {
			*p++ = c;
			if (c == '}') {
				break;
			}
		} else {
			if (c == '{') {
				*p++ = c;
				comment = true;
			} else if (c == '.') {
				*p++ = c;
				break;
			} else if (c == ' ') {
				break;
			} else {
				*p++ = c;
			}
		}
	}
	*p = '\0';
	return buf;
}

std::list<PGN> PGN::parse_all(FILE * fp)
{
	ASSERT(fp != NULL);
	
	std::list<PGN> ret;
	while (feof(fp)) {
		PGN pgn;
		if (!pgn.parse(fp))
			break;

		ret.push_back(pgn);
	}

	return ret;
}

std::list<PGN> PGN::parse_all(const char * filename)
{
	FILE * fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "cannot open %s for reading: %s\n",
				filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return parse_all(fp);
}


/******************************************************************************
 *
 * Methods of class EPD 
 *
 *****************************************************************************/

/*
 * TODO We need some error handling if parsing failes.
 */
EPD::EPD(const std::string & _s)
{
	char * s = new char[_s.size()+1];
	strcpy(s, _s.c_str());
	if (s[strlen(s)-1] == '\n')
		s[strlen(s)-1] = '\0';
	const char * p = s;
	parse(p);	
	delete s;
}

/*
 * Return an FEN representation for this EPD. If halfmove or fullmove
 * numbers are not specified, we just use 0 resp. 1.
 */
std::string EPD::get_fen() const
{
	std::string fmvn, hmvn;
	if ((hmvn = get1("hmvn")) == "")
		hmvn = "0";
	if ((fmvn = get1("fmvn")) == "")
		fmvn = "1";
	
	std::stringstream ss;
	ss << fen_position << " " << fen_color << " " << fen_castling << " "
		<< fen_ep << " " << hmvn << " " << fmvn;
	return ss.str();
}

/*
 * Get a list of all operands for an the opcode.
 */
std::list<std::string> EPD::get(const std::string & opcode) const
{
	std::list<std::string> ret;
	
	/* ops[opcode] does not work here, because operator[] is not const. */
	std::map<std::string, std::list<std::string> >::const_iterator it
		= ops.find(opcode);
	
	if (it != ops.end())
		ret = it->second;
	
	return ret;
}

/*
 * Get only one operand. To make things simpler, use this method for
 * opcodes that normally have only one operand, like id or hmvn.
 */
std::string EPD::get1(const std::string & opcode) const
{
	std::list<std::string> tmp = get(opcode);

	if (tmp.size() == 0) {
#ifdef DEBUG
		WARN("get1(): no operand for '%s', returning empty string",
				opcode.c_str());
#endif
		return std::string("");
	} else if (tmp.size() > 1) {		
		WARN("get1(): returning only 1st operand out of %d for '%s'",
				tmp.size(), opcode.c_str());
	}

	return tmp.front();
}

/*****************************************************************************
 * EPD parsing functions. This is good code (TM).
 *****************************************************************************/

void EPD::parse(const char * p)
{
	/* First the FEN */
	enum foo { S_POSITION, S_COLOR, S_CASTLING, S_EP, S_DONE };
	int state = S_POSITION;
	while (*p) {
		if (*p == ' ') {
			state++;
			p++;
			if (state == S_DONE)
				break;
			continue;
		}
		
		switch (state) {
		case S_POSITION:
			fen_position += *p;
			break;
		case S_COLOR:
			fen_color += *p;
			break;
		case S_CASTLING:
			fen_castling += *p;
			break;
		case S_EP:
			fen_ep += *p;
			break;
		default:
			BUG("illegal state");
		}
		
		p++;
	}

	/* Now the opcodes with their operands */
	while (*p) {
		switch (*p) {
		case ' ':
			break;
		default:
			p = parse_opcode(p);
		}
		p++;
	}
}

const char * EPD::parse_opcode(const char * p)
{
	std::string opcode;
	std::list<std::string> operands;
	
	while (*p) {
		switch (*p) {
		case ';':
#ifdef DEBUG
			printf("opcode = '%s', no operands\n",
					opcode.c_str());
#endif
			ops[opcode] = operands;
			return p;
		case ' ':
#ifdef DEBUG
			printf("opcode = '%s', operands follow\n",
					opcode.c_str());
#endif
			ops[opcode] = operands;
			p++;
			while (*p) {
				p = parse_operand(p, opcode);
				if (*p != ' ')
					break;
				p++;
			}
#ifdef DEBUG
			printf("opcode '%s' finished, had %d operands\n",
					opcode.c_str(), ops[opcode].size());
#endif
			return p;			
		default:
			opcode += *p;
		}
		p++;
	}

	return p;
}

const char * EPD::parse_operand(const char * p, const std::string & opcode)
{
	std::string operand;

	while (*p) {
		switch (*p) {
		case '"':
			/* A quoted string. Put everything up to
			 * the next `"' into operand. */
			p++;
			while (*p) {
				if (*p == '"') {
					break;
				} else {
					operand += *p++;
				}
			}
			break;			
		case ';':
#ifdef DEBUG
			printf("\toperand = '%s' (last)\n", operand.c_str());
#endif
			ops[opcode].push_back(operand);
			return p;
		case ' ':
#ifdef DEBUG
			printf("\toperand = '%s'\n", operand.c_str());
#endif
			ops[opcode].push_back(operand);
			return p;
		default:
			operand += *p;
		}
		p++;
	}

	return p;
}
