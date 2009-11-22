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
#include "bitboard.h"
#include "hash.h"
#include "move.h"
#include "movelist.h"
#include "basic.h"

#include <string>


/* Castling flags */
#define WKCASTLE	0x01
#define WQCASTLE	0x02
#define BKCASTLE	0x04
#define BQCASTLE	0x08
#define WCASTLE		(WKCASTLE | WQCASTLE)
#define BCASTLE		(BKCASTLE | BQCASTLE)

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
		
	Bitboard	position[2][6];
	Bitboard	position_all[2];
	Bitboard 	occupied;
	Bitboard 	occupied_l90;
	Bitboard 	occupied_l45;
	Bitboard 	occupied_r45;

	Square 		king[2];
	
	unsigned int 	flags;
	Square 		epsq;

	int 		material[2];
	bool 		has_castled[2];

	unsigned int	pce_movecnt[64];

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
    
	unsigned int get_flags() const
	{ return flags; }

	Square get_epsq() const
	{ return epsq; }
	
	Hashkey get_hashkey() const
	{ return hashkey; }
	
	Hashkey get_pawnhashkey() const
	{ return pawnhashkey; }

	inline Hashkey get_hashkey_noside() const;
	inline unsigned int get_pce_movecnt(Square sq) const;


      private:
	Bitboard get_pawns(Color side) const
	{ return position[side][PAWN]; }
	
	Bitboard get_knights(Color side) const
	{ return position[side][KNIGHT]; }
	
	Bitboard get_bishops(Color side) const
	{ return position[side][BISHOP]; }
	
	Bitboard get_rooks(Color side) const
	{ return position[side][ROOK]; }
	
	Bitboard get_queens(Color side) const
	{ return position[side][QUEEN]; }
	
	Bitboard get_kings(Color side) const
	{ return position[side][KING]; }
	
	Bitboard get_pieces(Color side) const
	{ return position_all[side]; }
	
	Bitboard get_blocker() const
	{ return occupied; }
	
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
	Color color_at(Square sq) const;
	Piece piece_at(Square sq) const;
	Square get_eppawn() const;
      private:
	void set_side(Color side);
	void switch_sides();
	void place_piece(Square sq, Color side, Piece ptype);
	void remove_piece(Square sq, Color side, Piece ptype);
	void move_piece(Square from, Square to, Color side, Piece ptype);
	void set_flag(unsigned int flag);
	void clear_flag(unsigned int flag);
	void set_epsq(Square sq);
	void clear_epsq();
      public:
	inline bool in_check() const;
	inline bool is_legal() const;
	inline int material_difference() const;
	inline int get_material(Color side) const;
	
	/* Attack functions, defined in board_attack.cc */
      public:
	bool is_attacked(Square to, Color atkside) const;
      private:
	Bitboard attackers(Square to, Color atkside) const;
	Bitboard pinned(Square to, Color side) const;
	inline Bitboard pawn_captures(Square from, Color side) const;
	inline Bitboard pawn_noncaptures(Square from, Color side) const;
	inline Bitboard knight_attacks(Square from) const;
	inline Bitboard bishop_attacks(Square from) const;
	inline Bitboard rook_attacks(Square from) const;
	inline Bitboard queen_attacks(Square from) const;
	inline Bitboard king_attacks(Square from) const;
	
	/* Move generation functions, defined in board_generate.cc */
      public:
	void generate_moves(Movelist * movelist, bool allpromo) const;
	void generate_captures(Movelist * movelist, bool allpromo) const;

	/*
	 * We cannot simply use 'allpromo = true' as default parameter,
	 * because this would give these functions a different signature than
	 * generate_noncaptures(), and we could not use a function pointer
	 * to all generate_*() functions, like in Shell::cmd_show() 
	 */
	
	inline void generate_moves(Movelist * movelist) const
	{
		generate_moves(movelist, true);
	}

	inline void generate_captures(Movelist * movelist) const
	{
		generate_captures(movelist, true);
	}
	
	void generate_noncaptures(Movelist * movelist) const;
	void generate_escapes(Movelist * movelist) const;
      private:
	void generate_castling(Movelist * movelist) const;
	void add_moves(Movelist * movelist, Square from, Bitboard to_bb) const;
	void add_move(Movelist * movelist, Square from, Square to) const;	

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
	static Hashkey hashkeys[2][6][64];
	static Hashkey hash_side;
	static Hashkey hash_ep[64];
	static Hashkey hash_wk;
	static Hashkey hash_wq;
	static Hashkey hash_bk;
	static Hashkey hash_bq;

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
	unsigned int flags;
	Square epsq;
	unsigned int pce_movecnt_to;
};
#endif


/*****************************************************************************
 *
 * Inline functions of class board
 *
 *****************************************************************************/

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
	ASSERT_DEBUG(sq >= 0 && sq < 64);
	return pce_movecnt[sq];
}

inline bool Board::in_check() const
{
	return is_attacked(get_king(side), opponent);
}

/*
 * Check if the position is legal, i.e. the enemy king
 * cannot be captured with the next move.
 */
inline bool Board::is_legal() const
{
	return !is_attacked(get_king(opponent), side);
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


/*
 * Basic attack functions: 
 *
 * Note that they don't filter out illegal captures
 * of own pieces.
 * 
 * Rotated bitboards are used to calculate
 * bishop, rook and queen attacks.
 */

inline Bitboard Board::pawn_captures(Square from, Color side) const
{
	Bitboard bb = Bitboard::pawn_capt_bb[side][from] & get_blocker();
	if (epsq != NO_SQUARE 
			&& Bitboard::pawn_capt_bb[side][from].testbit(epsq))
		bb.setbit(epsq);

	return (bb);
}

inline Bitboard Board::pawn_noncaptures(Square from, Color side) const
{
	const int rank2 = (side == WHITE) ? RANK2 : RANK7;
	const int dir = (side == WHITE) ? +8 : -8;

	Bitboard bb = NULLBITBOARD;
	if (!get_blocker().testbit(from+dir)) {
		bb.setbit(from+dir);
		if (RNK(from) == rank2 && !get_blocker().testbit(from+2*dir))
			bb.setbit(from+2*dir);
	}

	return (bb);
}

inline Bitboard Board::knight_attacks(Square from) const
{
	return (Bitboard::attack_bb[KNIGHT][from]);
}

inline Bitboard Board::bishop_attacks(Square from) const
{
	return (occupied_l45.atkl45(from) | occupied_r45.atkr45(from));
}

inline Bitboard Board::rook_attacks(Square from) const
{
	return (occupied.atk0(from) | occupied_l90.atkl90(from));
}

inline Bitboard Board::queen_attacks(Square from) const
{
	return (bishop_attacks(from) | rook_attacks(from));
}

inline Bitboard Board::king_attacks(Square from) const
{
	return (Bitboard::attack_bb[KING][from]);
}


#endif // BOARD_H
