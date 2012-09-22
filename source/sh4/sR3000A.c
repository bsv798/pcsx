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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "R3000A.h"
#include "PsxCommon.h"
#include "sh4.h"

u32 *recLUT;

#define PC_REC(x)	(recLUT[x >> 16] + (x & 0xffff))
#define PC_REC8(x)	(*(u8 *)PC_REC(x))
#define PC_REC16(x) (*(u16*)PC_REC(x))
#define PC_REC32(x) (*(u32*)PC_REC(x))

#define CLEAR_REC32(x) (*(u32*)(recLUT[(x) >> 16] + ((x) & 0xfffc)))

#define RECMEM_SIZE		(2*1024*1024)

#define CalcDispL(rx) (((u32)&(psxRegs.rx) - (u32)&psxRegs)>>2)

static char *recMem;				/* the recompiled blocks will be here */
static char *recRAM;				/* and the ptr to the blocks here */
static char *recROM;				/* and here */

static u32 pc;			/* recompiler pc */
static int count;					/* recompiler intruction count */
static int branch;					/* set for branch */
static u32 target;		/* branch target */

static void (*recBSC[64])();
static void (*recSPC[64])();
static void (*recREG[32])();
static void (*recCP0[32])();
static void (*recCP2[64])();
static void (*recCP2BSC[32])();


/* set a pending branch (target stored in R8) */
#define SetBranch() { \
	branch = 1; \
	psxRegs.code = PSXMu32(pc); \
	pc+=4; count++; \
	recBSC[psxRegs.code>>26](); \
 \
	MOV(R8, R0); \
	MOVLSG(CalcDispL(pc)); \
	CALLFunc((u32)psxBranchTest); \
}

#define REC_FUNC(f) \
void psx##f(); \
void rec##f() { \
	LoadImmediate32(psxRegs.code, R0); \
	MOVLSG(CalcDispL(code)); \
	LoadImmediate32(pc, R0); \
	MOVLSG(CalcDispL(pc)); \
	CALLFunc((u32)psx##f); \
}

static void recRecompile();

static int recInit() {
	int i;

	recLUT = (u32*) malloc(0x010000 * 4);

	recMem = (char*) malloc(RECMEM_SIZE);
	recRAM = (char*) malloc(0x200000);
	recROM = (char*) malloc(0x080000);
	if (recRAM == NULL || recROM == NULL || recMem == NULL || recLUT == NULL) {
		SysMessage("Error allocating memory"); return -1;
	}

	for (i=0; i<0x80; i++) recLUT[i + 0x0000] = (u32)&recRAM[(i & 0x1f) << 16];
	memcpy(recLUT + 0x8000, recLUT, 0x80 * 4);
	memcpy(recLUT + 0xa000, recLUT, 0x80 * 4);

	for (i=0; i<0x08; i++) recLUT[i + 0xbfc0] = (u32)&recROM[i << 16];

	return 0;
}

static void recReset() {
	memset(recRAM, 0, 0x200000);
	memset(recROM, 0, 0x080000);

	x86Init((char*)((u32)recMem|0xa0000000));
//	x86Init(recMem);
	branch = 0;
}

static void recShutdown() {
	if (recMem == NULL) return;
	free(recLUT);
	free(recMem);
	free(recRAM);
	free(recROM);
}

static void recError() {
	SysReset();
	ClosePlugins();
	SysMessage("Unrecoverable error while running recompiler\n");
	SysRunGui();
}

#define execute() { \
	void (**recFunc)(); \
	char *p; \
 \
	p =	(char*)PC_REC(psxRegs.pc); \
	if (p != NULL) recFunc = (void (**)()) (u32)p; \
	else { recError(); return; } \
 \
	if (*recFunc == 0) { \
		recRecompile(); \
	} \
	(*recFunc)(); \
}

static void DumpRegs() {
	int i, j;

	printf("%lx %lx\n", psxRegs.pc, psxRegs.cycle);
	for (i=0; i<4; i++) {
		for (j=0; j<8; j++)
			printf("%lx ", psxRegs.GPR.r[j*i]);
		printf("\n");
	}
}

static void recExecuteBios() {
	while (psxRegs.pc != 0x80030000) {
		execute();
	}
}

static void recExecute() {
	for (;;) execute();
}

static void recExecuteBlock() {
	execute();
}

static void recClear(u32 Addr, u32 Size) {
	if (Size == 1) { CLEAR_REC32(Addr) = 0; CLEAR_REC32(Addr+4) = 0; } else 
	memset((void*)PC_REC(Addr), 0, Size * 4);
}

static void recNULL() {
//	SysMessage("recUNK: %8.8x\n", psxRegs.code);
}

/*********************************************************
* goes to opcodes tables...                              *
* Format:  table[something....]                          *
*********************************************************/

