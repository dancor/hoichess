/* $Id: bitboard_asm.h 1462 2007-12-18 20:49:56Z holger $
 *
 * HoiChess/win32/bitboard_asm.h
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
 * This code was originally taken from Crafty.
 */

#ifdef USE_ASM_LSB
inline int Bitboard::lsb() const
{
	uint32_t rt;
	uint32_t bitsh = ((uint32_t) (bits>>32));
	uint32_t bitsl = ((uint32_t) bits);

	__asm {
		    mov     ebx, bitsl
		    bsf     eax, ebx
		    jnz     b
		    mov     ebx, bitsh
		    bsf     eax, ebx
		    jnz     a
		    mov     eax, -1
		    jmp     b
		a:  add     eax, 32
		b:  mov     rt, eax
	}
	
	return rt;
}
#endif

#ifdef USE_ASM_MSB
inline int Bitboard::msb() const
{
	uint32_t rt;
	uint32_t bitsh = ((uint32_t) (bits>>32));
	uint32_t bitsl = ((uint32_t) bits);

	__asm {
		    mov     ebx, bitsh
		    bsr     eax, ebx
		    jnz     a
		    mov     ebx, bitsl
		    bsr     eax, ebx
		    jnz     b
		    mov     eax, -1
		    jmp     b
		a:  add     eax, 32
		b:  mov     rt, eax
	}
	
	return rt;
}
#endif

#ifdef USE_ASM_POPCNT
#error "asm popcnt() not implemented"
#endif
