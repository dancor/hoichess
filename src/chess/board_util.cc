/* $Id: board_util.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/board_util.cc
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
#include "board.h"
#include "move.h"
#include "basic.h"

#include <stdio.h>

bool Board::is_mate() const
{
	Movelist movelist;
	generate_moves(&movelist);
	movelist.filter_illegal(*this);
	return (in_check() && movelist.size() == 0);
}

bool Board::is_stalemate() const
{
	Movelist movelist;
	generate_moves(&movelist);
	movelist.filter_illegal(*this);
	return (!in_check() && movelist.size() == 0);
}

bool Board::is_valid() const
{
	/* Each side must have exactly one king. */
	if (get_kings(WHITE).popcnt() != 1)
		return false;
	if (get_kings(BLACK).popcnt() != 1)
		return false;

	/* Pawns cannot be on rank 1 or rank 8. */
	if ((get_pawns(WHITE) | get_pawns(BLACK))
			& (Bitboard::rank[RANK1] | Bitboard::rank[RANK8]))
		return false;
	
	/* The opponent's king must not be in check. */
	if (is_attacked(get_king(opponent), side))
		return false;
	
	return true;
}

bool Board::is_material_draw() const
{
	if (material[WHITE] == 0 && material[BLACK] == 0) {
		/* only kings left -> draw */
		return true;
	} else if (get_pawns(WHITE) || get_pawns(BLACK)) {
		/* there are still pawns -> no draw */
 		return false;
	} else if ((material[WHITE] < mat_values[ROOK]
			|| (material[WHITE] == 2 * mat_values[KNIGHT]
				&& get_knights(WHITE).popcnt() == 2))
		&& (material[BLACK] < mat_values[ROOK]
			|| (material[BLACK] == 2 * mat_values[KNIGHT]
				&& get_knights(BLACK).popcnt() == 2))) {
		/* both sides have only a knight or a bishop -> draw
		 * both sides have <= 2 knights only -> draw */
		return true;
	}
		
	/* TODO  more */
	
	
	return false;
}

void Board::print(FILE * fp, Move last_move) const
{
	Square sq;
	char c;
	for (int i=7; i>=0; i--) {
		fprintf(fp, "  +---+---+---+---+---+---+---+---+\n");
		fprintf(fp, "%d ", i+1);
		for (int j=0; j<=7; j++) {
			sq = SQUARE(i,j);
			if (piece_at(sq) != NO_PIECE) {
				c = piece_char[piece_at(sq)];
				c = (color_at(sq) == WHITE)
					? toupper(c) : tolower(c);
			} else {
				c = ' ';
			}
			
			if (ansicolor && fp == stdout) {
				const char * a1 = (i+j)%2
					? "\033[47m" : "\033[40m";

				const char * a2 = "";
				const char * a3 = "";
				if (last_move && (sq == last_move.from() 
						|| sq == last_move.to())) {
					a2 = XSIDE(side) == WHITE
						? "\033[31;1m"
						: "\033[34;1m";
					a3 = "\033[7m";
				} else {
					if (color_at(sq) == WHITE) {
						a2 = "\033[31;1m";
					} else if (color_at(sq) == BLACK) {
						a2 = "\033[34;1m";
					}
				}

				const char * a4 = "\033[0m";
				fprintf(fp, "|%s %s%s%c%s%s %s", a1, a2, a3, 
						c, a4, a1, a4);
			} else {
				fprintf(fp, "| %c ", c);
			}
		}
		fprintf(fp, "|\n");
	}
	fprintf(fp, "  +---+---+---+---+---+---+---+---+\n");
	fprintf(fp, "    a   b   c   d   e   f   g   h\n");
	
	fprintf(fp, "\n  [%d] %s  %c%c%c%c  %s\n",
			moveno,
			(side == WHITE) ? "White" : "Black",
			(flags & WKCASTLE) ? 'K' : ' ',
			(flags & WQCASTLE) ? 'Q' : ' ',
			(flags & BKCASTLE) ? 'k' : ' ',
			(flags & BQCASTLE) ? 'q' : ' ',
			(epsq != NO_SQUARE) ? square_str[epsq] : "-" );
}

