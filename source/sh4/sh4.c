/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2002  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>

#include "sh4.h"

void x86Init(char *ptr) {
	x86Ptr = ptr;
}

void x86Shutdown() {
}

void Align4() {
	if ((u32)x86Ptr & 0x3)
		x86Ptr = (u8*) (((u32)x86Ptr & ~0x3) + 0x4);	// Align on Dword boundary
}

void Align32() {
	if ((u32)x86Ptr & 0x1f)
		x86Ptr = (u8*) (((u32)x86Ptr & ~0x1f) + 0x20);	// Align on ICache boundary
}

void shSetImm(u8 *j8) {
	u32 tmp = (u32)j8 & 0xfffffffc;
	*j8 = (u8)((((u32)x86Ptr - tmp) - 4)>>2);
}

void shSetBRA(u16 *j16) {	// 12 bit displacement
	*j16 = (*j16 & 0xf000) | ((((u32)x86Ptr - (u32)j16) - 4)>>1);
}

void shSetBx(u8 *j8) {
	*j8 = (((u8*)x86Ptr - j8) - 4)>>1;
}

#define write8(val)  *(unsigned char *)x86Ptr = val; x86Ptr++;
#define write16(val) *(unsigned short*)x86Ptr = val; x86Ptr+=2;
#define write32(val) *(u32 *)x86Ptr = val; x86Ptr+=4;

/* Transfer Instructions */