//REC_SYS(SPECIAL);
static void recSPECIAL() {
	recSPC[_Funct_]();
}

static void recREGIMM() {
	recREG[_Rt_]();
}

static void recCOP0() {
	recCP0[_Rs_]();
}

//REC_SYS(COP2);
static void recCOP2() {
	recCP2[_Funct_]();
}

static void recBASIC() {
	recCP2BSC[_Rs_]();
}

//end of Tables opcodes...

/*********************************************************
* Arithmetic with immediate operand                      *
* Format:  OP rt, rs, immediate                          *
*********************************************************/

//REC_FUNC(ADDI);
//REC_FUNC(ADDIU);
//REC_FUNC(ANDI);
//REC_FUNC(ORI);
//REC_FUNC(XORI);
//REC_FUNC(SLTI);
//REC_FUNC(SLTIU);

void recADDIU()  {
// Rt = Rs + Im
	if (!_Rt_) return;

	if (_Imm_) {
		if (_Rs_) {
			LoadImmediate32(_Imm_, R1);
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
			ADD(R1, R0);
		} else {
			LoadImmediate32(_Imm_, R0);
		}
	} else {
		if (_Rs_)
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
		else
			XOR(R0, R0);
	}		
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
}

void recADDI()  {
// Rt = Rs + Im
	recADDIU();
}