void Board::print_small(FILE * fp) const
{
	Square sq;
	char c;
	for (int i=7; i>=0; i--) {
		//fprintf(fp, "%d ", i+1);
		for (int j=0; j<=7; j++) {
			sq = SQUARE(i,j);
			if (piece_at(sq) != NO_PIECE) {
				c = piece_char[piece_at(sq)];
				if (color_at(sq) == WHITE)
					c = toupper(c);
				else 
					c = tolower(c);
			} else {
				c = '.';
			}			
			fprintf(fp, " %c", c);
		}
		fprintf(fp, "\n");
	}
	//fprintf(fp, "\n   a b c d e f g h\n");
	
	fprintf(fp, "\n [%d] %s  %c%c%c%c  %s\n",
			moveno,
			(side == WHITE) ? "White" : "Black",
			(flags & WKCASTLE) ? 'K' : ' ',
			(flags & WQCASTLE) ? 'Q' : ' ',
			(flags & BKCASTLE) ? 'k' : ' ',
			(flags & BQCASTLE) ? 'q' : ' ',
			(epsq != NO_SQUARE) ? square_str[epsq] : "-" );
}

std::string Board::get_fen() const
{
	char ss[128];
	char * s = ss;

	/* position */
	Square sq;
	Piece pce;
	int empty = 0;
	for (int rnk = RANK8; rnk >= RANK1; rnk--) {
		for (int fil = FILEA; fil <= FILEH; fil++) {
			sq = SQUARE(rnk, fil);
			pce = piece_at(sq);
			if (pce == NO_PIECE) {
				empty++;
			} else {
				if (empty != 0) {
					*s++ = '0' + empty;
					empty = 0;
				}
	
				if (color_at(sq) == WHITE)
					*s++ = toupper(piece_char[pce]);
				else
					*s++ = tolower(piece_char[pce]);
			}		
	
			if (FIL(sq) == FILEH) {
				if (empty != 0) {
					*s++ = '0' + empty;
					empty = 0;
				}
	
				if (RNK(sq) != RANK1) 
					*s++ = '/';
			}
		}
	}
	
	/* side to move */
	*s++ = ' ';
	*s++ = (side == WHITE) ? 'w' : 'b';

	/* castling flags */
	*s++ = ' ';
	if (flags & WKCASTLE)
		*s++ = 'K';
	if (flags & WQCASTLE)
		*s++ = 'Q';
	if (flags & BKCASTLE)
		*s++ = 'k';
	if (flags & BQCASTLE)
		*s++ = 'q';
	if (! (flags & (WCASTLE | BCASTLE)))
		*s++ = '-';

	/* enpassant square, halfmove and fullmove clocks */
	int n = snprintf(s, 20, " %s %d %d",
			(epsq != NO_SQUARE) ? square_str[epsq] : "-",
			movecnt50,
			moveno);
	s += n;
		
	return std::string(ss);
}

bool Board::parse_fen(const char * s)
{
	clear();
	
	char * p; 

	char pos[80+1];
	char clr;
	char flg[4+1];
	char ep[2+1];
	int hmc;
	int fmc;

	if (sscanf(s, "%80s %c %4s %2s %d %d", pos, &clr, flg, ep, &hmc, &fmc)
			!= 6)
		return false;

	/* position */
	int rnk = RANK8, fil = FILEA;
	Square sq;
	p = pos;
	while (*p) {
		if (fil > FILEH+1 || rnk < RANK1)
			return false;

		sq = SQUARE(rnk, fil);
		switch (*p++) {
			case 'P': place_piece(sq, WHITE, PAWN); fil++; break;
			case 'N': place_piece(sq, WHITE, KNIGHT); fil++; break;
			case 'B': place_piece(sq, WHITE, BISHOP); fil++; break;
			case 'R': place_piece(sq, WHITE, ROOK); fil++; break;
			case 'Q': place_piece(sq, WHITE, QUEEN); fil++; break;
			case 'K': place_piece(sq, WHITE, KING); fil++; break;
			case 'p': place_piece(sq, BLACK, PAWN); fil++; break;
			case 'n': place_piece(sq, BLACK, KNIGHT); fil++; break;
			case 'b': place_piece(sq, BLACK, BISHOP); fil++; break;
			case 'r': place_piece(sq, BLACK, ROOK); fil++; break;
			case 'q': place_piece(sq, BLACK, QUEEN); fil++; break;
			case 'k': place_piece(sq, BLACK, KING); fil++; break;
			case '8': fil += 8; break;
			case '7': fil += 7; break;
			case '6': fil += 6; break;
			case '5': fil += 5; break;
			case '4': fil += 4; break;
			case '3': fil += 3; break;
			case '2': fil += 2; break;
			case '1': fil += 1; break;
			case '/':
				  fil = FILEA;
				  rnk--;
				  break;
			default:
				  return false;
		}
	}

	/* side to move */
	if (clr == 'w')
		set_side(WHITE);
	else if (clr == 'b')
		set_side(BLACK);
	else
		return false;

	/* castling flags */
	p = flg;
	while (*p) {
		switch (*p++) {
			case 'K': set_flag(WKCASTLE); break;
			case 'Q': set_flag(WQCASTLE); break;
			case 'k': set_flag(BKCASTLE); break;
			case 'q': set_flag(BQCASTLE); break;
			case '-': 
				  break;
			default:
				return false;
		}
	}

	/* enpassant square */
	if (ep[0] == '-') {
		clear_epsq();
	} else {
		int f = ep[0] - 'a';
		int r = ep[1] - '1';
		
		if (f < FILEA || f > FILEH)
			return false;
		else if (r != RANK3 && r != RANK6)
			return false;

		set_epsq(SQUARE(r, f));
	}
	
	/* halfmove and fullmove clocks */
	movecnt50 = hmc;
	moveno = fmc;
	
	/* validate position */
	if (!is_valid() || !is_legal())
		return false;

	return true;
}

