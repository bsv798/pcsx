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
#include <stdlib.h>
#include <math.h>
#include "Gte.h"
#include "R3000A.h"

#include "sh4/sh4.h"

#define GTE_DUMP
#ifdef GTE_DUMP
#define G_OP(name,delay) printf("* : %08X : %02d : %s\n", psxRegs.code, delay, name);
#define G_SD(reg)  printf("+D%02d : %08X\n", reg, psxRegs.CP2D.r[reg]);
#define G_SC(reg)  printf("+C%02d : %08X\n", reg, psxRegs.CP2C.r[reg]);
#define G_GD(reg)  printf("-D%02d : %08X\n", reg, psxRegs.CP2D.r[reg]);
#define G_GC(reg)  printf("-C%02d : %08X\n", reg, psxRegs.CP2C.r[reg]);
#else
#define G_OP(name,delay)
#define G_SD(reg)
#define G_SC(reg)
#define G_GD(reg)
#define G_GC(reg)
#endif
#undef GTE_DUMP

#define SUM_FLAG if(gteFLAG & 0x7F87E000) gteFLAG |= 0x80000000;

#ifdef __WIN32__
#pragma warning(disable:4244)
#endif

#define gteVX0     ((s16*)psxRegs.CP2D.r)[0]
#define gteVY0     ((s16*)psxRegs.CP2D.r)[1]
#define gteVZ0     ((s16*)psxRegs.CP2D.r)[2]
#define gteVX1     ((s16*)psxRegs.CP2D.r)[4]
#define gteVY1     ((s16*)psxRegs.CP2D.r)[5]
#define gteVZ1     ((s16*)psxRegs.CP2D.r)[6]
#define gteVX2     ((s16*)psxRegs.CP2D.r)[8]
#define gteVY2     ((s16*)psxRegs.CP2D.r)[9]
#define gteVZ2     ((s16*)psxRegs.CP2D.r)[10]
#define gteRGB     psxRegs.CP2D.r[6]
#define gteOTZ     ((s16*)psxRegs.CP2D.r)[7*2]
#define gteIR0     ((s32*)psxRegs.CP2D.r)[8]
#define gteIR1     ((s32*)psxRegs.CP2D.r)[9]
#define gteIR2     ((s32*)psxRegs.CP2D.r)[10]
#define gteIR3     ((s32*)psxRegs.CP2D.r)[11]
#define gteSX0     ((s16*)psxRegs.CP2D.r)[12*2]
#define gteSY0     ((s16*)psxRegs.CP2D.r)[12*2+1]
#define gteSX1     ((s16*)psxRegs.CP2D.r)[13*2]
#define gteSY1     ((s16*)psxRegs.CP2D.r)[13*2+1]
#define gteSX2     ((s16*)psxRegs.CP2D.r)[14*2]
#define gteSY2     ((s16*)psxRegs.CP2D.r)[14*2+1]
#define gteSXP     ((s16*)psxRegs.CP2D.r)[15*2]
#define gteSYP     ((s16*)psxRegs.CP2D.r)[15*2+1]
#define gteSZx     ((u16*)psxRegs.CP2D.r)[16*2]
#define gteSZ0     ((u16*)psxRegs.CP2D.r)[17*2]
#define gteSZ1     ((u16*)psxRegs.CP2D.r)[18*2]
#define gteSZ2     ((u16*)psxRegs.CP2D.r)[19*2]
#define gteRGB0    psxRegs.CP2D.r[20]
#define gteRGB1    psxRegs.CP2D.r[21]
#define gteRGB2    psxRegs.CP2D.r[22]
#define gteMAC0    psxRegs.CP2D.r[24]
#define gteMAC1    ((s32*)psxRegs.CP2D.r)[25]
#define gteMAC2    ((s32*)psxRegs.CP2D.r)[26]
#define gteMAC3    ((s32*)psxRegs.CP2D.r)[27]
#define gteIRGB    psxRegs.CP2D.r[28]
#define gteORGB    psxRegs.CP2D.r[29]
#define gteLZCS    psxRegs.CP2D.r[30]
#define gteLZCR    psxRegs.CP2D.r[31]

#define gteR       ((u8 *)psxRegs.CP2D.r)[6*4]
#define gteG       ((u8 *)psxRegs.CP2D.r)[6*4+1]
#define gteB       ((u8 *)psxRegs.CP2D.r)[6*4+2]
#define gteCODE    ((u8 *)psxRegs.CP2D.r)[6*4+3]
#define gteC       gteCODE

#define gteR0      ((u8 *)psxRegs.CP2D.r)[20*4]
#define gteG0      ((u8 *)psxRegs.CP2D.r)[20*4+1]
#define gteB0      ((u8 *)psxRegs.CP2D.r)[20*4+2]
#define gteCODE0   ((u8 *)psxRegs.CP2D.r)[20*4+3]
#define gteC0      gteCODE0

#define gteR1      ((u8 *)psxRegs.CP2D.r)[21*4]
#define gteG1      ((u8 *)psxRegs.CP2D.r)[21*4+1]
#define gteB1      ((u8 *)psxRegs.CP2D.r)[21*4+2]
#define gteCODE1   ((u8 *)psxRegs.CP2D.r)[21*4+3]
#define gteC1      gteCODE1

