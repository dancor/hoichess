/* $Id: bitboard.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/bitboard.h
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
#ifndef BITBOARD_H
#define BITBOARD_H

#include "common.h"
#include "basic.h"


#define NULLBITBOARD    ((uint64_t) 0)


class Bitboard
{
      private:
	uint64_t bits;

	/* Constructors */
      public:
	FORCEINLINE Bitboard();
	FORCEINLINE Bitboard(uint64_t bits);

	/* Operators / Casts */
      public:
	FORCEINLINE Bitboard & operator=(const Bitboard & bb);
	FORCEINLINE operator uint64_t() const;

	FORCEINLINE Bitboard operator&(const Bitboard & bb) const;
	FORCEINLINE Bitboard operator|(const Bitboard & bb) const;
	FORCEINLINE Bitboard operator~() const;
	FORCEINLINE Bitboard & operator&=(const Bitboard & bb);
	FORCEINLINE Bitboard & operator|=(const Bitboard & bb);
	FORCEINLINE Bitboard & operator++();
	FORCEINLINE Bitboard operator++(int);

	/* Basic bitboard functions */
      public:
	FORCEINLINE void setbit(unsigned int b);
	FORCEINLINE void clearbit(unsigned int b);
	FORCEINLINE bool testbit(unsigned int b) const;
	
	inline int firstbit() const;
	inline int lsb() const;
	inline int msb() const;
	inline int popcnt() const;

	/* Rotated attack functions */
      public:
	inline Bitboard atk0(Square from) const;
	inline Bitboard atkl90(Square from) const;
	inline Bitboard atkl45(Square from) const;
	inline Bitboard atkr45(Square from) const;
	
	/* Utility functions */
      public:
	void print() const;
	void print2() const;

	/* Static Data Members */
      public:
	static Bitboard file[8];
	static Bitboard rank[8];
	static Bitboard attack_bb[6][64];
	static Bitboard pawn_capt_bb[2][64];
	static Bitboard ray_bb[64][64];
	
	static Bitboard passed_pawn_mask[2][64];
	static Bitboard isolated_pawn_mask[64];
	static Bitboard connected_pawn_mask[64];

	static const int map_l90[64];
	static const int map_l45[64];
	static const int map_r45[64];
	
      private:
	static int8_t lsb_lut[65536];
	static int8_t msb_lut[65536];
	static int8_t popcnt_lut[65536];
	
	static const Square inv_map_l90[64];
	static const Square inv_map_l45[64];
	static const Square inv_map_r45[64];
	static const int shift_0[64];
	static const int shift_l90[64];
	static const int shift_l45[64];
	static const int shift_r45[64];
	static const int diaglen_l45[64];
	static const int diaglen_r45[64];
	static const int mask_l45[64];
	static const int mask_r45[64];
	static Bitboard rot_atk_0[64][256];
	static Bitboard rot_atk_l90[64][256];
	static Bitboard rot_atk_l45[64][256];
	static Bitboard rot_atk_r45[64][256];
	
	/* Static Member Functions */
      public:
	static void init();
      private:
	static void init_attack_bb();
	static void init_pawn_capt_bb();
	static void init_ray_bb();
	static void init_masks();
	static void init_rot_atk();
};


#if defined(__GNUC__) && defined(__i386__)

#define USE_ASM_LSB
#define USE_ASM_MSB
//#define USE_ASM_POPCNT	// LUT version is faster
#include "i386/bitboard_asm.h"

#elif defined(WIN32)

#define USE_ASM_LSB
#define USE_ASM_MSB
//#define USE_ASM_POPCNT	// LUT version is faster
#include "win32/bitboard_asm.h"

#endif


inline Bitboard::Bitboard()
	: bits((uint64_t) 0)
{}

inline Bitboard::Bitboard(uint64_t _bits)
	: bits(_bits)
{}


inline Bitboard & Bitboard::operator=(const Bitboard & bb)
{
	bits = bb.bits;
	return *this;
}

inline Bitboard::operator uint64_t() const
{
	return bits;
}


inline Bitboard Bitboard::operator&(const Bitboard & bb) const
{
	return bits & bb.bits;
}

inline Bitboard Bitboard::operator|(const Bitboard & bb) const
{
	return bits | bb.bits;
}

inline Bitboard Bitboard::operator~() const
{
	return ~bits;
}

inline Bitboard & Bitboard::operator&=(const Bitboard & bb)
{
	bits &= bb.bits;
	return *this;
}

inline Bitboard & Bitboard::operator|=(const Bitboard & bb)
{
	bits |= bb.bits;
	return *this;
}

/* This is the prefix operator++ */
inline Bitboard & Bitboard::operator++()
{
	bits++;
	return *this;
}

/* This is the postfix operator++ */
inline Bitboard Bitboard::operator++(int)
{
	Bitboard tmp = *this;
	bits++;
	return tmp;
}


inline void Bitboard::setbit(unsigned int b)
{
	bits |= (((uint64_t) 1) << b);
}

inline void Bitboard::clearbit(unsigned int b)
{
	bits &= ~(((uint64_t) 1) << b);
}

inline bool Bitboard::testbit(unsigned int b) const
{
	return (bits & (((uint64_t) 1) << b));
}


inline int Bitboard::firstbit() const
{
	/* We take msb, because the LUT version of msb() is
	 * faster than lsb(). */
	return msb();
}


/*
 * Lookup table version of msb/lsb scan routines.
 * This code was taken from GNU Chess.
 */

#ifndef USE_ASM_LSB
inline int Bitboard::lsb() const
{
	if (bits & 0xffff) return lsb_lut[bits & 0xffff];
	if ((bits >> 16) & 0xffff) return lsb_lut[(bits >> 16) & 0xffff] + 16;
	if ((bits >> 32) & 0xffff) return lsb_lut[(bits >> 32) & 0xffff] + 32;
	if ((bits >> 48) & 0xffff) return lsb_lut[(bits >> 48) & 0xffff] + 48;
	return -1;
}
#endif

#ifndef USE_ASM_MSB
inline int Bitboard::msb() const
{
	if (bits >> 48) return msb_lut[bits >> 48] + 48;
	if (bits >> 32) return msb_lut[bits >> 32] + 32;
	if (bits >> 16) return msb_lut[bits >> 16] + 16;
	return msb_lut[bits];
}
#endif

/*
 * Lookup table version of population count routine.
 * This code was taken from GNU Chess.
 */

#ifndef USE_ASM_POPCNT
inline int Bitboard::popcnt() const
{
	return popcnt_lut[bits & 0xffff]
		+ popcnt_lut[(bits >> 16) & 0xffff]
		+ popcnt_lut[(bits >> 32) & 0xffff]
		+ popcnt_lut[(bits >> 48) & 0xffff];
}
#endif


/*
 * Rotated attack functions.
 * They obviously only make sense when called
 * for the matching rotated bitboard.
 */

inline Bitboard Bitboard::atk0(Square from) const
{
	return rot_atk_0[from][(bits >> shift_0[from]) & 0xff];
}

inline Bitboard Bitboard::atkl90(Square from) const
{
	return rot_atk_l90[from][(bits >> shift_l90[from]) & 0xff];
}

inline Bitboard Bitboard::atkl45(Square from) const
{
	return rot_atk_l45[from][(bits >> shift_l45[from]) & mask_l45[from]];
}

inline Bitboard Bitboard::atkr45(Square from) const
{
	return rot_atk_r45[from][(bits >> shift_r45[from]) & mask_r45[from]];
}


#endif // BITBOARD_H
