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

#include <string.h>
#include <stdlib.h>

#include "PsxCommon.h"


int psxMemInit() {
	int i;

	psxMemRLUT = (long*)malloc(0x10000 * 4);
	psxMemWLUT = (long*)malloc(0x10000 * 4);
	memset(psxMemRLUT, 0, 0x10000 * 4);
	memset(psxMemWLUT, 0, 0x10000 * 4);

	psxM = (char*)malloc(0x00200000);
	psxP = (char*)malloc(0x00010000);
	psxH = (char*)malloc(0x00010000);
	psxR = (char*)malloc(0x00080000);
	if (psxMemRLUT == NULL || psxMemWLUT == NULL || 
		psxM == NULL || psxP == NULL || psxH == NULL) {
		SysMessage(_("Error allocating memory")); return -1;
	}

// MemR
	for (i=0; i<0x80; i++) psxMemRLUT[i + 0x0000] = (u32)&psxM[(i & 0x1f) << 16];
	memcpy(psxMemRLUT + 0x8000, psxMemRLUT, 0x80 * 4);
	memcpy(psxMemRLUT + 0xa000, psxMemRLUT, 0x80 * 4);

	for (i=0; i<0x01; i++) psxMemRLUT[i + 0x1f00] = (u32)&psxP[i << 16];

	for (i=0; i<0x01; i++) psxMemRLUT[i + 0x1f80] = (u32)&psxH[i << 16];

	for (i=0; i<0x08; i++) psxMemRLUT[i + 0xbfc0] = (u32)&psxR[i << 16];

// MemW
	for (i=0; i<0x80; i++) psxMemWLUT[i + 0x0000] = (u32)&psxM[(i & 0x1f) << 16];
	memcpy(psxMemWLUT + 0x8000, psxMemWLUT, 0x80 * 4);
	memcpy(psxMemWLUT + 0xa000, psxMemWLUT, 0x80 * 4);

	for (i=0; i<0x01; i++) psxMemWLUT[i + 0x1f00] = (u32)&psxP[i << 16];

	for (i=0; i<0x01; i++) psxMemWLUT[i + 0x1f80] = (u32)&psxH[i << 16];

	return 0;
}

void psxMemReset() {
	FILE *f = NULL;
	char Bios[256];

	memset(psxM, 0, 0x00200000);
	memset(psxP, 0, 0x00010000);

	if (strcmp(Config.Bios, "HLE")) {
		sprintf(Bios, "%s%s", Config.BiosDir, Config.Bios);
		f = fopen(Bios, "rb");

		if (f == NULL) {
			SysMessage (_("Could not open bios:\"%s\". Enabling HLE Bios\n"), Bios);
			memset(psxR, 0, 0x80000);
			Config.HLE = 1;
		}
		else {
			fread(psxR, 1, 0x80000, f);
			fclose(f);
			Config.HLE = 0;
		}
	} else Config.HLE = 1;
}

void psxMemShutdown() {
	free(psxM);
	free(psxP);
	free(psxH);
	free(psxR);
	free(psxMemRLUT);
	free(psxMemWLUT);
}

static int writeok=1;

u8 psxMemRead8(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return psxHu8(mem);
		else
			return psxHwRead8(mem);
	} else {
		p = (char *)(psxMemRLUT[t]);
		if (p != NULL) {
			return *(u8 *)(p + (mem & 0xffff));
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err lb %8.8lx\n", mem);
#endif
			return 0;
		}
	}
}

u16 psxMemRead16(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return psxHu16(mem);
		else
			return psxHwRead16(mem);
	} else {
		p = (char *)(psxMemRLUT[t]);
		if (p != NULL) {
			return SWAPu16(*(u16 *)(p + (mem & 0xffff)));
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err lh %8.8lx\n", mem);
#endif
			return 0;
		}
	}
}

u32 psxMemRead32(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return psxHu32(mem);
		else
			return psxHwRead32(mem);
	} else {
		p = (char *)(psxMemRLUT[t]);
		if (p != NULL) {
			return SWAPu32(*(u32 *)(p + (mem & 0xffff)));
		} else {
#ifdef PSXMEM_LOG
			if (writeok) { PSXMEM_LOG("err lw %8.8lx\n", mem); }
#endif
			return 0;
		}
	}
}

void psxMemWrite8(u32 mem, u8 value) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			psxHu8(mem) = value;
		else
			psxHwWrite8(mem, value);
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
			*(u8  *)(p + (mem & 0xffff)) = value;
#ifdef PSXREC
			if (!Config.Cpu) REC_CLEARM(mem&(~3));
#endif
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err sb %8.8lx\n", mem);
#endif
		}
	}
}

void psxMemWrite16(u32 mem, u16 value) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			psxHu16ref(mem) = SWAPu16(value);
		else
			psxHwWrite16(mem, value);
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
			*(u16 *)(p + (mem & 0xffff)) = SWAPu16(value);
#ifdef PSXREC
			if (!Config.Cpu) REC_CLEARM(mem&(~1));
#endif
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err sh %8.8lx\n", mem);
#endif
		}
	}
}

void psxMemWrite32(u32 mem, u32 value) {
	char *p;
	u32 t;

//	if ((mem&0x1fffff) == 0x71E18 || value == 0x48088800) SysPrintf("t2fix!!\n");
	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			psxHu32ref(mem) = SWAPu32(value);
		else
			psxHwWrite32(mem, value);
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
			*(u32 *)(p + (mem & 0xffff)) = SWAPu32(value);
#ifdef PSXREC
			if (!Config.Cpu) REC_CLEARM(mem);
#endif
		} else {
			if (mem != 0xfffe0130) {
#ifdef PSXREC
				if (!writeok && !Config.Cpu) REC_CLEARM(mem);
#endif

#ifdef PSXMEM_LOG
				if (writeok) { PSXMEM_LOG("err sw %8.8lx\n", mem); }
#endif
			} else {
				int i;

				switch (value) {
					case 0x800: case 0x804:
						if (writeok == 0) break;
						writeok = 0;
						memset(psxMemWLUT + 0x0000, 0, 0x80 * 4);
						memset(psxMemWLUT + 0x8000, 0, 0x80 * 4);
						memset(psxMemWLUT + 0xa000, 0, 0x80 * 4);
						break;
					case 0x1e988:
						if (writeok == 1) break;
						writeok = 1;
						for (i=0; i<0x80; i++) psxMemWLUT[i + 0x0000] = (u32)&psxM[(i & 0x1f) << 16];
						memcpy(psxMemWLUT + 0x8000, psxMemWLUT, 0x80 * 4);
						memcpy(psxMemWLUT + 0xa000, psxMemWLUT, 0x80 * 4);
						break;
					default:
#ifdef PSXMEM_LOG
						PSXMEM_LOG("unk %8.8lx = %x\n", mem, value);
#endif
						break;
				}
			}
		}
	}
}

void *psxMemPointer(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return (void *)&psxH[mem];
		else
			return NULL;
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
			return (void *)(p + (mem & 0xffff));
		}
		return NULL;
	}
}