#define gteR2      ((u8 *)psxRegs.CP2D.r)[22*4]
#define gteG2      ((u8 *)psxRegs.CP2D.r)[22*4+1]
#define gteB2      ((u8 *)psxRegs.CP2D.r)[22*4+2]
#define gteCODE2   ((u8 *)psxRegs.CP2D.r)[22*4+3]
#define gteC2      gteCODE2



#define gteR11  ((s16*)psxRegs.CP2C.r)[0]
#define gteR12  ((s16*)psxRegs.CP2C.r)[1]
#define gteR13  ((s16*)psxRegs.CP2C.r)[2]
#define gteR21  ((s16*)psxRegs.CP2C.r)[3]
#define gteR22  ((s16*)psxRegs.CP2C.r)[4]
#define gteR23  ((s16*)psxRegs.CP2C.r)[5]
#define gteR31  ((s16*)psxRegs.CP2C.r)[6]
#define gteR32  ((s16*)psxRegs.CP2C.r)[7]
#define gteR33  ((s16*)psxRegs.CP2C.r)[8]
#define gteTRX  ((s32*)psxRegs.CP2C.r)[5]
#define gteTRY  ((s32*)psxRegs.CP2C.r)[6]
#define gteTRZ  ((s32*)psxRegs.CP2C.r)[7]
#define gteL11  ((s16*)psxRegs.CP2C.r)[16]
#define gteL12  ((s16*)psxRegs.CP2C.r)[17]
#define gteL13  ((s16*)psxRegs.CP2C.r)[18]
#define gteL21  ((s16*)psxRegs.CP2C.r)[19]
#define gteL22  ((s16*)psxRegs.CP2C.r)[20]
#define gteL23  ((s16*)psxRegs.CP2C.r)[21]
#define gteL31  ((s16*)psxRegs.CP2C.r)[22]
#define gteL32  ((s16*)psxRegs.CP2C.r)[23]
#define gteL33  ((s16*)psxRegs.CP2C.r)[24]
#define gteRBK  ((s32*)psxRegs.CP2C.r)[13]
#define gteGBK  ((s32*)psxRegs.CP2C.r)[14]
#define gteBBK  ((s32*)psxRegs.CP2C.r)[15]
#define gteLR1  ((s16*)psxRegs.CP2C.r)[32]
#define gteLR2  ((s16*)psxRegs.CP2C.r)[33]
#define gteLR3  ((s16*)psxRegs.CP2C.r)[34]
#define gteLG1  ((s16*)psxRegs.CP2C.r)[35]
#define gteLG2  ((s16*)psxRegs.CP2C.r)[36]
#define gteLG3  ((s16*)psxRegs.CP2C.r)[37]
#define gteLB1  ((s16*)psxRegs.CP2C.r)[38]
#define gteLB2  ((s16*)psxRegs.CP2C.r)[39]
#define gteLB3  ((s16*)psxRegs.CP2C.r)[40]
#define gteRFC  ((s32*)psxRegs.CP2C.r)[21]
#define gteGFC  ((s32*)psxRegs.CP2C.r)[22]
#define gteBFC  ((s32*)psxRegs.CP2C.r)[23]
#define gteOFX  ((s32*)psxRegs.CP2C.r)[24]
#define gteOFY  ((s32*)psxRegs.CP2C.r)[25]
#define gteH    ((u16*)psxRegs.CP2C.r)[52]
#define gteDQA  ((s16*)psxRegs.CP2C.r)[54]
#define gteDQB  ((s32*)psxRegs.CP2C.r)[28]
#define gteZSF3 ((s16*)psxRegs.CP2C.r)[58]
#define gteZSF4 ((s16*)psxRegs.CP2C.r)[60]
#define gteFLAG psxRegs.CP2C.r[31]

__inline unsigned long MFC2(int reg) {
	switch(reg) {
		case 29:
			gteORGB = (((gteIR1 >> 7) & 0x1f)) |
					  (((gteIR2 >> 7) & 0x1f)<<5) |
					  (((gteIR3 >> 7) & 0x1f)<<10);
			return gteORGB;

		default:
			return psxRegs.CP2D.r[reg];
	}
}

__inline void MTC2(unsigned long value, int reg) {
	int a;

	switch(reg) {
		case 8: case 9: case 10: case 11:
			psxRegs.CP2D.r[reg] = (short)value;
			break;

		case 15:
			psxRegs.CP2D.r[12] = psxRegs.CP2D.r[13];
			psxRegs.CP2D.r[13] = psxRegs.CP2D.r[14];
			psxRegs.CP2D.r[14] = value;
			psxRegs.CP2D.r[15] = value;
			break;

		case 16: case 17: case 18: case 19:
			psxRegs.CP2D.r[reg] = (value & 0xffff);
			break;

		case 28:
			psxRegs.CP2D.r[28] = value;
			gteIR1 = ((value      ) & 0x1f) << 7;
			gteIR2 = ((value >>  5) & 0x1f) << 7;
			gteIR3 = ((value >> 10) & 0x1f) << 7;
			break;

		case 30:
			psxRegs.CP2D.r[30] = value;

			a = psxRegs.CP2D.r[30];
#if defined(__WIN32__)
			if (a > 0) {
				__asm {
					mov eax, a;
					bsr eax, eax;
					mov a, eax;
				}
				psxRegs.CP2D.r[31] = 31 - a;
			} else if (a < 0) {
				__asm {
					mov eax, a;
					xor eax, 0xffffffff;
					bsr eax, eax;
					mov a, eax;
				}
				psxRegs.CP2D.r[31] = 31 - a;
			} else {
				psxRegs.CP2D.r[31] = 32;
			}
#elif defined(__LINUX__)
			if (a > 0) {
				__asm__ ("bsrl %1, %0\n" : "=r"(a) : "r"(a) );
				psxRegs.CP2D.r[31] = 31 - a;
			} else if (a < 0) {
				a^= 0xffffffff;
				__asm__ ("bsrl %1, %0\n" : "=r"(a) : "r"(a) );
				psxRegs.CP2D.r[31] = 31 - a;
			} else {
				psxRegs.CP2D.r[31] = 32;
			}
#else
			if (a > 0) {
				int i;
				for (i=31; (a & (1 << i)) == 0 && i >= 0; i--);
				psxRegs.CP2D.r[31] = 31 - i;
			} else if (a < 0) {
				int i;
				a^= 0xffffffff;
				for (i=31; (a & (1 << i)) == 0 && i >= 0; i--);
				psxRegs.CP2D.r[31] = 31 - i;
			} else {
				psxRegs.CP2D.r[31] = 32;
			}
#endif
			break;
		
		default:
			psxRegs.CP2D.r[reg] = value;
	}
}

