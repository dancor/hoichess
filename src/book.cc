/* $Id: book.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/book.cc
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

#include "common.h"
#include "book.h"
#include "pgn.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <list>
#include <vector>



/*****************************************************************************
 *
 * Member functions of class BookHeader.
 *
 *****************************************************************************/

BookHeader::BookHeader()
{
	magic = s_magic;
}

/*
 * Function to convert from book to host byte order.
 */
BookHeader BookHeader::h2b(const BookHeader& h, bool swap_byteorder)
{
	if (!swap_byteorder) {
		return h;
	} else {
		BookHeader b;
		b.size = reverse_byte_order(h.size);
		b.magic = reverse_byte_order(h.magic);
		return b;
	}
}

/*
 * Function to convert from book to host byte order.
 */
BookHeader BookHeader::b2h(const BookHeader& b, bool swap_byteorder)
{
	if (!swap_byteorder) {
		return b;
	} else {
		/* mapping is symmetric */
		return h2b(b, swap_byteorder);
	}
}


/*****************************************************************************
 *
 * Member functions of class BookEntry.
 *
 *****************************************************************************/

BookEntry::BookEntry()
{
	hashkey = NULLHASHKEY;
	for (unsigned int i=0; i<NR_MOVES; i++) {
		move[i] = NO_MOVE;
		count[i] = 0;
	}
}

/*
 * Create a BookEntry from the type of vector that is returned by
 * Book::group_moves().
 */
BookEntry::BookEntry(Hashkey _hashkey,
		std::vector<std::pair<Move, unsigned int> > moves)
{
	hashkey = _hashkey;
	for (unsigned int i=0; i<NR_MOVES; i++) {
		if (i >= moves.size())
			break;
		move[i] = moves[i].first;
		count[i] = moves[i].second;
	}
}


/*
 * Function to convert from book to host byte order.
 */
BookEntry BookEntry::h2b(const BookEntry& h, bool swap_byteorder) 
{
	if (!swap_byteorder) {
		return h;
	} else {
		BookEntry b;
		b.hashkey = reverse_byte_order(h.hashkey);
		for (unsigned int i=0; i<NR_MOVES; i++) {
#ifdef HOICHESS	// FIXME for Xiangqi
			b.move[i] = Move(reverse_byte_order(h.move[i]));
#endif
			b.count[i] = reverse_byte_order(h.count[i]);
		}
		return b;
	}
}

/*
 * Function to convert from book to host byte order.
 */
BookEntry BookEntry::b2h(const BookEntry& b, bool swap_byteorder)
{
	if (!swap_byteorder) {
		return b;
	} else {
		/* mapping is symmetric */
		return h2b(b, swap_byteorder);
	}
}



/*
 * Randomly choose one move, with the distribution given by count[].
 */
Move BookEntry::choose() const
{
	if (is_empty()) {
		WARN("BookEntry contains no move!");
		return NO_MOVE;
	}
	
	unsigned int total_count = 0;
	unsigned int i;
	
	for (i=0; i<NR_MOVES; i++) {
		if (move[i] == NO_MOVE)
			break;
		total_count += count[i];
	}

	Move * bag = new Move[total_count];
	unsigned int k = 0;
	for (i=0; i<NR_MOVES; i++) {
		if (move[i] == NO_MOVE)
			break;
		for (unsigned int j=0; j<count[i]; j++) {
			bag[k++] = move[i];
		}
	}

	k = rand() % total_count;
	Move ret = bag[k];
	delete[] bag;
	return ret;
}

/*
 * Return the number of moves in this book entry.
 */
unsigned int BookEntry::nr_moves() const
{
	unsigned int n = 0;
	for (unsigned int i=0; i<NR_MOVES; i++) {
		if (move[i] == NO_MOVE)
			break;
		n++;
	}
	return n;
}

/*
 * Check if all moves in this BookEntry are valid and legal
 * on a given board.
 */
bool BookEntry::is_valid_and_legal(const Board & board) const
{
	for (unsigned int i=0; i<BookEntry::NR_MOVES; i++) {
		Move mov = move[i];
		if (mov == NO_MOVE) {
			break;
		} else if (!mov.is_valid(board) || !mov.is_legal(board)) {
			return false;
		}
	}

	return true;
}

