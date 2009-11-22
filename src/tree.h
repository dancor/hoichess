/* $Id: tree.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/tree.h
 *
 * Copyright (C) 2005 Holger Ruckdeschel <holger@hoicher.de>
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
#ifndef TREE_H
#define TREE_H

#include "common.h"
#include "board.h"
#include "historytable.h"

/* forward declaration */
class Tree;

class Node {
	friend class Tree;

      public:
	enum node_type { UNKNOWN, ROOT, FULLWIDTH, QUIESCE };

      private:
	enum node_state {
		GEN_CAPTURES, SCORE_CAPTURES, CAPTURES,
		GEN_NONCAPTURES, SCORE_NONCAPTURES, NONCAPTURES,
		GEN_ESCAPES, SCORE_ESCAPES, ESCAPES,
		SCORE_ALL, ALL,
		DONE
	};
	
      private:
	Tree * tree;	
#ifdef USE_UNMAKE_MOVE
	BoardHistory hist;
#else
	Board board;
#endif
	Hashkey hashkey;
	bool incheck;
	int material;
	Movelist movelist;
	bool captures_generated;
	bool noncaptures_generated;
	bool escapes_generated;
	int current_move_no;

	enum node_type type;
	enum node_state state;
	Move best;
	Move hashmv;
	Move killer1;
	Move killer2;
	HistoryTable * historytable;
	Move played_move;

      public:
	Node();
	
      public:
	Move first();
	Move next();
	Move pick();

	void score_moves();

	inline Hashkey get_hashkey() const;
	inline bool in_check() const;
	inline int material_balance() const;
		
	inline unsigned int get_movelist_size() const;
	inline unsigned int get_current_move_no() const;
	inline Move get_current_move() const;

	inline void set_current_score(int score);
	
	inline enum node_type get_type() const;
	inline void set_type(enum node_type t);
	
	inline Move get_best() const;
	inline void set_best(Move mov);
	
	inline Move get_hashmv() const;
	inline void set_hashmv(Move mov);

	inline void set_historytable(HistoryTable * ht);
	inline void add_killer(Move mov);
	inline Move get_played_move() const;
};

class Tree {
      private:
#ifdef USE_UNMAKE_MOVE
	Board board;
	Board rootboard;
#endif
	Node * nodes;
	unsigned int current_ply;

      public:
	Tree();
	~Tree();

      public:
	void clear_killer();
	void set_root(const Board & board);
	Node * make_move(Move mov);
	void unmake_move();

	inline Node * operator[](unsigned int ply);
	inline const Node * operator[](unsigned int ply) const;

	inline const Board & get_board() const;
	inline Board get_rootboard() const;
	inline unsigned int get_current_ply() const;
};


/*****************************************************************************
 *
 * Inline functions of class Node
 *
 *****************************************************************************/

inline Hashkey Node::get_hashkey() const
{
	return hashkey;
}

inline bool Node::in_check() const
{
	return incheck;
}

inline int Node::material_balance() const
{
	return material;
}

inline unsigned int Node::get_movelist_size() const
{
	return movelist.size();
}

inline unsigned int Node::get_current_move_no() const
{
	return current_move_no;
}

inline Move Node::get_current_move() const
{
	return movelist[current_move_no];
}

inline void Node::set_current_score(int score)
{
	movelist.set_score(current_move_no, score);
}

inline enum Node::node_type Node::get_type() const
{
	return type;
}

inline void Node::set_type(enum Node::node_type t)
{
	type = t;
}

inline Move Node::get_best() const
{
	return best;
}

inline void Node::set_best(Move mov)
{
	best = mov;
}

inline Move Node::get_hashmv() const
{
	return hashmv;
}

inline void Node::set_hashmv(Move mov)
{
	hashmv = mov;
}

inline void Node::set_historytable(HistoryTable * ht)
{
	historytable = ht;
}

inline void Node::add_killer(Move mov)
{
	if (killer1 == NO_MOVE) {
		killer1 = mov;
	} else if (killer2 != killer1) {
		killer2 = mov;
	}
}

inline Move Node::get_played_move() const
{
	return played_move;
}


/*****************************************************************************
 *
 * Inline functions of class Tree
 *
 *****************************************************************************/

inline Node * Tree::operator[](unsigned int ply)
{
	ASSERT_DEBUG(ply <= current_ply);
	return &nodes[ply];
}

inline const Node * Tree::operator[](unsigned int ply) const
{
	ASSERT_DEBUG(ply <= current_ply);
	return &nodes[ply];
}

inline const Board & Tree::get_board() const
{
#ifdef USE_UNMAKE_MOVE
	return board;
#else
	return nodes[current_ply].board;
#endif
}

inline Board Tree::get_rootboard() const
{
#ifdef USE_UNMAKE_MOVE
	return rootboard;
#else
	return nodes[0].board;
#endif
}

inline unsigned int Tree::get_current_ply() const
{
	return current_ply;
}

#endif // TREE_H
