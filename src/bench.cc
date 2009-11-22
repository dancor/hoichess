/* $Id: bench.cc 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/bench.cc
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

#include "common.h"
#include "bench.h"

#include "board.h"
#include "clock.h"
#include "eval.h"

const char * Bench::fens[] = {
	/* positions of Bratko-Kopec test */
	"1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - - 0 1",
	"3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - - 0 1",
	"2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - - 0 1",
	"rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq - 0 1",
	"r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - - 0 1",
	"2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - - 0 1",
	"1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - - 0 1",
	"4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - - 0 1",
	"2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - - 0 1",
	"3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - - 0 1",
	"2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - - 0 1",
	"r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - - 0 1",
	"r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - - 0 1",
	"rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - - 0 1",
	"2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - - 0 1",
	"r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq - 0 1",
	"r2q1rk1/1ppnbppp/p2p1nb1/3Pp3/2P1P1P1/2N2N1P/PPB1QP2/R1B2RK1 b - - 0 1",
	"r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/N1P2N2/1PB3PP/R1B1QRK1 b - - 0 1",
	"3rr3/2pq2pk/p2p1pnp/8/2QBPP2/1P6/P5PP/4RRK1 b - - 0 1",
	"r4k2/pb2bp1r/1p1qp2p/3pNp2/3P1P2/2N3P1/PPP1Q2P/2KRR3 w - - 0 1",
	"3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - - 0 1",
	"2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - - 0 1",
	"r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq - 0 1",
	"r2qnrnk/p2b2b1/1p1p2pp/2pPpp2/1PP1P3/PRNBB3/3QNPPP/5RK1 w - - 0 1",

	/* some artificial positions with lots of captures */
	"k2nRnR1/3PP1p1/N1Nn1Pp1/3NnPpB/qqqP1P1B/3P4/QQQB3B/K5R1 w - - 0 1",
	"k2nRnR1/3PP1p1/N1Nn1Pp1/1q1NnPpB/q1qP1P1B/3P4/QQQB3B/K5R1 b - - 0 1",
	"kqRRrQNr/rnqpbnpr/QprPqpqP/pnPqPqPn/pPqPqPpP/QqNNNNNN/RBqrqbqr/KRrRrRrR w - - 0 1",

	/* some artificial positions to test generate_escapes */
	"k3r3/8/8/2n5/4K2q/2n5/8/7b w - - 0 1",
	"k2r1r2/8/8/8/4K3/8/8/4N2b w - - 0 1",
	"k2r1r2/8/7R/8/4K3/6N1/5P2/4N2b w - - 0 1",

	NULL
};


Bench::Bench()
{
}

Bench::~Bench()
{
}

void Bench::bench_movegen()
{
	printf("Running move generator benchmark...\n");
	log("bench movegen\n");
	
	unsigned int mps_moves = bench_movegen(&Board::generate_moves,
			"generate_moves");
	unsigned int mps_captures = bench_movegen(&Board::generate_captures,
			"generate_captures");
	unsigned int mps_noncaptures = bench_movegen(
			&Board::generate_noncaptures, "generate_noncaptures");
	unsigned int mps_escapes = bench_movegen(&Board::generate_escapes,
			"generate_escapes");

	log("mps_moves=%u\n", mps_moves);
	log("mps_captures=%u\n", mps_captures);
	log("mps_noncaptures=%u\n", mps_noncaptures);
	log("mps_escapes=%u\n", mps_escapes);
	if (verbose) {
		printf("Average speed of generate_moves:       %9uk moves/s\n",
				mps_moves / 1000);
		printf("Average speed of generate_captures:    %9uk moves/s\n",
				mps_captures / 1000);
		printf("Average speed of generate_noncaptures: %9uk moves/s\n",
				mps_noncaptures / 1000);
		printf("Average speed of generate_escapes:     %9uk moves/s\n",
				mps_escapes / 1000);
	}

	/* Calculate weighted average.
	 * In the current version of HoiChess, search uses generate_moves()
	 * instead generate_escapes(), which is buggy. */
	float mps_weighted = (float) (
			  mps_moves       *  5
			+ mps_captures    * 80
			+ mps_noncaptures * 15
			+ mps_escapes     *  0
		) / 100;
	printf("Average move generator speed (weighted average):"
							" %.1fk moves/s\n",
			mps_weighted / 1000);
	log("mps_weighted=%.0f\n", mps_weighted);
	log("bench movegen finished\n");
}

