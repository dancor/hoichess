/* $Id: book.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/book.h
 *
 * Copyright (C) 2005-2006 Holger Ruckdeschel <holger@hoicher.de>
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
#ifndef BOOK_H
#define BOOK_H

#include "common.h"
#include "board.h"
#include "hash.h"
#include "move.h"

#include <stdio.h> 

#include <list>
#include <vector>
#include <stdexcept>


class BookEntry {
	friend class Book;
	static const unsigned int NR_MOVES = 4;

      private:
	/* Do not change the order or the type of those members. They are
	 * written to the book in binary format. */
	Hashkey hashkey;		/* is uint64_t */
	Move move[NR_MOVES];		/* is uint32_t */
	uint32_t count[NR_MOVES];

      public:
	BookEntry();
	BookEntry(Hashkey _hashkey,
			std::vector<std::pair<Move, unsigned int> > moves);
	
	/* Functions to convert between host and book byte order. */
	static BookEntry h2b(const BookEntry& h, bool swap_byteorder);
	static BookEntry b2h(const BookEntry& b, bool swap_byteorder);
	
      public:
	Move choose() const;
	inline bool is_empty() const;
	unsigned int nr_moves() const;
	bool is_valid_and_legal(const Board & board) const;
	void print(const Board & board) const;	
};

inline bool BookEntry::is_empty() const
{
	return move[0] == NO_MOVE;
}


class BookHeader {
	friend class Book;

      private:
	/* Do not change the order or the type of those members. They are
	 * written to the book in binary format. */
	uint32_t size;
	uint32_t magic;

#if defined(HOICHESS)
	static const uint32_t s_magic = 0xdaabaffeL;
#elif defined(HOIXIANGQI)
	static const uint32_t s_magic = 0x6a8dda83L;
#else
# error "neither HOICHESS nor HOIXIANGQI defined"
#endif

      public:
	BookHeader();
	
	/* Functions to convert between host and book byte order. */
	static BookHeader h2b(const BookHeader& h, bool swap_byteorder);
	static BookHeader b2h(const BookHeader& b, bool swap_byteorder);
};

class BookException {
	std::string msg; 

      public:
	BookException(const std::string& msg) : msg(msg)
	{}

      public:
	const std::string& get_msg() {
		return msg;
	}
};




class Book {
      private:
	FILE * fp;
	bool swap_byteorder;
	BookHeader header;
	
      public:
	Book(const char * filename);
	Book(const char * filename, unsigned long size);
	~Book();

      public:
	bool lookup(const Board & board, BookEntry * entry) const;
	bool put(const BookEntry & entry);

	static void create_from_pgn(const char * bookfile,
			const char * pgnfile,
			unsigned int depth,
			unsigned int min_move_count);

      private:
	unsigned long hashfunc(Hashkey hashkey, unsigned int i) const;
	static std::vector<std::pair<Move, unsigned int> > group_moves(
			std::list<Move> moves, unsigned int min_move_count);
	void read_header();
	void write_header();
	BookEntry read_entry(unsigned long slot) const;
	void write_entry(unsigned long slot, const BookEntry & entry);
};


#endif // BOOK_H
