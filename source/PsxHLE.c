/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2003  Pcsx Team
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


#include "PsxCommon.h"

static void hleDummy() {
	psxRegs.pc = psxRegs.GPR.n.ra;

	psxBranchTest();
}

static void hleA0() {
	u32 call = psxRegs.GPR.n.t1 & 0xff;

	if (biosA0[call]) biosA0[call]();

	psxBranchTest();
}

static void hleB0() {
	u32 call = psxRegs.GPR.n.t1 & 0xff;

	if (biosB0[call]) biosB0[call]();

	psxBranchTest();
}

static void hleC0() {
	u32 call = psxRegs.GPR.n.t1 & 0xff;

	if (biosC0[call]) biosC0[call]();

	psxBranchTest();
}

static void hleBootstrap() { // 0xbfc00000
	SysPrintf("hleBootstrap\n");
	CheckCdrom();
	LoadCdrom();
	SysPrintf("CdromLabel: \"%s\": PC = %8.8lx (SP = %8.8lx)\n", CdromLabel, psxRegs.pc, psxRegs.GPR.n.sp);
}

typedef struct {                   
	unsigned long _pc0;      
	unsigned long gp0;      
	unsigned long t_addr;   
	unsigned long t_size;   
	unsigned long d_addr;   
	unsigned long d_size;   
	unsigned long b_addr;   
	unsigned long b_size;   
	unsigned long S_addr;
	unsigned long s_size;
	unsigned long _sp,_fp,_gp,ret,base;
} EXEC;

static void hleExecRet() {
	EXEC *header = (EXEC*)PSXM(psxRegs.GPR.n.s0);

	SysPrintf("ExecRet %x: %x\n", psxRegs.GPR.n.s0, header->ret);

	psxRegs.GPR.n.ra = header->ret;
	psxRegs.GPR.n.sp = header->_sp;
	psxRegs.GPR.n.s8 = header->_fp;
	psxRegs.GPR.n.gp = header->_gp;
	psxRegs.GPR.n.s0 = header->base;

	psxRegs.GPR.n.v0 = 1;
	psxRegs.pc = psxRegs.GPR.n.ra;
}

void (*psxHLEt[256])() = {
	hleDummy, hleA0, hleB0, hleC0,
	hleBootstrap, hleExecRet
};