void gteMFC2() {
	if (!_Rt_) return;
	psxRegs.GPR.r[_Rt_] = MFC2(_Rd_);
}

void gteCFC2() {
	if (!_Rt_) return;
	psxRegs.GPR.r[_Rt_] = psxRegs.CP2C.r[_Rd_];
}

void gteMTC2() {
	MTC2(psxRegs.GPR.r[_Rt_], _Rd_);
}

void gteCTC2() {
	psxRegs.CP2C.r[_Rd_] = psxRegs.GPR.r[_Rt_];
}

#define _oB_ (psxRegs.GPR.r[_Rs_] + _Imm_)

void gteLWC2() {
	MTC2(psxMemRead32(_oB_), _Rt_);
}

void gteSWC2() {
	psxMemWrite32(_oB_, MFC2(_Rt_));
}

/////LIMITATIONS AND OTHER STUFF************************************
#define MAC2IR() \
{ \
	if ((s32)gteMAC1 < -32768) { gteIR1=(long)(-32768); gteFLAG|=1<<24;} \
	else \
	if ((s32)gteMAC1 >  32767) { gteIR1=(long)( 32767); gteFLAG|=1<<24;} \
	else gteIR1=(long)gteMAC1; \
	if ((s32)gteMAC2 < -32768) { gteIR2=(long)(-32768); gteFLAG|=1<<23;} \
	else \
	if ((s32)gteMAC2 >  32767) { gteIR2=(long)( 32767); gteFLAG|=1<<23;} \
	else gteIR2=(long)gteMAC2; \
	if ((s32)gteMAC3 < -32768) { gteIR3=(long)(-32768); gteFLAG|=1<<22;} \
	else \
	if ((s32)gteMAC3 >  32767) { gteIR3=(long)( 32767); gteFLAG|=1<<22;} \
	else gteIR3=(long)gteMAC3; \
}


#define MAC2IR1() \
{           \
	if ((s32)gteMAC1 < 0) { gteIR1=(long)0; gteFLAG|=1<<24;}  \
	else if ((s32)gteMAC1 > 32767) { gteIR1=(long)(32767); gteFLAG|=1<<24;} \
	else gteIR1=(long)gteMAC1;                                                         \
	if ((s32)gteMAC2 < 0) { gteIR2=(long)0; gteFLAG|=1<<23;}      \
	else if ((s32)gteMAC2 > 32767) { gteIR2=(long)(32767); gteFLAG|=1<<23;}    \
	else gteIR2=(long)gteMAC2;                                                            \
	if ((s32)gteMAC3 < 0) { gteIR3=(long)0; gteFLAG|=1<<22;}         \
	else if ((s32)gteMAC3 > 32767) { gteIR3=(long)(32767); gteFLAG|=1<<22;}       \
	else gteIR3=(long)gteMAC3; \
}

s32 limC  (s32 x) {

	if (x <     0) { x =     0; gteFLAG |= (1<<18); } else
	if (x > 65535) { x = 65535; gteFLAG |= (1<<18); } return (x);
}

double limD1 (double x) {

	if (x <  1024.0) { x =  1024.0; gteFLAG |= (1<<14); } else
	if (x >  1023.0) { x =  1023.0; gteFLAG |= (1<<14); } return (x);
}

double limD2 (double x) {

	if (x <  1024.0) { x =  1024.0; gteFLAG |= (1<<13); } else
	if (x >  1023.0) { x =  1023.0; gteFLAG |= (1<<13); } return (x);
}

s32 limE  (s32 x) {

	if (x <     0) { x =     0; gteFLAG |= (1<<12); } else
	if (x >  4095) { x =  4095; gteFLAG |= (1<<12); } return (x);
}

double limG1(double x) {
	
	if (x > 2147483647.0f) { gteFLAG |= (1<<16); } else
	if (x <-2147483648.0f) { gteFLAG |= (1<<15); }

	if (x >       1023.0f) { x =  1023.0f; gteFLAG |= (1<<14); } else
	if (x <      -1024.0f) { x = -1024.0f; gteFLAG |= (1<<14); } return (x);
}