unsigned int Bench::bench_movegen(movegen_t movegen, const char * movegen_name)
{
	ASSERT(movegen != NULL);
	ASSERT(movegen_name != NULL);
	
	if (verbose >= 1) {
		printf("Move generator: %s\n", movegen_name);
	}

	float mps_sum = 0;
	unsigned int mps_cnt = 0;
	for (const char ** p = &fens[0]; *p != NULL; p++) {
		const char * fen = *p;
		if (verbose >= 2) {
			printf("\tPosition: %s\n", fen);
		}
		
		Board board(fen);
		Movelist movelist;
		Clock clock(1);
		clock.start();
		unsigned long moves = 0;
		while (!clock.timeout()) {
			for (unsigned int i=0; i<1000; i++) {
				movelist.clear();
				(board.*movegen)(&movelist);
				moves += movelist.size();
			}
			if (moves == 0) {
				break;
			}
		}
		clock.stop();
		
		float secs = (float) clock.get_elapsed_time() / 100;
		float mps;
		if (moves > 0 && secs > 0) {
			mps = moves / secs;
			mps_sum += mps;
			mps_cnt++;
			
			log("movegen=%s, position='%s', moves=%lu, time=%.2f, "
					"speed=%.0f\n",
					movegen_name, fen, moves, secs, mps);
			if (verbose >= 2) {
				printf("\tMoves generated: %lu in %.2f s"
						" (%.1fk moves/s)\n",
						moves, secs, mps / 1000);
			}
		}
	}
	
	float mps_avg = (mps_cnt > 0) ? (mps_sum / mps_cnt) : 0;
	return (unsigned int) mps_avg;
}

void Bench::bench_evaluator()
{
	printf("Running evaluator benchmark...\n");
	log("bench evaluator\n");

	float eps_sum = 0;
	unsigned int eps_cnt = 0;
	for (const char ** p = &fens[0]; *p != NULL; p++) {
		const char * fen = *p;
		if (verbose >= 2) {
			printf("\tPosition: %s\n", fen);
		}

		Board board(fen);
		Evaluator eval;
		Clock clock(1);
		clock.start();
		unsigned long evals = 0;
		while (!clock.timeout()) {
			for (unsigned int i=0; i<1000; i++) {
				eval.eval(board, -INFTY, INFTY, NO_COLOR);
				evals++;
			}
		}
		clock.stop();
		
		float secs = (float) clock.get_elapsed_time() / 100;
		float eps;
		if (evals > 0 && secs > 0) {
			eps = evals / secs;
			eps_sum += eps;
			eps_cnt++;

			log("position='%s', evals=%lu, time=%.2f, speed=%.0f\n",
					fen, evals, secs, eps);
			if (verbose >= 2) {
				printf("\tEvaluations: %lu in %.2f s"
						" (%.1fk evaluations/s)\n",
						evals, secs, eps / 1000);
			}
		}
	}

	float eps_avg = (eps_cnt > 0) ? (eps_sum / eps_cnt) : 0;
	printf("Average evaluator speed: %.1fk evaluations/s\n",
			eps_avg / 1000);	
	log("eps_avg=%.0f\n", eps_avg);
	log("bench evaluator finished\n");
}

void Bench::bench_makemove()
{
	printf("Running makemove benchmark...\n");
	log("bench makemove\n");

	float mps_sum = 0;
	unsigned int mps_cnt = 0;
	for (const char ** p = &fens[0]; *p != NULL; p++) {
		const char * fen = *p;
		if (verbose >= 2) {
			printf("\tPosition: %s\n", fen);
		}

		Board board(fen);
		Board newboard;
		Movelist movelist;
		board.generate_moves(&movelist);
		Clock clock(1);
		clock.start();
		unsigned long moves = 0;
		while (!clock.timeout()) {
			for (unsigned int i=0; i<1000; i++) {
				for (unsigned int k=0; k<movelist.size(); k++) {
					Move mov = movelist[k];
					ASSERT_DEBUG(mov.is_valid(board));
					/* We must copy the behavior of search,
					 * i.e., run make_move/unmake_move
					 * cycles, or, in case unmake_move is
					 * disabled, copy the board and call
					 * make_move on the copy. */
#ifdef USE_UNMAKE_MOVE
					BoardHistory hist
						= board.make_move(mov);
					board.unmake_move(hist);
#else
					newboard = board;
					newboard.make_move(mov);
#endif
					moves++;
				}
			}
		}
		clock.stop();
		
		float secs = (float) clock.get_elapsed_time() / 100;
		float mps;
		if (moves > 0 && secs > 0) {
			mps = moves / secs;
			mps_sum += mps;
			mps_cnt++;

			log("position='%s', moves=%lu, time=%.2f, speed=%.0f\n",
					fen, moves, secs, mps);
			if (verbose >= 2) {
				printf("\tMoves made: %lu in %.2f s"
						" (%.1fk moves/s)\n",
						moves, secs, mps / 1000);
			}
		}
	}
	
	float mps_avg = (mps_cnt > 0) ? (mps_sum / mps_cnt) : 0;
	printf("Average makemove speed: %.1fk moves/s\n", mps_avg / 1000);	
	log("mps_avg=%.0f\n", mps_avg);
	log("bench makemove finished\n");
}
