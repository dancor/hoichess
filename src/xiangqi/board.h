/* $Id: board.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/board.h
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
#ifndef BOARD_H
#define BOARD_H

#include "common.h"
#include "hash.h"
#include "move.h"
#include "movelist.h"
#include "basic.h"

#include <string>


#ifdef USE_UNMAKE_MOVE
/* Forward declaration */
class BoardHistory;
#endif

class Board
{
	friend class Evaluator;

	/* Data Members */
      private:
	Color		side, opponent;
	int		moveno;
	int 		movecnt50;

	Piece		position_pieces[90];
	Color		position_colors[90];
	Square 		king[2];
	
//	unsigned int 	flags;

	int 		material[2];

	unsigned int	pce_movecnt[90];

	Hashkey 	hashkey;
	Hashkey 	pawnhashkey;

	
	
	/* Constructors / Destructor, defined in board.cc */
      public:
	Board();
	Board(const std::string& fen);
	inline ~Board() {};
	
	/* Accessors */
      public:
	Color get_side() const
	{ return side; }

	int get_moveno() const
	{ return moveno; }

	int get_movecnt50() const
	{ return movecnt50; }
    
#if 0
	unsigned int get_flags() const
	{ return flags; }
#endif

	Hashkey get_hashkey() const
	{ return hashkey; }
	
	Hashkey get_pawnhashkey() const
	{ return pawnhashkey; }

	inline Hashkey get_hashkey_noside() const;
	inline unsigned int get_pce_movecnt(Square sq) const;


      private:
	Square get_king(Color side) const
	{ return king[side]; }
	

	/* Basic board functions, defined in board.cc */
      public:
	void clear();
#ifdef USE_UNMAKE_MOVE
	BoardHistory make_move(Move mov);
	void unmake_move(const BoardHistory & hist);
#else
	void make_move(Move mov);
#endif
	bool is_valid_move(Move mov) const;
	bool is_legal_move(Move mov) const;
	inline Color color_at(Square sq) const;
	inline Piece piece_at(Square sq) const;
      private:
	void set_side(Color side);
	void switch_sides();
	void place_piece(Square sq, Color side, Piece ptype);
	void remove_piece(Square sq, Color side, Piece ptype);
	void move_piece(Square from, Square to, Color side, Piece ptype);
//	void set_flag(unsigned int flag);
//	void clear_flag(unsigned int flag);
      public:
	inline bool in_check() const;
	inline bool is_legal() const;
	inline int material_difference() const;
	inline int get_material(Color side) const;
	
	/* Attack functions, defined in board_attack.cc */
      private:
	unsigned int pawn_attacks(Square from, Color side, Square tos[])
		const;
	unsigned int guard_attacks(Square from, Color side, Square tos[])
		const;
	unsigned int elephant_attacks(Square from, Color side, Square tos[])
		const;
	unsigned int knight_attacks(Square from, Color side, Square tos[])
		const;
	unsigned int cannon_attacks(Square from, Color side, Square tos[])
		const;
	unsigned int rook_attacks(Square from, Color side, Square tos[]) 
		const;
	unsigned int king_attacks(Square from, Color side, Square tos[])
		const;
      public:
	bool is_attacked(Square to, Color atkside) const;
	bool kings_facing() const;
	
	/* Move generation functions, defined in board_generate.cc */
      public:
	void generate_moves(Movelist * movelist) const;
	void generate_captures(Movelist * movelist) const;
	void generate_noncaptures(Movelist * movelist) const;
	void generate_escapes(Movelist * movelist) const;

	/*
	 * For compatibility with chess/board.
	 */
	
	inline void generate_moves(Movelist * movelist, bool allpromo) const
	{
		(void) allpromo;
		generate_moves(movelist);
	}

	inline void generate_captures(Movelist * movelist, bool allpromo) const
	{
		(void) allpromo;
		generate_captures(movelist);
	}
	
      private:

	/* Utility functions, defined in board_util.cc */
      public:
	bool is_mate() const;
	bool is_stalemate() const;
	bool is_valid() const;
	bool is_material_draw() const;
	void print(FILE * fp = stdout, Move last_move = Move()) const;
	void print_small(FILE * fp = stdout) const;
	std::string get_fen() const;
	bool parse_fen(const char * s);
	bool parse_fen(const std::string & str);
	Move parse_move(const std::string & str) const;
	Move parse_move_1(const std::string & str) const;
	Move do_parse_move_1(const std::string & str) const;
	bool operator==(const Board & board) const;     

	
	/* Static Data Members */
      private:
	static Hashkey hashkeys[2][7][90];
	static Hashkey hash_side;

	/* Static Member Functions */
      public:
	static void init();
};

#ifdef USE_UNMAKE_MOVE
/* 
 * This class contains all necessary information to unmake a previous move.
 */
class BoardHistory
{
	friend class Board;
      private:
#ifdef DEBUG
	/* This is used by unmake_move() to verify that the board status
	 * was correctly restored. */
	Board oldboard;
#endif
	Move move;
	int movecnt50;
//	unsigned int flags;
	unsigned int pce_movecnt_to;
};
#endif


/*****************************************************************************
 *
 * Inline functions of class board
 *
 *****************************************************************************/

inline Color Board::color_at(Square sq) const
{
	ASSERT_DEBUG(sq >= A0 && sq <= I9);
	return position_colors[sq];
}

inline Piece Board::piece_at(Square sq) const
{
	ASSERT_DEBUG(sq >= A0 && sq <= I9);
	return position_pieces[sq];
}

inline Hashkey Board::get_hashkey_noside() const
{
	if (side == BLACK) {
		return hashkey ^ hash_side;
	} else {
		return hashkey;
	}
}				     
	
inline unsigned int Board::get_pce_movecnt(Square sq) const
{
	ASSERT_DEBUG(sq >= A0 && sq <= I9);
	return pce_movecnt[sq];
}

inline bool Board::in_check() const
{
	return kings_facing() || is_attacked(get_king(side), opponent);
}

/*
 * Check if the position is legal, i.e. the enemy king
 * cannot be captured with the next move.
 */
inline bool Board::is_legal() const
{
	return !kings_facing() && !is_attacked(get_king(opponent), side);
}

inline int Board::material_difference() const
{
	return (material[side] - material[opponent]);
}

inline int Board::get_material(Color side) const
{
	ASSERT_DEBUG(side != NO_COLOR);
	return material[side];
}

#endif // BOARD_H