double limG2(double x) {
	
	if (x > 2147483647.0f) { gteFLAG |= (1<<16); } else
	if (x <-2147483648.0f) { gteFLAG |= (1<<15); }

	if (x >       1023.0f) { x =  1023.0f; gteFLAG |= (1<<13); } else
	if (x <      -1024.0f) { x = -1024.0f; gteFLAG |= (1<<13); } return (x);
}

void limitB_z(float *v) {
	if (v[0] < 0.0) { v[0] = 0.0; gteFLAG |= 1<<24; } else
	if (v[0] > 32767.0) { v[0] = 32767.0; gteFLAG |= 1<<24; }
	
	if (v[1] < 0.0) { v[1] = 0.0; gteFLAG |= 1<<23; } else
	if (v[1] > 32767.0) { v[1] = 32767.0; gteFLAG |= 1<<23; }
		
	if (v[2] < 0.0) { v[2] = 0.0; gteFLAG |= 1<<22; } else
	if (v[2] > 32767.0) { v[2] = 32767.0; gteFLAG |= 1<<22; }
}

void limitB_m(float *v) {
	if (v[0] < -32768.0) { v[0] = -32768.0; gteFLAG |= 1<<24; } else
	if (v[0] > 32767.0) { v[0] = 32767.0; gteFLAG |= 1<<24; }
	
	if (v[1] < -32768.0) { v[1] = -32768.0; gteFLAG |= 1<<23; } else
	if (v[1] > 32767.0) { v[1] = 32767.0; gteFLAG |= 1<<23; }
		
	if (v[2] < -32768.0) { v[2] = -32768.0; gteFLAG |= 1<<22; } else
	if (v[2] > 32767.0) { v[2] = 32767.0; gteFLAG |= 1<<22; }
}

void UpdateColor()
{
	s32 x;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	x = gteMAC1 >> 4;
	if (x <   0) { x =   0; gteFLAG |= (1<<21); } else
	if (x > 255) { x = 255; gteFLAG |= (1<<21); }
	gteR2 = x;
	x = gteMAC2 >> 4;
	if (x <   0) { x =   0; gteFLAG |= (1<<20); } else
	if (x > 255) { x = 255; gteFLAG |= (1<<20); }
	gteG2 = x;
	x = gteMAC3 >> 4;
	if (x <   0) { x =   0; gteFLAG |= (1<<19); } else
	if (x > 255) { x = 255; gteFLAG |= (1<<19); }
	gteB2 = x;
	gteCODE2 = gteCODE;
}	

void cFarBackColor(float *ir_t, float *mac_t, u32 lim, float r, float g, float b)
{
	float tv[3];
	tv[0] = fcVector[0] - r;
	tv[1] = fcVector[1] - g;
	tv[2] = fcVector[2] - b;

	limitB_m(tv);

	ir_t[0] = mac_t[0] = r + ((ir_t[3] * tv[0])/4096.0);
	ir_t[1] = mac_t[1] = g + ((ir_t[3] * tv[1])/4096.0);
	ir_t[2] = mac_t[2] = b + ((ir_t[3] * tv[2])/4096.0);

	if (lim) limitB_z(ir_t);
	else limitB_m(ir_t);
}

//********END OF LIMITATIONS**********************************/

void gteRTPS() {
	double DSZ;

#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_RTPS\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("RTPS", 14);
		G_SD(0);
		G_SD(1);

		G_SD(16); // Store original fifo
		G_SD(17);
		G_SD(18);
		G_SD(19);

		G_SC(0);
		G_SC(1);
		G_SC(2);
		G_SC(3);
		G_SC(4);
		G_SC(5);
		G_SC(6);
		G_SC(7);

		G_SC(24);
		G_SC(25);
		G_SC(26);
		G_SC(27);
		G_SC(28);
	}
#endif

	asmLoadVec(&gteVX0, ir); ir[3] = 4096.0; mat_vec_load(rMatrix, trVector);
	gteFLAG = asmTransformVector((u32)ir, (u32)mac, 0);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);

	gteSZx = gteSZ0;
	gteSZ0 = gteSZ1;
	gteSZ1 = gteSZ2;
	gteSZ2 = limC(gteMAC3);
	
    psxRegs.CP2D.r[12]= psxRegs.CP2D.r[13];
	psxRegs.CP2D.r[13]= psxRegs.CP2D.r[14];

	if (gteSZ2 == 0) {
		DSZ = 2*65536.0; gteFLAG |= 1<<17;
	} else {
		DSZ = control[2] / gteSZ2;
		if (DSZ > 2.0) { DSZ = 2.0f; gteFLAG |= 1<<17; }
		DSZ *= 65536.0;
	}
	
	gteSX2 = limG1((control[0] + (ir[0] * DSZ))/65536.0);
	gteSY2 = limG2((control[1] + (ir[1] * DSZ))/65536.0);
	gteSXP = gteSX2; gteSYP = gteSY2;
	
	gteMAC0 = (control[4] + control[3] * DSZ);
	gteIR0  = limE((s32)gteMAC0>>12);

	SUM_FLAG;

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(8);
		G_GD(9);
		G_GD(10);
		G_GD(11);
		
		//G_GD(12);
		//G_GD(13);
		G_GD(14);

		G_GD(16);
		G_GD(17);
		G_GD(18);
		G_GD(19);

		G_GD(24);
		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteRTPT() {
    double DSZ;

#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_RTPT\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("RTPT", 22);
		G_SD(0);
		G_SD(1);
		G_SD(2);
		G_SD(3);
		G_SD(4);
		G_SD(5);

		G_SD(16); // Store original fifo
		G_SD(17);
		G_SD(18);
		G_SD(19);

		G_SC(0);
		G_SC(1);
		G_SC(2);
		G_SC(3);
		G_SC(4);
		G_SC(5);
		G_SC(6);
		G_SC(7);

		G_SC(24);
		G_SC(25);
		G_SC(26);
		G_SC(27);
		G_SC(28);
	}
