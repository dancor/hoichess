/* $Id: bitboard_asm.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/i386/bitboard_asm.h
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

/*
 * x86 assembler versions of msb/lsb scan routines.
 * This code was taken from Crafty and slightly modified to match
 * our interpretation of the bit positions.
 */

#ifdef USE_ASM_LSB
inline int Bitboard::lsb() const
{
	uint32_t dummy1, dummy2, dummy3;
	asm("    bsf     %2, %0"      "\n\t"
	    "    jnz     2f"          "\n\t"
	    "    bsf     %1, %0"      "\n\t"
	    "    jnz     1f"          "\n\t"
	    "    movl    $-1, %0"     "\n\t"
	    "    jmp     2f"          "\n\t"
	    "1:  addl    $32,%0"      "\n\t"
	    "2:"
		: "=&q" (dummy1), "=&q" (dummy2), "=&q" (dummy3)
		: "1" ((uint32_t) (bits>>32)), "2" ((uint32_t) bits)
		: "cc");
	
	return (dummy1);
}
#endif

#ifdef USE_ASM_MSB
inline int Bitboard::msb() const
{ 
	uint32_t dummy1, dummy2, dummy3;
	asm("    bsr     %1, %0"      "\n\t"
	    "    jnz     1f"          "\n\t"
	    "    bsr     %2, %0"      "\n\t"
	    "    jnz     2f"          "\n\t"
	    "    movl    $-1, %0"     "\n\t"
	    "    jmp     2f"          "\n\t"
	    "1:  addl    $32,%0"      "\n\t"
	    "2:"
		: "=&q" (dummy1), "=&q" (dummy2), "=&q" (dummy3)
		: "1" ((uint32_t) (bits>>32)), "2" ((uint32_t) bits)
		: "cc");
	
	return (dummy1);
}
#endif

/*
 * x86 assembler version of population count routine.
 * This code was taken from Crafty. However, it seems to be slower
 * than the LUT version.
 */

#ifdef USE_ASM_POPCNT
inline int Bitboard::popcnt() const
{
	uint32_t dummy1, dummy2, dummy3, dummy4;

	asm("        xorl    %0, %0"                    "\n\t"
	    "        testl   %2, %2"                    "\n\t"
	    "        jz      2f"                        "\n\t"
	    "1:      leal    -1(%2), %1"                "\n\t"
	    "        incl    %0"                        "\n\t"
	    "        andl    %1, %2"                    "\n\t"
	    "        jnz     1b"                        "\n\t"
	    "2:      testl   %3, %3"                    "\n\t"
	    "        jz      4f"                        "\n\t"
	    "3:      leal    -1(%3), %1"                "\n\t"
	    "        incl    %0"                        "\n\t"
	    "        andl    %1, %3"                    "\n\t"
	    "        jnz     3b"                        "\n\t"
	    "4:"                                        "\n\t"
		: "=&q" (dummy1), "=&q" (dummy2), "=&q" (dummy3), "=&q" (dummy4)
		: "2" ((uint32_t) (bits>>32)), "3" ((uint32_t) bits)
		: "cc");

	return (dummy1);
}
#endif