/* 
 * Print information about the book entry. We prepend every line with a
 * single space, as required by the "bk" command.
 */
void BookEntry::print(const Board & board) const
{
	unsigned long total_count = 0;
	for (unsigned i=0; i<NR_MOVES; i++) {
		if (move[i] == NO_MOVE)
			break;
		total_count += count[i];
	}
	
	if (total_count == 0) {
		return;
	}
	
	for (unsigned int i=0; i<BookEntry::NR_MOVES; i++) {
		if (move[i] == NO_MOVE) {
			break;
		} else {
			printf(" %-7s (%2lu%%)\n", move[i].san(board).c_str(),
					count[i] * 100 / total_count);
		}
	}
}


/*****************************************************************************
 *
 * Member functions of class Book.
 *
 *****************************************************************************/

/*
 * Open the book for reading.
 */
Book::Book(const char * filename)
{
	fp = fopen(filename, "rb");
	if (!fp) {
		std::string msg = strprintf("Cannot open %s for reading: %s\n",
			filename, strerror(errno));
		throw BookException(msg);
	}

	swap_byteorder = false;
	read_header();
	if (header.magic != BookHeader::s_magic) {
		swap_byteorder = true;
		read_header();
		if (header.magic != BookHeader::s_magic) {
			fclose(fp);
			std::string msg = strprintf(
				"%s seems to be no HoiChess opening book"
				" (magic = 0x%0lx, should be 0x%0lx)\n",
				filename, 	
				(unsigned long) header.magic,
				(unsigned long) BookHeader::s_magic);
			msg += strprintf(
				"Maybe you are trying to use an opening book"
				" for HoiChess with HoiXiangqi or"
				" vice versa.\n");
			throw BookException(msg);
		}
	}
}

/*
 * Create a new, empty book with `size' slots.
 * The book will be opened r/w, so put() can be used to fill it with life.
 */