#endif

	gteFLAG = 0;

	mat_vec_load(rMatrix, trVector);

	gteSZx = gteSZ2;

// VX0

	asmLoadVec(&gteVX0, ir); ir[3] = 4096.0;
	gteFLAG = asmTransformVector((u32)ir, (u32)mac, 0);

	gteSZ0 = limC((s32)mac[2]);

	if (gteSZ0 == 0) {
		DSZ = 2*65536.0; gteFLAG |= 1<<17;
	} else {
		DSZ = control[2] / gteSZ0;
		if (DSZ > 2.0) { DSZ = 2.0f; gteFLAG |= 1<<17; }
		DSZ *= 65536.0;
	}
	
	gteSX0 = limG1((control[0] + (ir[0] * DSZ))/65536.0);
	gteSY0 = limG2((control[1] + (ir[1] * DSZ))/65536.0);

// VX1

	asmLoadVec(&gteVX1, ir); ir[3] = 4096.0;
	gteFLAG |= asmTransformVector((u32)ir, (u32)mac, 0);

	gteSZ1 = limC((s32)mac[2]);

	if (gteSZ1 == 0) {
		DSZ = 2*65536.0; gteFLAG |= 1<<17;
	} else {
		DSZ = control[2] / gteSZ1;
		if (DSZ > 2.0) { DSZ = 2.0f; gteFLAG |= 1<<17; }
		DSZ *= 65536.0;
	}
	
	gteSX1 = limG1((control[0] + (ir[0] * DSZ))/65536.0);
	gteSY1 = limG2((control[1] + (ir[1] * DSZ))/65536.0);

// VX2

	asmLoadVec(&gteVX2, ir); ir[3] = 4096.0;
	gteFLAG |= asmTransformVector((u32)ir, (u32)mac, 0);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);

	gteSZ2 = limC((s32)mac[2]);

	if (gteSZ2 == 0) {
		DSZ = 2*65536.0; gteFLAG |= 1<<17;
	} else {
		DSZ = control[2] / gteSZ2;
		if (DSZ > 2.0) { DSZ = 2.0f; gteFLAG |= 1<<17; }
		DSZ *= 65536.0;
	}
	
	gteSX2 = limG1((control[0] + (ir[0] * DSZ))/65536.0);
	gteSY2 = limG2((control[1] + (ir[1] * DSZ))/65536.0);
	gteSXP = gteSX2; gteSYP = gteSY2;

	gteMAC0 = (control[4] + control[3] * DSZ);
	gteIR0  = limE((s32)gteMAC0>>12);

	SUM_FLAG;

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(8);
		G_GD(9);
		G_GD(10);
		G_GD(11);
		
		G_GD(12);
		G_GD(13);
		G_GD(14);

		G_GD(16);
		G_GD(17);
		G_GD(18);
		G_GD(19);

		G_GD(24);
		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteMVMVA() {
#ifdef GTE_LOG
	GTE_LOG("GTE_MVMVA %lx\n", psxRegs.code & 0x1ffffff);
#endif

	switch (psxRegs.code & 0x18000) {
		case 0x00000:
			asmLoadVec(&gteVX0, ir);
			break;
		case 0x08000:
			asmLoadVec(&gteVX1, ir);
			break;
		case 0x10000:
			asmLoadVec(&gteVX2, ir);
			break;
		case 0x18000:
			asmLoad16(&gteIR1, ir);
			break;
	}	

	switch (psxRegs.code & 0x66000) {
		case 0x00000:
			mat_vec_load(rMatrix, trVector); ir[3] = 4096.0; break;
		case 0x02000:
			mat_vec_load(rMatrix, bkVector); ir[3] = 4096.0; break;
		case 0x04000:
			mat_vec_load(rMatrix, fcVector); ir[3] = 4096.0; break;
		case 0x06000:
			mat_zero_load(rMatrix); ir[3] = 0.0; break;

		case 0x20000:
			mat_vec_load(lMatrix, trVector); ir[3] = 4096.0; break;
		case 0x22000:
			mat_vec_load(lMatrix, bkVector); ir[3] = 4096.0; break;
		case 0x24000:
			mat_vec_load(lMatrix, fcVector); ir[3] = 4096.0; break;
		case 0x26000:
			mat_zero_load(lMatrix); ir[3] = 0.0; break;

		case 0x40000:
			mat_vec_load(cMatrix, trVector); ir[3] = 4096.0; break;
		case 0x42000:
			mat_vec_load(cMatrix, bkVector); ir[3] = 4096.0; break;
		case 0x44000:
			mat_vec_load(cMatrix, fcVector); ir[3] = 4096.0; break;
		case 0x46000:
			mat_zero_load(rMatrix); ir[3] = 0.0; break;
	}
			

	if (!(psxRegs.code & 0x80000)) {
		ir[0] *= 4096.0; ir[1] *= 4096.0; ir[2] *= 4096.0;
	}

	gteFLAG = asmTransformVector((u32)ir, (u32)mac, psxRegs.code & 0x400);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);
}