bool Board::parse_fen(const std::string & str)
{
	return parse_fen(str.c_str());
}

Move Board::parse_move(const std::string & str) const
{
	/*
	 * Compare the input against all possible moves in coordinate
	 * notation and SAN.
	 */
	Movelist moves;
	generate_moves(&moves);
	moves.filter_illegal(*this);
	for (unsigned int i=0; i<moves.size(); i++) {
		Move mov = moves[i];

		if (str == mov.str() || str == mov.san(*this)) {
			return mov;
		}

		/* Try non-standard SAN (omitted '+' or '#', etc.) */
		for (int j=1; j<=5; j++) {
			if (str == mov.san(*this, j)) {
				return mov;
			}
		}
	}
	
	return NO_MOVE;
}

/*
 * Alternative implementation of move parser. The actual parsing is done
 * in do_parse_move_1().
 *
 * This version is much much faster than parse_move() above, which is 
 * especially nice during opening book generation. However, the new code
 * is a very quick and dirty hack, so we won't use it in places where speed
 * is not important at all, e.g. when parsing user input in shell.
 */
Move Board::parse_move_1(const std::string & str) const
{
	Move mov = do_parse_move_1(str);
	if (mov && is_valid_move(mov) && is_legal_move(mov)) {
		return mov;
	} else {
		return NO_MOVE;
	}
}

/*
 * This is write-only code.
 */