/* mov Rm,Rn */
void MOV(int m, int n) {
	write16(0x6003 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* mov #imm, Rn */
void MOVI(int i, int n) {
	write16(0xe000 | ((n&0xf)<<8) | (i&0xff));
}

/* mov.l @(disp,PC),Rn */
u8* MOVLI(int d, int n) {
	write16(0xd000 | ((n&0xf)<<8) | (d&0xff));
	return x86Ptr - 2;
}

/* mov.l @(disp,GBR),R0 */
u8* MOVLLG(int d) {
	write16(0xc600 | (d&0xff));
	return x86Ptr - 2;
}

/* mov.l R0,@(disp,GBR) */
u8* MOVLSG(int d) {
	write16(0xc200 | (d&0xff));
	return x86Ptr - 2;
}

/* mov.l @Rm,Rn */
void MOVLL(int m, int n) {
	write16(0x6002 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* mov.l Rm,@Rn */
void MOVLS(int m, int n) {
	write16(0x2002 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* mov.l Rm,@-Rn */
void MOVLM(int m, int n) {
	write16(0x2006 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* mov.l @Rm+,Rn */
void MOVLP(int m, int n) {
	write16(0x6006 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* movt Rn */
void MOVT(int n) {
	write16(0x0029 | ((n&0xf)<<8));
}

/* swap.w Rm,Rn */
void SWAPW(int m, int n) {
	write16(0x6009 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* Branch instructions */

/* bra disp */
u16* BRA(int disp) {
	write16(0xa000 | (disp&0x0fff)); 
	return (u16*)(x86Ptr - 2);
}

/* bt disp */
u8* BT(int disp) {
	write16(0x8900 | (disp&0x00ff)); 
	return x86Ptr - 2;
}

/* bf disp */
u8* BF(int disp) {
	write16(0x8b00 | (disp&0x00ff)); 
	return x86Ptr - 2;
}

/* jsr @Rn */
void JSR(int n) {
	write16(0x400b | ((n&0xf)<<8)); 
}

/* rts */
void RET() {
	write16(0x000b); 
}

/* Logical Instructions */

/* and Rm, Rn */
void AND(int m, int n) {
	write16(0x2009 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* and #imm, R0 */
void ANDI(int i) {
	write16(0xc900 | (i&0xff));
}

/* xor Rm, Rn */
void XOR(int m, int n) {
	write16(0x200a | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* xor #imm, R0 */
void XORI(int i) {
	write16(0xca00 | (i&0xff));
}

/* or Rm, Rn */
void OR(int m, int n) {
	write16(0x200b | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* not Rm, Rn */
void NOT(int m, int n) {
	write16(0x6007 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* tst Rm, Rn */
void TST(int m, int n) {
	write16(0x2008 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* Arithmetic Instructions */

/* add Rm, Rn */
void ADD(int m, int n) {
	write16(0x300c | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* sub Rm, Rn */
void SUB(int m, int n) {
	write16(0x3008 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* neg Rm, Rn */
void NEG(int m, int n) {
	write16(0x600b | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* cmp/eq Rm, Rn */
void CMPEQ(int m, int n) {
	write16(0x3000 | ((n&0xf)<<8) | ((m&0xf)<<4));
}	

/* cmp/gt Rm, Rn */
void CMPGT(int m, int n) {
	write16(0x3007 | ((n&0xf)<<8) | ((m&0xf)<<4));
}	

/* cmp/ge Rm, Rn */
void CMPGE(int m, int n) {
	write16(0x3003 | ((n&0xf)<<8) | ((m&0xf)<<4));
}	

/* cmp/hi Rm, Rn */
void CMPHI(int m, int n) {
	write16(0x3006 | ((n&0xf)<<8) | ((m&0xf)<<4));
}	

/* mul.l Rm, Rn */
void MULL(int m, int n) {
	write16(0x0007 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* dmuls.l Rm, Rn */
void DMULS(int m, int n) {
	write16(0x300d | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* dmulu.l Rm, Rn */
void DMULU(int m, int n) {
	write16(0x3005 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* exts.b Rm, Rn */
void EXTSB(int m, int n) {
	write16(0x600e | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* exts.w Rm, Rn */
void EXTSW(int m, int n) {
	write16(0x600f | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* extu.b Rm, Rn */
void EXTUB(int m, int n) {
	write16(0x600c | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* extu.w Rm, Rn */
void EXTUW(int m, int n) {
	write16(0x600d | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* Shift Instructions */

/* shld Rm, Rn */
void SHLD(int m, int n) {
	write16(0x400d | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* shad Rm, Rn */
void SHAD(int m, int n) {
	write16(0x400c | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* shlr2 Rm, Rn */
void SHLR2(int n) {
	write16(0x4009 | ((n&0xf)<<8));
}

/* shlr16 Rm, Rn */
void SHLR16(int n) {
	write16(0x4029 | ((n&0xf)<<8));
}

/* shll8 Rm, Rn */
void SHLL8(int n) {
	write16(0x4018 | ((n&0xf)<<8));
}

/* System Control Instructions */

/* stc.l gbr,@-Rn */
void STCMGBR(int n) {
	write16(0x4013 | ((n&0xf)<<8));
}

/* ldc.l @Rm+,gbr */
void LDCMGBR(int m) {
	write16(0x4017 | ((m&0xf)<<8));
}

/* ldc Rm,gbr */
void LDCGBR(int m) {
	write16(0x401e | ((m&0xf)<<8));
}

/* sts.l pr,@-Rn */
void STSMPR(int n) {
	write16(0x4022 | ((n&0xf)<<8));
}

/* lds.l @Rm+,pr */
void LDSMPR(int m) {
	write16(0x4026 | ((m&0xf)<<8));
}

/* sts.l macl,Rn */
void STSMACL(int n) {
	write16(0x001a | ((n&0xf)<<8));
}

/* sts.l macH,Rn */
void STSMACH(int n) {
	write16(0x000a | ((n&0xf)<<8));
}

/* nop */
void NOP() {
	write16(0x0009); 
}

/* Floating-Point Instructions */

/* fmov.s FRm, @Rn */
void FMOV_STORE(int m, int n) {
	write16(0xf00a | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* float fpul, FRn */
void FLOAT(int n) {
	write16(0xf02d | ((n&0xf)<<8));
}

/* fdiv FRm, FRn */
void FDIV(int m, int n) {
	write16(0xf003 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* fmul FRm, FRn */
void FMUL(int m, int n) {
	write16(0xf002 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* ftrc FRm, fpul */
void FTRC(int m) {
	write16(0xf03d | ((m&0xf)<<8));
}

/* fmov.s FRm, @-Rn */
void FMOV_SAVE(int m, int n) {
	write16(0xf00b | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* fmov.s @Rm+, FRn */
void FMOV_RESTORE(int m, int n) {
	write16(0xf009 | ((n&0xf)<<8) | ((m&0xf)<<4));
}

/* Floating-Point Control Instructions */

/* lds Rm,fpscr */
void LDSFPSCR(int m) {
	write16(0x406a | ((m&0xf)<<8));
}

/* lds Rm,fpul */
void LDSFPUL(int m) {
	write16(0x405a | ((m&0xf)<<8));
}

/* sts.l fpul,Rn */
void STSFPUL(int n) {
	write16(0x005a | ((n&0xf)<<8));
}

/* sts.l fpscr,Rn */
void STSFPSCR(int n) {
	write16(0x006a | ((n&0xf)<<8));
}

/* call func */
void CALLFunc(u32 func) {
	LoadImmediate32(func, R1);
	JSR(R1);
	NOP();
	LoadImmediate32((u32)&psxRegs, R1);
	LDCGBR(R1);	// Load GBR with addr of psxRegs
}

/* memwrite and memread addr should be stored in reg */
void LoadImmediate32(u32 imm, u32 reg) {
	if (imm < 0x7f)
		MOVI(imm, reg);
	else if (imm >= 0xffffff80)
		MOVI(imm, reg);
	else {
		immPtr[immCount] = MOVLI(0, reg);
		immData[immCount] = imm;
//		printf("LoadImmediate32: %08x : %08x\n", immPtr[immCount], immData[immCount]);
		immCount++;
		if (immCount > 253)
			printf("*************** LoadImmediate32 Overflow ***************\n");
	}
}

void UpdateImmediate32(u32 force) {
	int i;
	u16 *bp;
	
	if (!immCount) return;
	if (!force)
		if (((u32)x86Ptr - (u32)immPtr[0]) < 768) return;
	
	bp = BRA(0);
	NOP();
	
	Align4();
//	printf("UpdateImmediate32: force %d with count %d\n", force, immCount);
	for (i = 0; i < immCount; i++) {
//		printf("UpdateImmediate32: %08x : %08x\n", (u32)immPtr[i], immData[i]);
		shSetImm(immPtr[i]);
		write32(immData[i]);	
	}
	shSetBRA(bp);
	immCount = 0;
}
 