void gteNCLIP() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_NCLIP\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("NCLIP", 8);
		G_SD(12);
		G_SD(13);
		G_SD(14);
	}
#endif

	gteFLAG = 0;

//	asmNCLIP((u32)psxRegs.CP2D.r);
	
	gteMAC0 =	gteSX0 * (gteSY1 - gteSY2) +
				gteSX1 * (gteSY2 - gteSY0) +
				gteSX2 * (gteSY0 - gteSY1);

	SUM_FLAG;

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(24);
		G_GC(31);
	}
#endif
}

void gteAVSZ3() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_AVSZ3\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("AVSZ3", 5);
		G_SD(16);
		G_SD(17);
		G_SD(18);
		G_SD(19);
		G_SC(29);
		G_SC(30);
	}
#endif

	gteFLAG = 0;
	gteMAC0 = ((gteSZ0 + gteSZ1 + gteSZ2) * (gteZSF3));
	gteOTZ = limC((s32)gteMAC0>>12);
	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(7);
		G_GD(24);
		G_GC(31);
	}
#endif
}

void gteAVSZ4() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_AVSZ4\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("AVSZ4", 6);
		G_SD(16);
		G_SD(17);
		G_SD(18);
		G_SD(19);
		G_SC(29);
		G_SC(30);
	}
#endif

	gteFLAG = 0;
	gteMAC0 = ((gteSZx + gteSZ0 + gteSZ1 + gteSZ2) * (gteZSF4));
	gteOTZ = limC((s32)gteMAC0>>12);
	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(7);
		G_GD(24);
		G_GC(31);
	}
#endif
}

void gteSQR() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_SQR %lx\n", psxRegs.code & 0x1ffffff);
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("SQR", 5);
		G_SD(9);
		G_SD(10);
		G_SD(11);
	}
#endif

	gteFLAG = 0;
	
	if (psxRegs.code & 0x80000) {
		
		gteMAC1 = ((short)gteIR1 * (short)gteIR1)>>12;
		gteMAC2 = ((short)gteIR2 * (short)gteIR2)>>12;
		gteMAC3 = ((short)gteIR3 * (short)gteIR3)>>12;
	} else {
		
		gteMAC1 = (short)gteIR1 * (short)gteIR1;
		gteMAC2 = (short)gteIR2 * (short)gteIR2;
		gteMAC3 = (short)gteIR3 * (short)gteIR3;
	}
	MAC2IR1();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);
		G_GD(25);
		G_GD(26);
		G_GD(27);
		G_GC(31);
	}
#endif
}

#define GTE_NCCS(vn) \
	asmLoadVec(&gteVX##vn, ir); ir[3] = 0.0; mat_zero_load(lMatrix); \
	gteFLAG = asmTransformVector((u32)ir, (u32)mac, 1); \
	ir[3] = 4096.0; mat_vec_load(cMatrix, bkVector); \
	gteFLAG |= asmTransformVector((u32)ir, (u32)mac, 1); \
	gteMAC1 = ir[0] = (gteR*ir[0])/256.0; \
	gteMAC2 = ir[1] = (gteG*ir[1])/256.0; \
	gteMAC3 = ir[2] = (gteB*ir[2])/256.0; \
	limitB_z(ir); \
	gteIR1 = ir[0]; gteIR2 = ir[1]; gteIR3 = ir[2]; \
	UpdateColor();


void gteNCCS()  {
#ifdef GTE_DUMP
   static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_NCCS\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("NCCS", 17);
		G_SD(0);
		G_SD(1);
		G_SD(6);
		G_SC(8);
		G_SC(9);
		G_SC(10);
		G_SC(11);
		G_SC(12);
		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
		G_SC(20);
	}
#endif

	gteFLAG = 0;

	GTE_NCCS(0);

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		//G_GD(24); Doc must be wrong.  PSX does not touch it.
		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteNCCT() {
#ifdef GTE_DUMP
   static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_NCCT\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("NCCT", 39);
		G_SD(0);
		G_SD(1);
		G_SD(2);
		G_SD(3);
		G_SD(4);
		G_SD(5);
		G_SD(6);

		G_SC(8);
		G_SC(9);
		G_SC(10);
		G_SC(11);
		G_SC(12);
		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
		G_SC(20);
	}
#endif

    gteFLAG = 0;

	GTE_NCCS(0);
	GTE_NCCS(1);
	GTE_NCCS(2);

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		G_GD(20);
		G_GD(21);
		G_GD(22);

		//G_GD(24); Doc must be wrong.  PSX does not touch it.
		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

#define GTE_NCDS(vn) \
	asmLoadVec(&gteVX##vn, ir); ir[3] = 0.0; mat_zero_load(lMatrix); \
	gteFLAG = asmTransformVector((u32)ir, (u32)mac, 1); \
	ir[3] = 4096.0; mat_vec_load(cMatrix, bkVector); \
	gteFLAG |= asmTransformVector((u32)ir, (u32)mac, 1); \
	ir[0] = ((gteR*ir[0])/256.0); ir[1] = ((gteG*ir[1])/256.0); \
	ir[2] = ((gteB*ir[2])/256.0); ir[3] = (short)gteIR0; \
	gteFLAG |= asmColorCalc(ir, mac, 1); \
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1); \
	UpdateColor();

void gteNCDS() {
#ifdef GTE_DUMP
   static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_NCDS\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("NCDS", 19);
		G_SD(0);
		G_SD(1);
		G_SD(6);
		G_SD(8);

		G_SC(8);
		G_SC(9);
		G_SC(10);
		G_SC(11);
		G_SC(12);
		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
		G_SC(20);
		G_SC(21);
		G_SC(22);
		G_SC(23);
	}
#endif

    gteFLAG = 0;

    GTE_NCDS(0);

	SUM_FLAG;

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		G_GD(20);
		G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteNCDT() {
#ifdef GTE_DUMP
   static int sample = 0; sample++;
#endif
 
#ifdef GTE_LOG
	GTE_LOG("GTE_NCDT\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("NCDT", 44);
		G_SD(0);
		G_SD(1);
		G_SD(2);
		G_SD(3);
		G_SD(4);
		G_SD(5);
		G_SD(6);
		G_SD(8);

		G_SC(8);
		G_SC(9);
		G_SC(10);
		G_SC(11);
		G_SC(12);
		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
		G_SC(20);
		G_SC(21);
		G_SC(22);
		G_SC(23);
	}
#endif

    gteFLAG = 0;
    GTE_NCDS(0);
    GTE_NCDS(1);
    GTE_NCDS(2);

	SUM_FLAG;

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		G_GD(20);
		G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
} 

#define	gteD1	(*(short *)&gteR11)
#define	gteD2	(*(short *)&gteR22)
#define	gteD3	(*(short *)&gteR33)

void gteOP() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_OP %lx\n", psxRegs.code & 0x1ffffff);
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("OP", 6);
		G_SD(9);
		G_SD(10);
		G_SD(11);

		G_SC(0);
		G_SC(2);
		G_SC(4);
	}
