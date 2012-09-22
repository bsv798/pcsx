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

#ifndef __SH4_H__
#define __SH4_H__

// Basic types
#include "PsxCommon.h"

/* general defines */

s8  *x86Ptr;
u8  *j8Ptr[32];
u16 *j16Ptr[32];
u8  *immPtr[256];
u32  immData[256];     // immediate values
u32  immCount;		// Counts the number of immediate values

float rMatrix[16] __attribute__ ((aligned (32)));	// keeps the rotation matrix
float lMatrix[16] __attribute__ ((aligned (32)));	// keeps the light matrix
float cMatrix[16] __attribute__ ((aligned (32)));	// keeps the color matrix
float trVector[4] __attribute__ ((aligned (32)));	// translation vector
float bkVector[4] __attribute__ ((aligned (32)));	// background vector
float fcVector[4] __attribute__ ((aligned (32)));	// far color vector
float control[7]  __attribute__ ((aligned (32)));	// rest of the control register

float ir[4]  __attribute__ ((aligned (32)));	// ir vector (clipped)
float mac[4] __attribute__ ((aligned (32)));	// mac vector

void x86Init(char *ptr);
void x86Shutdown();
void shSetBx(u8 *j8);
void shSetBRA(u16 *j16);
void Align4();
void Align32();

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R15 15

#define FR0 0
#define FR1 1
#define FR2 2
#define FR3 3

/*******************/
/* SH4 intructions */
/*******************/

/* Transfer Instructions */

/* mov Rm,Rn */
void MOV(int m, int n);
/* mov #imm, Rn */
void MOVI(int i, int n);
/* mov.l @(disp,PC),Rn */
u8* MOVLI(int d, int n);
/* mov.l @Rm,Rn */
void MOVLL(int m, int n);
/* mov.l @(disp,GBR),R0 */
u8* MOVLLG(int d);
/* mov.l R0,@(disp,GBR) */
u8* MOVLSG(int d);
/* mov.l Rm,@Rn */
void MOVLS(int m, int n);
/* mov.l Rm,@-Rn */
void MOVLM(int m, int n);
/* mov.l @Rm+,Rn */
void MOVLP(int m, int n);
/* movt Rn */
void MOVT(int n);
/* swap.w Rm,Rn */
void SWAPW(int m, int n);

/* Branch instructions */

/* bra disp */
u16* BRA(int disp);
/* bt disp */
u8* BT(int disp);
/* bf disp */
u8* BF(int disp);
/* jsr @Rn */
void JSR(int n);
/* ret */
void RET();

/* Logical Instructions */

/* and Rm, Rn */
void AND(int m, int n);
/* and #imm, R0 */
void ANDI(int i);
/* xor Rm, Rn */
void XOR(int m, int n);
/* xor #imm, R0 */
void XORI(int i);
/* or Rm, Rn */
void OR(int m, int n);
/* not Rm, Rn */
void NOT(int m, int n);
/* tst Rm, Rn */
void TST(int m, int n);

/* Arithmetic Instructions */

/* add Rm, Rn */
void ADD(int m, int n);
/* sub Rm, Rn */
void SUB(int m, int n);
/* neg Rm, Rn */
void NEG(int m, int n);
/* cmp/eq Rm, Rn */
void CMPEQ(int m, int n);
/* cmp/gt Rm, Rn */
void CMPGT(int m, int n);
/* cmp/ge Rm, Rn */
void CMPGE(int m, int n);
/* cmp/hi Rm, Rn */
void CMPHI(int m, int n);
/* mul.l Rm, Rn */
void MULL(int m, int n);
/* dmuls.l Rm, Rn */
void DMULS(int m, int n);
/* dmulu.l Rm, Rn */
void DMULU(int m, int n);
/* exts.b Rm, Rn */
void EXTSB(int m, int n);
/* exts.w Rm, Rn */
void EXTSW(int m, int n);
/* extu.b Rm, Rn */
void EXTUB(int m, int n);
/* extu.w Rm, Rn */
void EXTUW(int m, int n);

/* Shift Instructions */

/* shld Rm, Rn */
void SHLD(int m, int n);
/* shad Rm, Rn */
void SHAD(int m, int n);
/* shlr2 Rm, Rn */
void SHLR2(int n);
/* shll8 Rm, Rn */
void SHLL8(int n);
/* shlr16 Rm, Rn */
void SHLR16(int n);

/* System Control Instructions */

/* stc.l gbr,@-Rn */
void STCMGBR(int n);
/* ldc.l @Rm+,gbr */
void LDCMGBR(int m);
/* ldc Rm,gbr */
void LDCGBR(int m);
/* sts.l pr,@-Rn */
void STSMPR(int n);
/* lds.l @Rm+,pr */
void LDSMPR(int m);
/* sts.l macl,Rn */
void STSMACL(int n);
/* sts.l mach,Rn */
void STSMACH(int n);
/* nop */
void NOP();

/* Floating-Point Instructions */

/* fmov.s FRm, @Rn */
void FMOV_STORE(int m, int n);
/* float fpul, FRn */
void FLOAT(int n);
/* fdiv FRm, FRn */
void FDIV(int m, int n);
/* fmul FRm, FRn */
void FMUL(int m, int n);
/* ftrc FRm, fpul */
void FTRC(int m);
/* fmov.s FRm, @-Rn */
void FMOV_SAVE(int m, int n);
/* fmov.s @Rm+, FRn */
void FMOV_RESTORE(int m, int n);

/* Floating-Point Control Instructions */

/* lds Rm,fpscr */
void LDSFPSCR(int m);
/* lds Rm,fpul */
void LDSFPUL(int m);
/* sts.l fpul,Rn */
void STSFPUL(int n);
/* sts.l fpscr,Rn */
void STSFPSCR(int n);

/* call func */
void CALLFunc(u32 func);

void LoadImmediate32(u32 imm, u32 reg);
void UpdateImmediate32(u32 force);

void LoadMatrixElement(u32 r4, u32 r5, u32 r6, u32 r7);
u32  asmTransformVector(u32 ir, u32 mac, u32 clip);
u32  asmColorCalc(float *ir_t, float *mac_t, u32 lim);
void asmStore32(float *vec, u32 *addr);
void asmLoad16 (s32 *addr, float *vec);
void asmLoadVec(s16 *addr, float *vec);
void asmNCLIP(u32 data);
void mat_vec_load(float *matrix, float *vector);
void mat_zero_load(float *matrix);

#endif /* __SH4_H__ */