Move Board::do_parse_move_1(const std::string & str) const
{
	/* Castling */
	if (str == "O-O" || str == "O-O+" || str == "O-O#") {
		if (side == WHITE && (flags & WKCASTLE)) {
			return Move::castle(E1, G1);
		} else if (side == BLACK && (flags & BKCASTLE)) {
			return Move::castle(E8, G8);
		} else {
			return NO_MOVE;
		}
	} else if (str == "O-O-O" || str == "O-O-O+" || str == "O-O-O#") {
		if (side == WHITE && (flags & WQCASTLE)) {
			return Move::castle(E1, C1);
		} else if (side == BLACK && (flags & BQCASTLE)) {
			return Move::castle(E8, C8);
		} else {
			return NO_MOVE;
		}
	}

	unsigned int len = str.length();
	std::string str1 = str + "                ";
	const char * p = str1.c_str();

	int from_file = -1;
	int from_rank = -1;
	int to_file = -1;
	int to_rank = -1;
	Square from = NO_SQUARE;
	Square to = NO_SQUARE;
	Piece ptype = NO_PIECE;
	Piece cap_ptype = NO_PIECE;
	Piece promo_ptype = NO_PIECE;

	if ((len == 4 || len == 5)
			&& p[0] >= 'a' && p[0] <= 'h'
			&& p[1] >= '1' && p[1] <= '8'
			&& p[2] >= 'a' && p[2] <= 'h'
			&& p[3] >= '1' && p[3] <= '8') {
		/*
		 * Coordinate notation
		 */
		
		from_file = p[0] - 'a';
		from_rank = p[1] - '1';
		to_file = p[2] - 'a';
		to_rank = p[3] - '1';

		from = SQUARE(from_rank, from_file);
		to = SQUARE(to_rank, to_file);
		ptype = piece_at(from);
		cap_ptype = piece_at(to);

		if (ptype == KING) {
			/* castling? */
			if (side == WHITE && (flags & WKCASTLE) 
					&& from == E1 && to == G1) {
				return Move::castle(E1, G1);
			} else if (side == WHITE && (flags & WQCASTLE) 
					&& from == E1 && to == C1) {
				return Move::castle(E1, C1);
			} else if (side == BLACK && (flags & BKCASTLE) 
					&& from == E8 && to == G8) {
				return Move::castle(E8, G8);
			} else if (side == BLACK && (flags & BQCASTLE) 
					&& from == E8 && to == C8) {
				return Move::castle(E8, C8);
			}
		}

		if (len == 5) {
			/* Pawn promotion */
			switch (p[4]) {
			case 'n': promo_ptype = KNIGHT;	break;
			case 'b': promo_ptype = BISHOP;	break;
			case 'r': promo_ptype = ROOK;	break;
			case 'q': promo_ptype = QUEEN;	break;
			default:
				  return NO_MOVE;
			}

			if (cap_ptype != NO_PIECE) {
				return Move::promotion_capture(from, to,
						promo_ptype, cap_ptype);
			} else {
				return Move::promotion(from, to, promo_ptype);
			}
		} else {
			if (cap_ptype != NO_PIECE) {
				return Move::capture(from, to, ptype,
						cap_ptype);
			} else if (to == get_epsq()) {
				return Move::enpassant(from, to);
			} else {
				return Move::normal(from, to, ptype);
			}
		}
	}
	
	/*
	 * SAN
	 */
	
	if (p[0] >= 'a' && p[0] <= 'h') {
		/* Pawn move */
		  
		if (p[1] == 'x' && p[2] >= 'a' && p[2] <= 'h'
				&& p[3] >= '1' && p[3] <= '8') {
			/* capture */
			from_file = p[0] - 'a';
			to_file = p[2] - 'a';
			to_rank = p[3] - '1';
			if (side == WHITE) {
				from_rank = to_rank - 1;
			} else {
				from_rank = to_rank + 1;
			}

			from = SQUARE(from_rank, from_file);
			to = SQUARE(to_rank, to_file);
			cap_ptype = piece_at(to);

			if (p[4] == '=') {
				/* promotion capture */
				switch (p[5]) {
				case 'N': promo_ptype = KNIGHT;	break;
				case 'B': promo_ptype = BISHOP;	break;
				case 'R': promo_ptype = ROOK;	break;
				case 'Q': promo_ptype = QUEEN;	break;
				default:
					  return NO_MOVE;
				}

				return Move::promotion_capture(from, to,
						promo_ptype, cap_ptype);
			} else {
				/* non-promotion capture */
				if (to == get_epsq()) {
					return Move::enpassant(from, to);
				} else {
					return Move::capture(from, to, PAWN,
							cap_ptype);
				}
			}
		} else if (p[1] >= '1' && p[1] <= '8') {
			/* non-capture */
			from_file = p[0] - 'a';
			to_file = p[0] - 'a';
			to_rank = p[1] - '1';
			if (side == WHITE) {
				if (to_rank == 3) {
					if (piece_at(SQUARE(2, from_file))
							== PAWN) {
						/* one square ahead */
						from_rank = 2;
					} else if (piece_at(SQUARE(1,
							from_file) == PAWN)) {
						/* two squares ahead */
						from_rank = 1;
					} else {
						return NO_MOVE;
					}
				} else {
					from_rank = to_rank - 1;
				}
			} else {
				if (to_rank == 4) {
					if (piece_at(SQUARE(5, from_file))
							== PAWN) {
						/* one square ahead */
						from_rank = 5;
					} else if (piece_at(SQUARE(6,
							from_file) == PAWN)) {
						/* two squares ahead */
						from_rank = 6;
					} else {
						return NO_MOVE;
					}
				} else {
					from_rank = to_rank + 1;
				}
			}

			from = SQUARE(from_rank, from_file);
			to = SQUARE(to_rank, to_file);
			
			if (p[2] == '=') {
				/* promotion non-capture */
				switch (p[3]) {
				case 'N': promo_ptype = KNIGHT;	break;
				case 'B': promo_ptype = BISHOP;	break;
				case 'R': promo_ptype = ROOK;	break;
				case 'Q': promo_ptype = QUEEN;	break;
				default:
					  return NO_MOVE;
				}

				return Move::promotion(from, to, promo_ptype);
			} else {
				/* non-promotion non-capture */
				return Move::normal(from, to, PAWN);
			}
		} else {
			return NO_MOVE;
		}
		  
	} 

	/*
	 * SAN: Non-Pawn move
	 */
	
	switch (*p++) {
	case 'N': ptype = KNIGHT;	break;
	case 'B': ptype = BISHOP;	break;
	case 'R': ptype = ROOK;		break;
	case 'Q': ptype = QUEEN;	break;
	case 'K': ptype = KING;		break;

	default:
		  return NO_MOVE;
	}

	
	int f1 = -1, r1 = -1, f2 = -1, r2 = -1;
	
	if (*p >= 'a' && *p <= 'h') {
		f1 = *p - 'a';
		p++;
	}

	if (*p >= '1' && *p <= '8') {
		r1 = *p - '1';
		p++;
	}
	
	bool capture = false;
	if (*p == 'x') {
		capture = true;
		p++;
	}
	
	if (*p >= 'a' && *p <= 'h') {
		f2 = *p - 'a';
		p++;
	}

	if (*p >= '1' && *p <= '8') {
		r2 = *p - '1';
		p++;
	}

	if (f2 != -1 && r2 != -1) {
		from_file = f1;
		from_rank = r1;
		to_file = f2;
		to_rank = r2;
	} else if (f2 == -1 && r2 == -1) {
		to_file = f1;
		to_rank = r1;
	} else {
		return NO_MOVE;
	}

	if (to_file == -1 || to_rank == -1) {
		return NO_MOVE;
	}
	to = SQUARE(to_rank, to_file);
	
	if (from_rank == -1 || from_file == -1) {
		Bitboard from_bb;
		switch (ptype) {
		case KNIGHT:
			from_bb = knight_attacks(to) & get_knights(side);
			break;
		case BISHOP:
			from_bb = bishop_attacks(to) & get_bishops(side);
			break;
		case ROOK:   
			from_bb = rook_attacks(to) & get_rooks(side);
			break;
		case QUEEN:
			from_bb = queen_attacks(to) & get_queens(side);
			break;
		case KING: 
			from_bb = king_attacks(to) & get_kings(side);
			break;
		default:
			BUG("should not get here");
		}

		if (from_rank != -1) {
			from_bb &= Bitboard::rank[from_rank];
		}
		if (from_file != -1) {
			from_bb &= Bitboard::file[from_file];
		}

		unsigned int n = from_bb.popcnt();
		if (n == 0) {
			return NO_MOVE;
		} else if (n == 1) {
			from = from_bb.firstbit();
		} else {
			Bitboard pins = pinned(get_king(side), side);
			Bitboard ray = Bitboard::ray_bb[to][get_king(side)];
			bool ok = false;
			while (from_bb) {
				from = from_bb.firstbit();
				from_bb.clearbit(from);

				if (pins.testbit(from) && ray.testbit(from)) {
					ok = true;
					break;
				} else if (!pins.testbit(from)) {
					ok = true;
					break;
				}
			}

			if (!ok) {
				return NO_MOVE;
			}
		}
	} else {
		from = SQUARE(from_rank, from_file);
	}
		
	if (piece_at(from) != ptype) {
		return NO_MOVE;
	}
	cap_ptype = piece_at(to);
	if (capture && cap_ptype == NO_PIECE) {
		return NO_MOVE;
	} else if (!capture && cap_ptype != NO_PIECE) {
		return NO_MOVE;
	}
	
	if (cap_ptype != NO_PIECE) {
		return Move::capture(from, to, ptype, cap_ptype);
	} else {
		return Move::normal(from, to, ptype);
	}
}


bool Board::operator==(const Board & board) const
{
	for (Piece pce = PAWN; pce <= KING; pce++) {
		if (position[WHITE][pce] != board.position[WHITE][pce])
			return false;
		if (position[BLACK][pce] != board.position[BLACK][pce])
			return false;
	}
	
	if (side != board.side || flags != board.flags 
			|| epsq != board.epsq)
		return false;

	return true;
}