#endif

	gteFLAG = 0;
	
	if (psxRegs.code  & 0x80000) {
		
		gteMAC1 = (gteD2 * (short)gteIR3 - gteD3 * (short)gteIR2) / 4096.0f;
		gteMAC2 = (gteD3 * (short)gteIR1 - gteD1 * (short)gteIR3) / 4096.0f;
        gteMAC3 = (gteD1 * (short)gteIR2 - gteD2 * (short)gteIR1) / 4096.0f;
	} else {
		
		gteMAC1 = gteD2 * (short)gteIR3 - gteD3 * (short)gteIR2;
		gteMAC2 = gteD3 * (short)gteIR1 - gteD1 * (short)gteIR3;
		gteMAC3 = gteD1 * (short)gteIR2 - gteD2 * (short)gteIR1;
	}

	MAC2IR();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteDCPL() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_DCPL\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("DCPL", 8);
		G_SD(6);
		G_SD(8);
		G_SD(9);
		G_SD(10);
		G_SD(11);

		G_SC(21);
		G_SC(22);
		G_SC(23);
	}
#endif

	asmLoad16(&gteIR1, ir);	
	ir[0] = ((gteR*ir[0])/256.0); ir[1] = ((gteG*ir[1])/256.0);
	ir[2] = ((gteB*ir[2])/256.0); ir[3] = (short)gteIR0;
	gteFLAG = asmColorCalc(ir, mac, 0);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteGPF() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_GPF %lx\n", psxRegs.code & 0x1ffffff);
#endif
#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("GPF", 5);
		G_SD(6);
		G_SD(8);
		G_SD(9);
		G_SD(10);
		G_SD(11);
	}
#endif

	gteFLAG = 0;

	if (psxRegs.code & 0x80000) {
		gteMAC1 = ((short)gteIR0 * (short)gteIR1)>>12;
		gteMAC2 = ((short)gteIR0 * (short)gteIR2)>>12;
		gteMAC3 = ((short)gteIR0 * (short)gteIR3)>>12;
	} else {
		gteMAC1 = (short)gteIR0 * (short)gteIR1;
		gteMAC2 = (short)gteIR0 * (short)gteIR2;
        gteMAC3 = (short)gteIR0 * (short)gteIR3;
	}
	MAC2IR();
	
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteGPL() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_GPL %lx\n", psxRegs.code & 0x1ffffff);
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("GPL", 5);
		G_SD(6);
		G_SD(8);
		G_SD(9);
		G_SD(10);
		G_SD(11);

		G_SD(25);
		G_SD(26);
		G_SD(27);
	}
#endif

	gteFLAG = 0;
	
	if (psxRegs.code & 0x80000) {
		
		gteMAC1 = gteMAC1 + ((short)gteIR0 * (short)gteIR1) / 4096.0f;
		gteMAC2 = gteMAC2 + ((short)gteIR0 * (short)gteIR2) / 4096.0f;
        gteMAC3 = gteMAC3 + ((short)gteIR0 * (short)gteIR3) / 4096.0f;
	} else {
		
		gteMAC1 = gteMAC1 + ((short)gteIR0 * (short)gteIR1);
		gteMAC2 = gteMAC2 + ((short)gteIR0 * (short)gteIR2);
        gteMAC3 = gteMAC3 + ((short)gteIR0 * (short)gteIR3);
	}
	MAC2IR();
	
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteDPCS() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_DPCS\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("DPCS", 8);
		G_SD(6);
		G_SD(8);

		G_SC(21);
		G_SC(22);
		G_SC(23);
	}