Book::Book(const char * filename, unsigned long size)
{
	fp = fopen(filename, "w+b");
	if (!fp) {
		fprintf(stderr, "Cannot open %s for writing: %s\n",
			filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	swap_byteorder = false;
	header.size = size;
	write_header();
	
	/* Wipe out all slots */
	BookEntry entry;
	for (unsigned long i=0; i<size; i++) {
		write_entry(i, entry);
	}
}

Book::~Book()
{
	fclose(fp);
}

/*
 * Search book for position. If it was found, its BookEntry will be stored
 * in `entry' and the function will return true.
 */
bool Book::lookup(const Board & board, BookEntry * entry) const
{
	Hashkey hashkey = board.get_hashkey();
	unsigned long slot = 0;
	for (unsigned int i=0; i<header.size; i++) {
		slot = hashfunc(hashkey, i);

		*entry = read_entry(slot);
		if (entry->is_empty()) {
			/* Slot is totally empty */
			return false;
		} else if (entry->hashkey != hashkey) {
			/* Collision */
			continue;
		} else if (!entry->is_valid_and_legal(board)) {
			WARN("invalid or illegal move in book, perhaps an"
					"undetected hash collision");
			return false;
		} else {
			return true;
		}
	}

	return false;
}

bool Book::put(const BookEntry & newentry)
{
	unsigned long slot = 0;
	for (unsigned int i=0; i<header.size; i++) {
		slot = hashfunc(newentry.hashkey, i);

		BookEntry oldentry = read_entry(slot);
		if (oldentry.is_empty()) {
			/* Slot was empty */
#ifdef DEBUG
			if (i > 5) {
				printf("(%d)", i); fflush(stdout);
			}
#endif
			break;
		} else if (oldentry.hashkey != newentry.hashkey) {
			/* Collision */
			if (i == header.size-1) {
#ifdef DEBUG
				printf("X"); fflush(stdout);
#endif
				return false;
			}
			continue;
		} else {
			/* Overwrite */
#ifdef DEBUG
			printf("O"); fflush(stdout);
#endif
			break;
		}	

	}
	
	write_entry(slot, newentry);
	return true;
}

/*
 * This is a simple multi hash function. It works rather well in practice.
 */
unsigned long Book::hashfunc(Hashkey hashkey, unsigned int i) const
{
	return (hashkey % header.size + i * (hashkey % (header.size-1)))
		% header.size;
}

/*
 * Read a PGN game database and create a new opening book from the first
 * `depth' moves of each game.
 */
void Book::create_from_pgn(const char * bookfile, const char * pgnfile,
		unsigned int depth, unsigned int min_move_count)
{
	FILE * fp = fopen(pgnfile, "r");
	if (!fp) {
		fprintf(stderr, "Cannot open %s for reading: %s\n",
			pgnfile, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	
	/*
	 * Read all games from PGN file. Create a map with a position's
	 * hash key and a list of moves played in this position.
	 *
	 * TODO This is very slow.
	 */
	std::map<Hashkey, std::list<Move> > pgnmap;
	unsigned long read = 0, skipped = 0;

	while (!feof(fp)) {
		PGN pgn;
		if (pgn.parse(fp)) {
			Board board = pgn.get_opening();
			std::list<Move> moves = pgn.get_moves();
			unsigned int i = 0;
			for (std::list<Move>::iterator it = moves.begin();
					it != moves.end();
					it++) {
				ASSERT(it->is_valid(board));
				ASSERT(it->is_legal(board));
				pgnmap[board.get_hashkey()].push_back(*it);
				
				i++;
				if (i > depth && depth > 0)
					break;
				
				board.make_move(*it);
			}

			read++;
		} else {
			skipped++;
		}

		if ((read+skipped) % 231 == 0) {
			printf("Reading games: %lu games read, "
					"%lu games skipped due to errors\r",
					read, skipped);
			fflush(stdout);
		}
	}

	fclose(fp);
	
	printf("Reading games: %lu games read, "
			"%lu games skipped due to errors\n",
			read, skipped);

	printf("Total number of different positions in games: %lu\n",
			(unsigned long) pgnmap.size());


	/*
	 * Create a BookEntry for each position stored in
	 * the map, and store all book entries in a list.
	 */

	/* Statistics about average number of moves per BookEntry. */
	unsigned long stat_mpe_sum = 0, stat_mpe_cnt = 0;
	unsigned long nr_entries = 0;
	unsigned long nr_entries_total = pgnmap.size();
	std::list<BookEntry> entries;
	for (std::map<Hashkey, std::list<Move> >::iterator it = pgnmap.begin();
			it != pgnmap.end();
			it++) {
		BookEntry entry(it->first, group_moves(it->second,
					min_move_count));
		if (!entry.is_empty()) {
			entries.push_back(entry);
			stat_mpe_sum += entry.nr_moves();
			stat_mpe_cnt++;
		}
		nr_entries++;

		if (nr_entries % 500 == 0 || nr_entries == nr_entries_total) {
			printf("Sorting and filtering book contents: %lu%%\r",
					nr_entries * 100 / nr_entries_total);
			fflush(stdout);
		}
	}
	printf("\n");
	
	float stat_mpe_avg = (stat_mpe_cnt != 0) ?
		((float) stat_mpe_sum / stat_mpe_cnt) : ((float) 0);
	printf("Average number of moves per position: %.2f\n", stat_mpe_avg);

	
	/*
	 * Write book to file.
	 */
	
	unsigned long booksize = entries.size();
	printf("Opening book will contain %lu positions.\n", booksize);

	/* Add some extra space to reduce hash collisions */
	unsigned long ext_booksize = (unsigned long) (booksize * 1.1);
	
	printf("Creating opening book with %lu entries.\n", ext_booksize);
	Book book(bookfile, ext_booksize);
	
	unsigned long written = 0, collisions = 0;
	for (std::list<BookEntry>::iterator it = entries.begin();
			it != entries.end();
			it++) {
		if (book.put(*it)) {
			written++;
		} else {
			collisions++;
		}
		
		unsigned long total = written + collisions;
		if (total % 500 == 0 || total == booksize) {
			printf("Writing book to file: %lu%%\r",
					total * 100 / booksize);
			fflush(stdout);
		}
	}
	printf("\n");
	
	printf("%lu entries written, %lu irresolvable collisions\n",
			written, collisions);
}
		

/*
 * Helper class needed by group_moves() to define a
 * strict weak ordering of std::pair<Move, int>.
 */
class cannot_imagine_a_name_for_this_class {
      public:
	inline bool operator()(const std::pair<Move, unsigned int> & a,
			const std::pair<Move, unsigned int> & b) const
	{
		return a.second > b.second;
	}
};

/*
 * Take a list of moves and create a vector of all distinct moves together
 * with their total number of occurrencies in the list. The returned vector
 * will be sorted descendingly by the number of occurrencies. However, only
 * moves that appeared at least min_move_count times are kept.
 * We do this because we want to have only the most frequently played moves
 * in the opening book.
 */
std::vector<std::pair<Move, unsigned int> > Book::group_moves(
		std::list<Move> moves,
		unsigned int min_move_count)
{
	/* Count total number of occurrencies of each move. */
	std::map<Move, unsigned int, Move::strict_weak_ordering> count;
	for (std::list<Move>::iterator it = moves.begin(); 
			it != moves.end();
			it++) {
		count[*it]++;
	}
	
	/* Put entries of map into vector. */
	std::vector<std::pair<Move, unsigned int> > ret;
	for (std::map<Move, unsigned int, Move::strict_weak_ordering>::iterator
							it2 = count.begin();
			it2 != count.end();
			it2++) {
		Move mov = it2->first;
		unsigned int cnt = it2->second;

		/* Keep only frequently played moves. */
		if (cnt >= min_move_count) {
			ret.push_back(std::pair<Move, unsigned int>(mov, cnt));
		}
	}

	cannot_imagine_a_name_for_this_class lt;
	std::sort(ret.begin(), ret.end(), lt);

	return ret;
}


/*****************************************************************************
 * Low-level book access funtions.
 *****************************************************************************/

void Book::read_header()
{
	if (fseek(fp, 0, SEEK_SET) == -1) {
		perror("Book::read_header(): fseek() failed");
		exit(EXIT_FAILURE);
	}

	BookHeader tmp_header;
	if (fread(&tmp_header, sizeof(BookHeader), 1, fp) != 1) {
		perror("Book::read_header(): fread() failed");
		exit(EXIT_FAILURE);
	}
	header = BookHeader::b2h(tmp_header, swap_byteorder);
}

void Book::write_header()
{
	if (fseek(fp, 0, SEEK_SET) == -1) {
		perror("Book::write_header(): fseek() failed");
		exit(EXIT_FAILURE);
	}
	
	BookHeader tmp_header = BookHeader::h2b(header, swap_byteorder);
	if (fwrite(&tmp_header, sizeof(BookHeader), 1, fp) != 1) {
		perror("Book::read_header(): fwrite() failed");
		exit(EXIT_FAILURE);
	}
}

BookEntry Book::read_entry(unsigned long slot) const
{
	if (slot >= header.size) {
		BUG("Slot is beyond end of book: slot = %d, size = %d",
				slot, header.size);
	}
	
	unsigned long pos = sizeof(BookHeader) + slot * sizeof(BookEntry);
	if (fseek(fp, pos, SEEK_SET) == -1) {
		perror("Book::read_entry(): fseek() failed");
		exit(EXIT_FAILURE);
	}
	
	BookEntry tmp_entry;
	if (fread(&tmp_entry, sizeof(BookEntry), 1, fp) != 1) {
		perror("Book::read_entry(): fread() failed");
		exit(EXIT_FAILURE);
	}
	return BookEntry::b2h(tmp_entry, swap_byteorder);
}

void Book::write_entry(unsigned long slot, const BookEntry & entry)
{
	if (slot >= header.size) {
		BUG("Slot is beyond end of book: slot = %d, size = %d",
				slot, header.size);
	}
	
	unsigned long pos = sizeof(BookHeader) + slot * sizeof(BookEntry);
	if (fseek(fp, pos, SEEK_SET) == -1) {
		perror("Book::write_entry(), fseek failed");
		exit(EXIT_FAILURE);
	}

	BookEntry tmp_entry = BookEntry::h2b(entry, swap_byteorder);
	if (fwrite(&tmp_entry, sizeof(BookEntry), 1, fp) != 1) {
		perror("Book::write_entry(): fwrite() failed");
		exit(EXIT_FAILURE);
	}
}