void recSLTI() {
// Rt = Rs < Im (signed)
	if (!_Rt_) return;

	if (_Imm_) {
		LoadImmediate32(_Imm_, R1);
		MOVLLG(CalcDispL(GPR.r[_Rs_]));
		CMPGT(R0, R1);
		MOVT(R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	} else {
		XOR(R1, R1);
		MOVLLG(CalcDispL(GPR.r[_Rs_]));
		CMPGT(R0, R1);
		MOVT(R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	}
}

void recSLTIU() {
// Rt = Rs < Im (unsigned)
	if (!_Rt_) return;

	if (_ImmU_) {
		LoadImmediate32(_ImmU_, R1);
		MOVLLG(CalcDispL(GPR.r[_Rs_]));
		CMPHI(R0, R1);
		MOVT(R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	} else {
		XOR(R1, R1);
		MOVLLG(CalcDispL(GPR.r[_Rs_]));
		CMPHI(R0, R1);
		MOVT(R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	}
}

void recANDI() {
// Rt = Rs And Im
	if (!_Rt_) return;

	if (_Rs_ && _ImmU_) {
		LoadImmediate32(_ImmU_, R1);
		MOVLLG(CalcDispL(GPR.r[_Rs_]));
		AND(R1, R0);
	} else {
		XOR(R0, R0);
	}		
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
}

void recORI() {
// Rt = Rs Or Im
	if (!_Rt_) return;

	if (_ImmU_) {
		if (_Rs_) {
			LoadImmediate32(_ImmU_, R1);
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
			OR(R1, R0);
		} else {
			LoadImmediate32(_ImmU_, R0);
		}
	} else {
		if (_Rs_)
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
		else
			XOR(R0, R0);
	}		
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
}

void recXORI() {
// Rt = Rs Xor Im
	if (!_Rt_) return;

	if (_ImmU_) {
		if (_Rs_) {
			LoadImmediate32(_ImmU_, R1);
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
			XOR(R1, R0);
		} else {
			LoadImmediate32((_ImmU_ ^ 0), R0);
		}
	} else {
		if (_Rs_) {
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
			XORI(0);
		} else
			XOR(R0, R0);
	}		
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
}
//end of * Arithmetic with immediate operand  

/*********************************************************
* Load higher 16 bits of the first word in GPR with imm  *
* Format:  OP rt, immediate                              *
*********************************************************/
//REC_FUNC(LUI);

void recLUI()  {
// Rt = Imm << 16
	if (!_Rt_) return;

	LoadImmediate32(psxRegs.code << 16, R0);
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
}
//End of Load Higher .....

/*********************************************************
* Register arithmetic                                    *
* Format:  OP rd, rs, rt                                 *
*********************************************************/

//REC_FUNC(ADD);
//REC_FUNC(ADDU);
//REC_FUNC(SUB);
//REC_FUNC(SUBU);
//REC_FUNC(AND);
//REC_FUNC(OR);
//REC_FUNC(XOR);
//REC_FUNC(NOR);
//REC_FUNC(SLT);
//REC_FUNC(SLTU);

void recADDU() {
// Rd = Rs + Rt 
	if (!_Rd_) return;

	if (_Rs_ && _Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		MOV(R0, R1);
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		ADD(R1, R0);
	} else if (_Rs_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
	} else if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recADD() {
	recADDU();
}

void recSUBU() {
// Rd = Rs - Rt
	if (!_Rd_) return;

	if (_Rs_ || _Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		MOV(R0, R1);
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		SUB(R1, R0);
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}   

void recSUB() {
	recSUBU();
}

void recAND() {
// Rd = Rs And Rt
	if (!_Rd_) return;

	if (_Rs_ && _Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		MOV(R0, R1);
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		AND(R1, R0);
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}   

void recOR() {
// Rd = Rs Or Rt
	if (!_Rd_) return;

	if (_Rs_ && _Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		MOV(R0, R1);
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		OR(R1, R0);
	} else if (_Rs_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
	} else if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}   

void recXOR() {
// Rd = Rs Xor Rt
	if (!_Rd_) return;

	if (_Rs_ && _Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		MOV(R0, R1);
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		XOR(R1, R0);
	} else if (_Rs_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		XORI(0);
	} else if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		XORI(0);
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recNOR() {
// Rd = Rs Nor Rt
	if (!_Rd_) return;

	if (_Rs_ && _Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		MOV(R0, R1);
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		OR(R1, R0);
		NOT(R0, R0);
	} else if (_Rs_) {
		MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
		NOT(R0, R0);
	} else if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
		NOT(R0, R0);
	} else {
		MOVI(0xff, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recSLT() {
// Rd = Rs < Rt (signed)
	if (!_Rd_) return;

	MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
	MOV(R0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
	CMPGT(R1, R0);
	MOVT(R0);
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}  

void recSLTU() { 
// Rd = Rs < Rt (unsigned)
	if (!_Rd_) return;

	MOVLLG(CalcDispL(GPR.r[_Rs_])); // Load reg into R0
	MOV(R0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rt_])); // Load reg into R0
	CMPHI(R1, R0);
	MOVT(R0);
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}
//End of * Register arithmetic

/*********************************************************
* Register mult/div & Register trap logic                *
* Format:  OP rs, rt                                     *
*********************************************************/

//REC_FUNC(MULT);
//REC_FUNC(MULTU);
REC_FUNC(DIV);
REC_FUNC(DIVU);

void recMULT() {
// Lo/Hi = Rs * Rt (signed)

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOV(R0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rt_]));
	DMULS(R1, R0);
	STSMACL(R0);
	MOVLSG(CalcDispL(GPR.n.lo));
	STSMACH(R0);
	MOVLSG(CalcDispL(GPR.n.hi));
}

void recMULTU() {
// Lo/Hi = Rs * Rt (unsigned)

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOV(R0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rt_]));
	DMULU(R1, R0);
	STSMACL(R0);
	MOVLSG(CalcDispL(GPR.n.lo));
	STSMACH(R0);
	MOVLSG(CalcDispL(GPR.n.hi));
}

//End of * Register mult/div & Register trap logic  

//REC_FUNC(LB);
//REC_FUNC(LBU);
//REC_FUNC(LH);
//REC_FUNC(LHU);
//REC_FUNC(LW);

//REC_FUNC(SB);
//REC_FUNC(SH);
//REC_FUNC(SW);

REC_FUNC(LWL);
REC_FUNC(LWR);
REC_FUNC(SWL);
REC_FUNC(SWR);

void recLB() {
// Rt = mem[Rs + Im] (signed)

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);

	CALLFunc((u32)psxMemRead8);
	if (_Rt_) {
		EXTSB(R0, R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	}
}

void recLBU() {
// Rt = mem[Rs + Im] (unsigned)

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);

	CALLFunc((u32)psxMemRead8);
	if (_Rt_) {
		EXTUB(R0, R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	}
}

void recLH() {
// Rt = mem[Rs + Im] (signed)

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);

	CALLFunc((u32)psxMemRead16);
	if (_Rt_) {
		EXTSW(R0, R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	}
}

void recLHU() {
// Rt = mem[Rs + Im] (unsigned)

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);

	CALLFunc((u32)psxMemRead16);
	if (_Rt_) {
		EXTUW(R0, R0);
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
	}
}

void recLW() {
// Rt = mem[Rs + Im] (unsigned)

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);

	CALLFunc((u32)psxMemRead32);
	if (_Rt_)
		MOVLSG(CalcDispL(GPR.r[_Rt_]));
}

/*
void recLWL() {
}

void recLWR() {
}
*/
void recSB() {
// mem[Rs + Im] = Rt

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);
	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		EXTUB(R0, R5);
	} else
		MOVI(0, R5);
	CALLFunc((u32)psxMemWrite8);
}

void recSH() {
// mem[Rs + Im] = Rt

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);
	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		EXTUW(R0, R5);
	} else
		MOVI(0, R5);
	CALLFunc((u32)psxMemWrite16);
}