#endif

	ir[0] = (gteR*16.0); ir[1] = (gteG*16.0);
	ir[2] = (gteB*16.0); ir[3] = (short)gteIR0;
	gteFLAG = asmColorCalc(ir, mac, 0);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteDPCT() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif

#ifdef GTE_LOG
	GTE_LOG("GTE_DPCT\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("DPCT", 17);
		G_SD(8);

		G_SD(20);
		G_SD(21);
		G_SD(22);

		G_SC(21);
		G_SC(22);
		G_SC(23);
	}
#endif

	ir[0] = (gteR0*16.0); ir[1] = (gteG0*16.0);
	ir[2] = (gteB0*16.0); ir[3] = (short)gteIR0;
	gteFLAG = asmColorCalc(ir, mac, 0);
	asmStore32(mac, &gteMAC1);
	UpdateColor();

	ir[0] = (gteR0*16.0); ir[1] = (gteG0*16.0);
	ir[2] = (gteB0*16.0); ir[3] = (short)gteIR0;
	gteFLAG |= asmColorCalc(ir, mac, 0);
	asmStore32(mac, &gteMAC1);
	UpdateColor();

	ir[0] = (gteR0*16.0); ir[1] = (gteG0*16.0);
	ir[2] = (gteB0*16.0); ir[3] = (short)gteIR0;
	gteFLAG |= asmColorCalc(ir, mac, 0);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		G_GD(20);
		G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

#define GTE_NCS(vn) \
	asmLoadVec(&gteVX##vn, ir); ir[3] = 0.0; mat_zero_load(lMatrix); \
	gteFLAG = asmTransformVector((u32)ir, (u32)mac, 1); \
	ir[3] = 4096.0; mat_vec_load(cMatrix, bkVector); \
	gteFLAG |= asmTransformVector((u32)ir, (u32)mac, 1); \
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1); \
	UpdateColor();

void gteNCS() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_NCS\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("NCS", 14);
		G_SD(0);
		G_SD(1);
		G_SD(6);

		G_SC(8);
		G_SC(9);
		G_SC(10);
		G_SC(11);
		G_SC(12);
		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
		G_SC(20);
	}
#endif

	gteFLAG = 0;

	GTE_NCS(0);

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteNCT() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_NCT\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("NCT", 30);
		G_SD(0);
		G_SD(1);
		G_SD(2);
		G_SD(3);
		G_SD(4);
		G_SD(5);
		G_SD(6);

		G_SC(8);
		G_SC(9);
		G_SC(10);
		G_SC(11);
		G_SC(12);
		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
		G_SC(20);
	}
#endif

	gteFLAG = 0;
	
	GTE_NCS(0);
	GTE_NCS(1);
	GTE_NCS(2);

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		G_GD(20);
		G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteCC() {
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_CC\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("CC", 11);
		G_SD(6);
		G_SD(9);
		G_SD(10);
		G_SD(11);

		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
	}
#endif

	asmLoad16(&gteIR1, ir); ir[3] = 4096.0; mat_vec_load(cMatrix, bkVector);
	gteFLAG = asmTransformVector((u32)ir, (u32)mac, 1); 
	gteMAC1 = mac[0] = (gteR*ir[0])/256.0; 
	gteMAC2 = mac[1] = (gteG*ir[1])/256.0; 
	gteMAC3 = mac[2] = (gteB*ir[2])/256.0; 
	limitB_z(mac); 
	gteIR1 = mac[0]; gteIR2 = mac[1]; gteIR3 = mac[2]; 
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteINTPL() { //test opcode
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_INTP\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("INTPL", 8);
		G_SD(6);
		G_SD(8);
		G_SD(9);
		G_SD(10);
		G_SD(11);

		G_SC(21);
		G_SC(22);
		G_SC(23);
	}
#endif

	asmLoad16(&gteIR1, ir); ir[3] = (short)gteIR0;
	gteFLAG = asmColorCalc(ir, mac, 0);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		G_GD(20);
		G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}

void gteCDP() { //test opcode
#ifdef GTE_DUMP
	static int sample = 0; sample++;
#endif
#ifdef GTE_LOG
	GTE_LOG("GTE_CDP\n");
#endif

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_OP("CDP", 13);
		G_SD(6);
		G_SD(8);
		G_SD(9);
		G_SD(10);
		G_SD(11);

		G_SC(13);
		G_SC(14);
		G_SC(15);
		G_SC(16);
		G_SC(17);
		G_SC(18);
		G_SC(19);
		G_SC(20);
		G_SC(21);
		G_SC(22);
		G_SC(23);
	}
#endif

	asmLoad16(&gteIR1, ir); ir[3] = 4096.0; mat_vec_load(cMatrix, bkVector);
	gteFLAG = asmTransformVector((u32)ir, (u32)mac, 1); 
	ir[0] = ((gteR*ir[0])/256.0); ir[1] = ((gteG*ir[1])/256.0);
	ir[2] = ((gteB*ir[2])/256.0); ir[3] = (short)gteIR0;
	gteFLAG |= asmColorCalc(ir, mac, 1);
	asmStore32(mac, &gteMAC1); asmStore32(ir, &gteIR1);
	UpdateColor();

	SUM_FLAG

#ifdef GTE_DUMP
	if(sample < 100)
	{
		G_GD(9);
		G_GD(10);
		G_GD(11);

		//G_GD(20);
		//G_GD(21);
		G_GD(22);

		G_GD(25);
		G_GD(26);
		G_GD(27);

		G_GC(31);
	}
#endif
}
