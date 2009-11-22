/* $Id: move.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/move.h
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
#ifndef MOVE_H
#define MOVE_H

#include "common.h"
#include "board.h"
#include "basic.h"

#include <string>

/* Need a forward declaration here since
 * move.h and board.h include each other. */
class Board;

#define NO_MOVE		Move()

class Move {
      public:
	enum { MOVE_NONE = 0x0000,
		MOVE_NORMAL = 0x0001, MOVE_CAPTURE = 0x0002,
		MOVE_NULL = 0x0020 };
	
      private:
	Square mov_from;
	Square mov_to;
	Piece mov_ptype;
	Piece mov_cap_ptype;
	uint16_t mov_flags;

	     	
      public:
	FORCEINLINE Move();
	
      private:
	inline Move(Square from, Square to, Piece ptype, Piece cap_ptype,
			uint16_t flags);

      public:
	inline Square from() const;
	inline Square to() const;
	inline Piece ptype() const;
	inline Piece cap_ptype() const;
	inline uint16_t flags() const;

	inline bool is_normal() const;
	inline bool is_capture() const;
	inline bool is_null() const;
	inline int mat_gain() const;

      public:
	inline static Move normal(Square from, Square to, Piece ptype);
	inline static Move capture(Square from, Square to, Piece ptype,
			Piece cap_ptype);
	inline static Move null();

      public:
	FORCEINLINE bool operator==(const Move& m2) const;
	FORCEINLINE bool operator!=(const Move& m2) const;
	inline bool operator<(const Move& m2) const;
	inline operator bool() const;
	bool is_valid(const Board & board) const;
	bool is_legal(const Board & board) const;

	std::string str() const;
	std::string san(const Board & board, int nonstd = 0) const;
	void print() const;
	
	/* Define a strict weak ordering of Move objects. This is useful
	 * when we want to use a std::map<Move, ...>, like we do in
	 * Book::group_moves(). */
	class strict_weak_ordering {
	      public:
		bool operator()(Move a, Move b) const {
			return a < b;
		}
	};

	/* Hmm, Borland compiler does not grant access to Move::mov. */
	friend class strict_weak_ordering;
};

inline Move::Move()
{
	mov_from = NO_SQUARE;
	mov_to = NO_SQUARE;
	mov_ptype = NO_PIECE;
	mov_cap_ptype = NO_PIECE;
	mov_flags = 0; 
}

inline Move::Move(Square from, Square to, Piece ptype, Piece cap_ptype,
		uint16_t flags)
{
	mov_from = from;
	mov_to = to;
	mov_ptype = ptype;
	mov_cap_ptype = cap_ptype;
	mov_flags = flags; 
}

inline Square Move::from() const
{
	return mov_from;
}

inline Square Move::to() const
{
	return mov_to;
}

inline Piece Move::ptype() const
{
	return mov_ptype;
}

inline Piece Move::cap_ptype() const
{
	return mov_cap_ptype;
}

inline uint16_t Move::flags() const
{
	return mov_flags;
}


inline bool Move::is_normal() const
{
	return (flags() & MOVE_NORMAL);
}

inline bool Move::is_capture() const
{
	return (flags() & MOVE_CAPTURE);
}

inline bool Move::is_null() const
{
	return (flags() & MOVE_NULL);
}

inline int Move::mat_gain() const
{
	int gain = 0;
	if (is_capture()) {
		gain += mat_values[cap_ptype()];
	}
	return gain;
}


inline Move Move::normal(Square from, Square to, Piece ptype)
{
	return Move(from, to, ptype, NO_PIECE, MOVE_NORMAL);
}
			
inline Move Move::capture(Square from, Square to, Piece ptype, Piece cap_ptype) 
{
	return Move(from, to, ptype, cap_ptype, MOVE_CAPTURE);
}
	
inline Move Move::null()
{
	return Move(NO_SQUARE, NO_SQUARE, NO_PIECE, NO_PIECE, MOVE_NULL);
}


inline bool Move::operator==(const Move& m2) const
{
	return (this->mov_from == m2.mov_from)
		&& (this->mov_to == m2.mov_to)
		&& (this->mov_ptype == m2.mov_ptype)
		&& (this->mov_cap_ptype == m2.mov_cap_ptype)
		&& (this->mov_flags == m2.mov_flags);
}

inline bool Move::operator!=(const Move& m2) const
{
	return !operator==(m2);
}

inline bool Move::operator<(const Move& m2) const
{
	if (this->mov_from < m2.mov_from) {
		return true;
	} else if (this->mov_from > m2.mov_from) {
		return false;
	} else if (this->mov_to < m2.mov_to) {
		return true;
	} else if (this->mov_to > m2.mov_to) {
		return false;
	} else if (this->mov_ptype < m2.mov_ptype) {
		return true;
	} else if (this->mov_ptype > m2.mov_ptype) {
		return false;
	} else if (this->mov_cap_ptype < m2.mov_cap_ptype) {
		return true;
	} else if (this->mov_cap_ptype > m2.mov_cap_ptype) {
		return false;
	} else if (this->mov_flags < m2.mov_flags) {
		return true;
	} else if (this->mov_flags > m2.mov_flags) {
		return false;
	} else {
		return false;
	}
}

inline Move::operator bool() const
{
	return (flags() != MOVE_NONE);
}

#endif // MOVE_H