void recSW() {
// mem[Rs + Im] = Rt

	LoadImmediate32(_Imm_, R4);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	ADD(R0, R4);
	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		MOV(R0, R5);
	} else
		MOVI(0, R5);
	CALLFunc((u32)psxMemWrite32);
}
/*
void recSWL() {
}

void recSWR() {
}
*/
//REC_FUNC(SLL);
//REC_FUNC(SRL);
//REC_FUNC(SRA);

void recSLL() {
// Rd = Rt << Sa
	if (!_Rd_) return;

	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		if (_Sa_) {
			MOVI(_Sa_, R1);
			SHLD(R1, R0);
		}
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recSRL() {
// Rd = Rt >> Sa
	if (!_Rd_) return;

	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		if (_Sa_) {
			MOVI(-(_Sa_), R1);
			SHLD(R1, R0);
		}
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recSRA() {
// Rd = Rt >> Sa
	if (!_Rd_) return;

	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		if (_Sa_) {
			MOVI(-(_Sa_), R1);
			SHAD(R1, R0);
		}
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

//REC_FUNC(SLLV);
//REC_FUNC(SRLV);
//REC_FUNC(SRAV);

void recSLLV() {
// Rd = Rt << Rs
	if (!_Rd_) return;

	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		if (_Rs_) {
			MOV(R0, R1);
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
			ANDI(0x1f);
			SHLD(R0, R1);
			MOV(R1, R0);
		}			
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recSRLV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		if (_Rs_) {
			MOV(R0, R1);
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
			ANDI(0x1f);
			NEG(R0, R0);
			SHLD(R0, R1);
			MOV(R1, R0);
		}			
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recSRAV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (_Rt_) {
		MOVLLG(CalcDispL(GPR.r[_Rt_]));
		if (_Rs_) {
			MOV(R0, R1);
			MOVLLG(CalcDispL(GPR.r[_Rs_]));
			ANDI(0x1f);
			NEG(R0, R0);
			SHAD(R0, R1);
			MOV(R1, R0);
		}			
	} else {
		XOR(R0, R0);
	}
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

//REC_SYS(SYSCALL);
//REC_SYS(BREAK);

void recSYSCALL() {
	LoadImmediate32(pc - 4, R0);
	MOVLSG(CalcDispL(pc));
	LoadImmediate32(branch == 1 ? 1 : 0, R5);
	MOVI(0x20, R4);
	CALLFunc ((u32)psxException);
	if (!branch) branch = 2;
}

void recBREAK() {
	LoadImmediate32(pc - 4, R0);
	MOVLSG(CalcDispL(pc));
	LoadImmediate32(branch == 1 ? 1 : 0, R5);
	MOVI(0x40, R4);
	CALLFunc ((u32)psxException);
	if (!branch) branch = 2;
}

/*
REC_FUNC(MFHI);
REC_FUNC(MTHI);
REC_FUNC(MFLO);
REC_FUNC(MTLO);
*/
void recMFHI() {
// Rd = Hi
	if (!_Rd_) return;

	MOVLLG(CalcDispL(GPR.n.hi)); // Load reg into R0
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recMTHI() {
// Hi = Rs

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOVLSG(CalcDispL(GPR.n.hi));
}

void recMFLO() {
// Rd = Lo
	if (!_Rd_) return;

	MOVLLG(CalcDispL(GPR.n.lo)); // Load reg into R0
	MOVLSG(CalcDispL(GPR.r[_Rd_]));
}

void recMTLO() {
// Lo = Rs

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOVLSG(CalcDispL(GPR.n.lo));
}

//REC_BRANCH(J);
//REC_BRANCH(JR);
//REC_BRANCH(JAL);
//REC_BRANCH(JALR);
//REC_BRANCH(BLTZ);
//REC_BRANCH(BGTZ);
//REC_BRANCH(BLTZAL);
//REC_BRANCH(BGEZAL);
//REC_BRANCH(BNE);
//REC_BRANCH(BEQ);
//REC_BRANCH(BLEZ);
//REC_BRANCH(BGEZ);

void recBLTZ() {
// Branch if Rs < 0

	MOVI(0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	CMPGT(R0, R1);
	j8Ptr[0] = BT(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();

	shSetBx(j8Ptr[0]);
	LoadImmediate32((_Imm_ * 4 + pc), R8);

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

void recBGEZ() {
// Branch if Rs >= 0

	MOVI(0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	CMPGT(R0, R1);
	j8Ptr[0] = BF(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();
	
	shSetBx(j8Ptr[0]);
	LoadImmediate32((_Imm_ * 4 + pc), R8);

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

void recBLTZAL() {
// Branch if Rs < 0

	MOVI(0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	CMPGT(R0, R1);
	j8Ptr[0] = BT(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();
	
	shSetBx(j8Ptr[0]);
	LoadImmediate32(pc + 4, R0);
	LoadImmediate32((_Imm_ * 4 + pc), R8);
	MOVLSG(CalcDispL(GPR.r[31]));

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

void recBGEZAL() {
// Branch if Rs >= 0

	MOVI(0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	CMPGT(R0, R1);
	j8Ptr[0] = BF(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();
	
	shSetBx(j8Ptr[0]);
	LoadImmediate32(pc + 4, R0);
	LoadImmediate32((_Imm_ * 4 + pc), R8);
	MOVLSG(CalcDispL(GPR.r[31]));

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

void recJ() {
// j target

	LoadImmediate32(_Target_ * 4 + (pc & 0xf0000000), R8);
	SetBranch();
}

void recJR() {
// jr Rs

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOV(R0, R8);
	SetBranch();
}

void recJALR() {
// jalr Rs

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOV(R0, R8);
	if (_Rd_) {
		LoadImmediate32(pc + 4, R0);
		MOVLSG(CalcDispL(GPR.r[_Rd_]));
	}
	SetBranch();
}

void recJAL() {
// jal target

	LoadImmediate32(_Target_ * 4 + (pc & 0xf0000000), R8);
	LoadImmediate32(pc + 4, R0);
	MOVLSG(CalcDispL(GPR.r[31]));
	SetBranch();
}

void recBEQ() {
// Branch if Rs == Rt

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOV(R0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rt_]));
	CMPEQ(R0, R1);
	j8Ptr[0] = BT(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();
	
	shSetBx(j8Ptr[0]);
	LoadImmediate32((_Imm_ * 4 + pc), R8);

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

void recBNE() {
// Branch if Rs != Rt

	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	MOV(R0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rt_]));
	CMPEQ(R0, R1);
	j8Ptr[0] = BF(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();
	
	shSetBx(j8Ptr[0]);
	LoadImmediate32((_Imm_ * 4 + pc), R8);

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

void recBLEZ() {
// Branch if Rs <= 0

	MOVI(0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	CMPGE(R0, R1);
	j8Ptr[0] = BT(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();
	
	shSetBx(j8Ptr[0]);
	LoadImmediate32((_Imm_ * 4 + pc), R8);

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

void recBGTZ() {
// Branch if Rs > 0

	MOVI(0, R1);
	MOVLLG(CalcDispL(GPR.r[_Rs_]));
	CMPGE(R0, R1);
	j8Ptr[0] = BF(0);
	LoadImmediate32(pc + 4, R8);
	j16Ptr[0] = BRA(0);
	NOP  ();
	
	shSetBx(j8Ptr[0]);
	LoadImmediate32((_Imm_ * 4 + pc), R8);

	shSetBRA(j16Ptr[0]);
	SetBranch();
}

//REC_FUNC(MFC0);
//REC_FUNC(MTC0);
//REC_FUNC(CFC0);
//REC_FUNC(CTC0);
//REC_FUNC(RFE);

void recMFC0() {
// Rt = Cop0->Rd
	if (!_Rt_) return;

	MOVLLG(CalcDispL(CP0.r[_Rd_])); // Load reg into R0
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
}

void recCFC0() {
// Rt = Cop0->Rd
	if (!_Rt_) return;

	MOVLLG(CalcDispL(CP0.r[_Rd_])); // Load reg into R0
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
}

void recMTC0() {
// Cop0->Rd = Rt

	MOVLLG(CalcDispL(GPR.r[_Rt_]));
	MOVLSG(CalcDispL(CP0.r[_Rd_]));
}

void recCTC0() {
// Cop0->Rd = Rt

	MOVLLG(CalcDispL(GPR.r[_Rt_]));
	MOVLSG(CalcDispL(CP0.r[_Rd_]));
}

void recRFE() {
	MOVI(0xf0, R2);
	MOVLLG(CalcDispL(CP0.n.Status));
	AND(R0, R2);
	ANDI(0x3c);
	SHLR2(R0);
	OR(R2, R0);
	MOVLSG(CalcDispL(CP0.n.Status));
}

#define CP2_FUNC(f) \
void gte##f(); \
void rec##f() { \
	LoadImmediate32(psxRegs.code, R0); \
	MOVLSG(CalcDispL(code)); \
	LoadImmediate32(pc, R0); \
	MOVLSG(CalcDispL(pc)); \
	CALLFunc ((u32)gte##f); \
}

CP2_FUNC(MFC2);
CP2_FUNC(MTC2);
//CP2_FUNC(CFC2);
//CP2_FUNC(CTC2);
CP2_FUNC(LWC2);
CP2_FUNC(SWC2);

CP2_FUNC(RTPS);
CP2_FUNC(OP);
//CP2_FUNC(NCLIP);
CP2_FUNC(DPCS);
CP2_FUNC(INTPL);
//CP2_FUNC(MVMVA);
CP2_FUNC(NCDS);
CP2_FUNC(NCDT);
CP2_FUNC(CDP);
CP2_FUNC(NCCS);
CP2_FUNC(CC);
CP2_FUNC(NCS);
CP2_FUNC(NCT);
CP2_FUNC(SQR);
CP2_FUNC(DCPL);
CP2_FUNC(DPCT);
CP2_FUNC(AVSZ3);
CP2_FUNC(AVSZ4);
CP2_FUNC(RTPT);
CP2_FUNC(GPF);
CP2_FUNC(GPL);
CP2_FUNC(NCCT);

void recNCLIP() {
	MOVI(0, R0);
	MOVLSG(CalcDispL(CP2C.r[31]));
	
	LoadImmediate32((u32)psxRegs.CP2D.r, R4);
	CALLFunc((u32)asmNCLIP);
}


void recMVMVA() {

	switch (psxRegs.code & 0x18000) {
		case 0x00000:	// VX0
			LoadImmediate32((u32)&psxRegs.CP2D.r[0], R4);
			LoadImmediate32((u32)ir, R5);
			CALLFunc((u32)asmLoadVec);
			break;
		case 0x08000:	// VX1
			LoadImmediate32((u32)&psxRegs.CP2D.r[2], R4);
			LoadImmediate32((u32)ir, R5);
			CALLFunc((u32)asmLoadVec);
			break;
		case 0x10000:	// VX2
			LoadImmediate32((u32)&psxRegs.CP2D.r[4], R4);
			LoadImmediate32((u32)ir, R5);
			CALLFunc((u32)asmLoadVec);
			break;
		case 0x18000:	// IR1
			LoadImmediate32((u32)&psxRegs.CP2D.r[9], R4);
			LoadImmediate32((u32)ir, R5);
			CALLFunc((u32)asmLoad16);
			break;
	}	

	switch (psxRegs.code & 0x60000) {
		case 0x00000:
			LoadImmediate32((u32)rMatrix, R4);
			break;
		case 0x20000:
			LoadImmediate32((u32)lMatrix, R4);
			break;
		case 0x40000:
			LoadImmediate32((u32)cMatrix, R4);
			break;
		case 0x60000:
			LoadImmediate32((u32)rMatrix, R4);
			break;
	}

	switch (psxRegs.code & 0x06000) {
		case 0x00000:
			LoadImmediate32((u32)trVector, R5);
			CALLFunc((u32)mat_vec_load);
			LoadImmediate32((u32)&ir[3], R4);
			LoadImmediate32((u32)0x45800000, R5);	// 4096.0
			MOVLS(R5, R4);
			break;
		case 0x02000:
			LoadImmediate32((u32)bkVector, R5);
			CALLFunc((u32)mat_vec_load);
			LoadImmediate32((u32)&ir[3], R4);
			LoadImmediate32((u32)0x45800000, R5);	// 4096.0
			MOVLS(R5, R4);
			break;
		case 0x04000:
			LoadImmediate32((u32)fcVector, R5);
			CALLFunc((u32)mat_vec_load);
			LoadImmediate32((u32)&ir[3], R4);
			LoadImmediate32((u32)0x45800000, R5);	// 4096.0
			MOVLS(R5, R4);
			break;
		case 0x06000:
			CALLFunc((u32)mat_zero_load);
			LoadImmediate32((u32)&ir[3], R4);
			LoadImmediate32((u32)0, R5);	// 0.0
			MOVLS(R5, R4);
			break;
	}
			

	if (!(psxRegs.code & 0x80000)) {
		// ir[0] *= 4096.0; ir[1] *= 4096.0; ir[2] *= 4096.0;
		LoadImmediate32((u32)ir, R1);
		LoadImmediate32(4096, R2);
		LDSFPUL(R2);
		FLOAT(FR3);
		FMOV_RESTORE(R1, FR0);
		FMUL(FR3, FR0);
		FMOV_RESTORE(R1, FR1);
		FMUL(FR3, FR1);
		FMOV_RESTORE(R1, FR2);
		FMUL(FR3, FR2);
		FMOV_SAVE(R1, FR2);
		FMOV_SAVE(R1, FR1);
		FMOV_SAVE(R1, FR0);
	}

	// gteFLAG = asmTransformVector((u32)ir, (u32)mac, psxRegs.code & 0x400);
	LoadImmediate32((u32)ir, R4);
	LoadImmediate32((u32)mac, R5);
	LoadImmediate32((u32)(psxRegs.code & 0x400), R6);
	CALLFunc((u32)asmTransformVector);
	MOVLSG(CalcDispL(CP2C.r[31]));	// gteFLAG

	// asmStore32(ir, &gteIR1);
	LoadImmediate32((u32)ir, R4);
	LoadImmediate32((u32)&psxRegs.CP2D.r[9], R5);
	CALLFunc((u32)asmStore32);

	// asmStore32(mac, &gteMAC1); 
	LoadImmediate32((u32)mac, R4);
	LoadImmediate32((u32)&psxRegs.CP2D.r[25], R5);
	CALLFunc((u32)asmStore32);

//	SUM_FLAG;
}

void recCFC2() {
	if (!_Rt_) return;
	MOVLLG(CalcDispL(CP2C.r[_Rd_]));
	MOVLSG(CalcDispL(GPR.r[_Rt_]));
//	psxRegs.GPR.r[_Rt_] = psxRegs.CP2C.r[_Rd_];
}

/* Loads an element into matrix mx: lower word at pos _p1, upper word at pos _p2 */
#define _LOAD_MATRIX(mx, _p1, _p2) { \
	LoadImmediate32((u32)mx##Matrix, R5); \
	MOVI(_p1*4, R6); \
	LoadImmediate32((u32)LoadMatrixElement, R1); \
	JSR(R1); \
	MOVI(_p2*4, R7); \
}

void recCTC2() {
	MOVLLG(CalcDispL(GPR.r[_Rt_]));
	MOVLSG(CalcDispL(CP2C.r[_Rd_]));
//	psxRegs.CP2C.r[_Rd_] = psxRegs.GPR.r[_Rt_];
//	printf("Store to CP2C %d at %08x\n", _Rd_, (u32)x86Ptr);
	switch(_Rd_) {
		case 0:
			MOV(R0, R4); _LOAD_MATRIX(r, 0, 4); break;
		case 1:
			MOV(R0, R4); _LOAD_MATRIX(r, 8, 1); break;
		case 2:
			MOV(R0, R4); _LOAD_MATRIX(r, 5, 9); break;
		case 3:
			MOV(R0, R4); _LOAD_MATRIX(r, 2, 6); break;
		case 4: // clear upper 16 Bit
			EXTUW(R0, R4); _LOAD_MATRIX(r, 10, 3); break;
			
		case 5: {
			LoadImmediate32((u32)&(trVector[0]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;

		case 6: {
			LoadImmediate32((u32)&(trVector[1]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;

		case 7: {
			LoadImmediate32((u32)&(trVector[2]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;

		case 8:
			MOV(R0, R4); _LOAD_MATRIX(l, 0, 4); break;
		case 9:
			MOV(R0, R4); _LOAD_MATRIX(l, 8, 1); break;
		case 10: 
			MOV(R0, R4); _LOAD_MATRIX(l, 5, 9); break;
		case 11:
			MOV(R0, R4); _LOAD_MATRIX(l, 2, 6); break;
		case 12: // clear upper 16 Bit
			EXTUW(R0, R4); _LOAD_MATRIX(l, 10, 3); break;

		case 13: {
			LoadImmediate32((u32)&(bkVector[0]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;

		case 14: {
			LoadImmediate32((u32)&(bkVector[1]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;

		case 15: {
			LoadImmediate32((u32)&(bkVector[2]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;

		case 16:
			MOV(R0, R4); _LOAD_MATRIX(c, 0, 4); break;
		case 17:
			MOV(R0, R4); _LOAD_MATRIX(c, 8, 1); break;
		case 18:
			MOV(R0, R4); _LOAD_MATRIX(c, 5, 9); break;
		case 19: 
			MOV(R0, R4); _LOAD_MATRIX(c, 2, 6); break;
		case 20: // clear upper 16 Bit
			EXTUW(R0, R4); _LOAD_MATRIX(c, 10, 3); break;

		case 21: {
			LoadImmediate32((u32)&(fcVector[0]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 22: {
			LoadImmediate32((u32)&(fcVector[1]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 23: {
			LoadImmediate32((u32)&(fcVector[2]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 24: { // OFX
			LoadImmediate32((u32)&(control[0]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 25: { // OFY
			LoadImmediate32((u32)&(control[1]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 26: { // H
			LoadImmediate32((u32)&(control[2]), R1);
			EXTUW(R0, R0);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 27: { // DQA
			LoadImmediate32((u32)&(control[3]), R1);
			EXTSW(R0, R0);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 28: { // DQB
			LoadImmediate32((u32)&(control[4]), R1);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 29: { // ZSF3
			LoadImmediate32((u32)&(control[5]), R1);
			EXTSW(R0, R0);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
		case 30: { // ZSF4
			LoadImmediate32((u32)&(control[6]), R1);
			EXTSW(R0, R0);
			LDSFPUL(R0); FLOAT(FR0); FMOV_STORE(FR0, R1);
			} break;
	}
}


static void recHLE() {
	CALLFunc((u32)psxHLEt[psxRegs.code & 0xff]);
	branch = 2;
}

//

static void (*recBSC[64])() = {
	recSPECIAL, recREGIMM, recJ   , recJAL  , recBEQ , recBNE , recBLEZ, recBGTZ,
	recADDI   , recADDIU , recSLTI, recSLTIU, recANDI, recORI , recXORI, recLUI ,
	recCOP0   , recNULL  , recCOP2, recNULL , recNULL, recNULL, recNULL, recNULL,
	recNULL   , recNULL  , recNULL, recNULL , recNULL, recNULL, recNULL, recNULL,
	recLB     , recLH    , recLWL , recLW   , recLBU , recLHU , recLWR , recNULL,
	recSB     , recSH    , recSWL , recSW   , recNULL, recNULL, recSWR , recNULL,
	recNULL   , recNULL  , recLWC2, recNULL , recNULL, recNULL, recNULL, recNULL,
	recNULL   , recNULL  , recSWC2, recHLE  , recNULL, recNULL, recNULL, recNULL
};

static void (*recSPC[64])() = {
	recSLL , recNULL, recSRL , recSRA , recSLLV   , recNULL , recSRLV, recSRAV,
	recJR  , recJALR, recNULL, recNULL, recSYSCALL, recBREAK, recNULL, recNULL,
	recMFHI, recMTHI, recMFLO, recMTLO, recNULL   , recNULL , recNULL, recNULL,
	recMULT, recMULTU, recDIV, recDIVU, recNULL   , recNULL , recNULL, recNULL,
	recADD , recADDU, recSUB , recSUBU, recAND    , recOR   , recXOR , recNOR ,
	recNULL, recNULL, recSLT , recSLTU, recNULL   , recNULL , recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL   , recNULL , recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL   , recNULL , recNULL, recNULL
};

static void (*recREG[32])() = {
	recBLTZ  , recBGEZ  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL  , recNULL  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recBLTZAL, recBGEZAL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL  , recNULL  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

static void (*recCP0[32])() = {
	recMFC0, recNULL, recCFC0, recNULL, recMTC0, recNULL, recCTC0, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recRFE , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

static void (*recCP2[64])() = {
	recBASIC, recRTPS , recNULL , recNULL, recNULL, recNULL , recNCLIP, recNULL, // 00
	recNULL , recNULL , recNULL , recNULL, recOP  , recNULL , recNULL , recNULL, // 08
	recDPCS , recINTPL, recMVMVA, recNCDS, recCDP , recNULL , recNCDT , recNULL, // 10
	recNULL , recNULL , recNULL , recNCCS, recCC  , recNULL , recNCS  , recNULL, // 18
	recNCT  , recNULL , recNULL , recNULL, recNULL, recNULL , recNULL , recNULL, // 20
	recSQR  , recDCPL , recDPCT , recNULL, recNULL, recAVSZ3, recAVSZ4, recNULL, // 28 
	recRTPT , recNULL , recNULL , recNULL, recNULL, recNULL , recNULL , recNULL, // 30
	recNULL , recNULL , recNULL , recNULL, recNULL, recGPF  , recGPL  , recNCCT  // 38
};

static void (*recCP2BSC[32])() = {
	recMFC2, recNULL, recCFC2, recNULL, recMTC2, recNULL, recCTC2, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

static void recRecompile() {
	/* if x86Ptr reached the mem limit reset whole mem */
	if (((u32)x86Ptr - ((u32)recMem|0xa0000000)) >= (RECMEM_SIZE - 0x10000))
		recReset();

	PC_REC32(psxRegs.pc) = (u32)x86Ptr & ~0x20000000;  // Clear non-cached bit
	pc = psxRegs.pc;

	immCount = 0;
	
	STSMPR(R15);	// Save Return Address
	STCMGBR(R15);
	MOVLM(R8, R15);
	
	LoadImmediate32((u32)&psxRegs, R1);
	LDCGBR(R1);	// Load GBR with addr of psxRegs
	
	for (count=0; count<500;) {
		char *p;

		p = (char *)PSXM(pc);
		if (p == NULL) recError();
		psxRegs.code = *(u32 *)p;

		pc+=4; count++;
		recBSC[psxRegs.code>>26]();

		if (branch) {
			break;
		}
		UpdateImmediate32(0);
	}

	if (!branch) {
		LoadImmediate32(pc, R0);
		MOVLSG(CalcDispL(pc));
	}

	/* store cycle */
	LoadImmediate32(count, R1);
	MOVLLG(CalcDispL(cycle));
	ADD(R1, R0);
	MOVLSG(CalcDispL(cycle));

	branch = 0;

	MOVLP(R15, R8);
	LDCMGBR(R15);
	LDSMPR(R15);
	RET();
	NOP();
	UpdateImmediate32(1);
	Align32();
}

R3000Acpu psxRec = {
	recInit,
	recReset,
	recExecute,
	recExecuteBlock,
	recClear,
	recShutdown
};
