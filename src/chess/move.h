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
	/* Move flags. */
	enum { MOVE_NONE	= 0x000,
		MOVE_NORMAL	= 0x001,
		MOVE_CAPTURE	= 0x002,
		MOVE_PROMOTION	= 0x004,
		MOVE_ENPASSANT	= 0x008,
		MOVE_CASTLE	= 0x010,
		MOVE_NULL	= 0x020
	};
	
      private:
	/* A bit field that describes the move's properties. The bits
	 * have the following meanings:
	 * 
	 *  0 -  5: from square
	 *  6 - 11: to square
	 * 12 - 14: type of moving piece, or piece type a pawn promotes to
	 * 15 - 17: type of captured piece, if any
	 * 18 - 31: flags
	 */
	uint32_t mov;
	     	
      public:
	FORCEINLINE Move();
	inline explicit Move(uint32_t m);
	
      private:
	inline Move(Square from, Square to, Piece ptype, Piece cap_ptype,
			uint16_t flags);

      public:
	inline Square from() const;
	inline Square to() const;
	inline Piece ptype() const;
	inline Piece promote_to() const;
	inline Piece cap_ptype() const;
	inline uint16_t flags() const;

	inline bool is_normal() const;
	inline bool is_capture() const;
	inline bool is_enpassant() const;
	inline bool is_promotion() const;
	inline bool is_castle() const;
	inline bool is_null() const;
	inline int mat_gain() const;

      public:
	inline static Move normal(Square from, Square to, Piece ptype);
	inline static Move capture(Square from, Square to, Piece ptype,
			Piece cap_ptype);
	inline static Move enpassant(Square from, Square to); 
	inline static Move promotion(Square from, Square to, Piece promote_to); 
	inline static Move promotion_capture(Square from, Square to,
			Piece promote_to, Piece cap_ptype); 
	inline static Move castle(Square from, Square to);
	inline static Move null();

      public:
	FORCEINLINE bool operator==(Move m2) const;
	FORCEINLINE bool operator!=(Move m2) const;
	FORCEINLINE bool operator<(Move m2) const;
	FORCEINLINE operator uint32_t() const;
	//inline operator bool() const;
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
	mov = 0;
}

inline Move::Move(uint32_t m)
{
	mov = m;
}

inline Move::Move(Square from, Square to, Piece ptype, Piece cap_ptype,
		uint16_t flags)
{
	mov = (from & 0x3f)
		| ((to & 0x3f) << 6)
		| ((ptype & 0x7) << 12)
		| ((cap_ptype & 0x7) << 15)
		| (flags << 18);
}

inline Square Move::from() const
{
	return (mov & 0x3f);
}

inline Square Move::to() const
{
	return ((mov >> 6) & 0x3f);
}

inline Piece Move::ptype() const
{
	if (flags() & MOVE_PROMOTION)
		return PAWN;
	return ((mov >> 12) & 0x7);
}

inline Piece Move::promote_to() const
{
	ASSERT_DEBUG(flags() & MOVE_PROMOTION);
	return ((mov >> 12) & 0x7);
}

inline Piece Move::cap_ptype() const
{
	return ((mov >> 15) & 0x7);
}

inline uint16_t Move::flags() const
{
	return ((uint16_t) (mov >> 18));
}


inline bool Move::is_normal() const
{
	return (flags() & MOVE_NORMAL);
}

inline bool Move::is_capture() const
{
	return (flags() & MOVE_CAPTURE);
}

inline bool Move::is_enpassant() const
{
	return (flags() & MOVE_ENPASSANT);
}

inline bool Move::is_promotion() const
{
	return (flags() & MOVE_PROMOTION);
}

inline bool Move::is_castle() const
{
	return (flags() & MOVE_CASTLE);
}

inline bool Move::is_null() const
{
	return (flags() & MOVE_NULL);
}

inline int Move::mat_gain() const
{
	int gain = 0;
	if (is_promotion()) {
		gain += mat_values[promote_to()] - mat_values[PAWN];
	}
	if (is_capture() || is_enpassant()) {
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
	
inline Move Move::enpassant(Square from, Square to) 
{
	return Move(from, to, PAWN, PAWN, MOVE_ENPASSANT);
}
	
inline Move Move::promotion(Square from, Square to, Piece promote_to) 
{
	return Move(from, to, promote_to, NO_PIECE, 
			MOVE_PROMOTION | MOVE_NORMAL);
}
	
inline Move Move::promotion_capture(Square from, Square to, Piece promote_to,
		Piece cap_ptype)
{
	return Move(from, to, promote_to, cap_ptype, 
			MOVE_PROMOTION | MOVE_CAPTURE);
}
	
inline Move Move::castle(Square from, Square to) 
{
	return Move(from, to, KING, NO_PIECE, MOVE_CASTLE);
}

inline Move Move::null()
{
	return Move(NO_SQUARE, NO_SQUARE, NO_PIECE, NO_PIECE, MOVE_NULL);
}


inline bool Move::operator==(Move m2) const
{
	return mov == m2.mov;
}

inline bool Move::operator!=(Move m2) const
{
	return mov != m2.mov;
}

inline bool Move::operator<(Move m2) const
{
	return mov < m2.mov;
}

inline Move::operator uint32_t() const
{
	return mov;
}

#if 0
inline Move::operator bool() const
{
	return (flags() != MOVE_NONE);
}
#endif

#endif // MOVE_H
