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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "Coff.h"
#include "PsxCommon.h"
#include "plugins.h"

int Log = 0;

char *LabelAuthors = { N_(
	"PCSX a psx emulator\n\n"
	"written by:\n"
	"main coder: linuzappz\n"
	"co-coders: shadow\n"
	"ex-coders: Nocomp, Pete Bernett, nik3d\n"
	"Webmaster: AkumaX")
};

char *LabelGreets = { N_(
	"Greets to: Duddie, Tratax, Kazzuya, JNS, Bobbi, Psychojak, Shunt,\n"
	"           Keith, DarkWatcher, Xeven, Liquid, Dixon, Prafull\n"
	"Special thanks to:\n"
	"Twin (we Love you twin0r), Roor (love for you too),\n"
	"calb (Thanks for help :) ), now3d (for great help to my psxdev stuff :) )")
};


// LOAD STUFF

#ifdef __MACOSX__

#define SWAP(b) b = (((unsigned char*)&b)[0]&0xff) | ((((unsigned char*)&b)[1]&0xff)<<8) | ((((unsigned char*)&b)[2]&0xff)<<16) | (((unsigned char*)&b)[3]<<24);

void swapEXE_HEADER(EXE_HEADER* exe) {
	SWAP(exe->text);
	SWAP(exe->data);                    
	SWAP(exe->pc0);
	SWAP(exe->gp0);                     
	SWAP(exe->t_addr);
	SWAP(exe->t_size);
	SWAP(exe->d_addr);                  
	SWAP(exe->d_size);                  
	SWAP(exe->b_addr);                  
	SWAP(exe->b_size);                  
	SWAP(exe->s_addr);
	SWAP(exe->s_size);
	SWAP(exe->SavedSP);
	SWAP(exe->SavedFP);
	SWAP(exe->SavedGP);
	SWAP(exe->SavedRA);
	SWAP(exe->SavedS0);
}

#endif

#define ISODCL(from, to) (to - from + 1)

struct iso_directory_record {
	char length			[ISODCL (1, 1)]; /* 711 */
	char ext_attr_length		[ISODCL (2, 2)]; /* 711 */
	char extent			[ISODCL (3, 10)]; /* 733 */
	char size			[ISODCL (11, 18)]; /* 733 */
	char date			[ISODCL (19, 25)]; /* 7 by 711 */
	char flags			[ISODCL (26, 26)];
	char file_unit_size		[ISODCL (27, 27)]; /* 711 */
	char interleave			[ISODCL (28, 28)]; /* 711 */
	char volume_sequence_number	[ISODCL (29, 32)]; /* 723 */
	unsigned char name_len		[ISODCL (33, 33)]; /* 711 */
	char name			[1];
};

#define btoi(b)		((b)/16*10 + (b)%16)		/* BCD to u_char */
#define itob(i)		((i)/10*16 + (i)%10)		/* u_char to BCD */

void mmssdd( char *b, char *p )
 {
	int m, s, d;
#if defined(__DREAMCAST__) || defined(__MACOSX__)
	int block = (b[0]&0xff) | ((b[1]&0xff)<<8) | ((b[2]&0xff)<<16) | (b[3]<<24);
#else
	int block = *((int*)b);
#endif
	
	block += 150;
	m = block / 4500;			// minuten
	block = block - m * 4500;	// minuten rest
	s = block / 75;				// sekunden
	d = block - s * 75;			// sekunden rest
	
	m = ( ( m / 10 ) << 4 ) | m % 10;
	s = ( ( s / 10 ) << 4 ) | s % 10;
	d = ( ( d / 10 ) << 4 ) | d % 10;	
	
	p[0] = m;
	p[1] = s;
	p[2] = d;
}

#define incTime() \
	time[0] = btoi(time[0]); time[1] = btoi(time[1]); time[2] = btoi(time[2]); \
	time[2]++; \
	if(time[2] == 75) { \
		time[2] = 0; \
		time[1]++; \
		if (time[1] == 60) { \
			time[1] = 0; \
			time[0]++; \
		} \
	} \
	time[0] = itob(time[0]); time[1] = itob(time[1]); time[2] = itob(time[2]);

#define READTRACK() \
	if (CDR_readTrack(time) == -1) return -1; \
	buf = CDR_getBuffer(); if (buf == NULL) return -1;

#define READDIR(_dir) \
	READTRACK(); \
	memcpy(_dir, buf+12, 2048); \
 \
	incTime(); \
	READTRACK(); \
	memcpy(_dir+2048, buf+12, 2048);

int GetCdromFile(u8 *mdir, u8 *time, s8 *filename) {
	struct iso_directory_record *dir;
	char ddir[4096];
	u8 *buf;
	int i;

	i = 0;
	while (i < 4096) {
		dir = (struct iso_directory_record*) &mdir[i];
		if (dir->length[0] == 0) {
			return -1;
		}
		i += dir->length[0];

		if (dir->flags[0] & 0x2) { // it's a dir
			if (!strnicmp((char*)&dir->name[0], filename, dir->name_len[0])) {
				if (filename[dir->name_len[0]] != '\\') continue;
				
				filename+= dir->name_len[0] + 1;

				mmssdd(dir->extent, (char*)time);
				READDIR(ddir);
				i = 0;
			}
		} else {
			if (!strnicmp((char*)&dir->name[0], filename, strlen(filename))) {
				mmssdd(dir->extent, (char*)time);
				break;
			}
		}
	}
	return 0;
}

int LoadCdrom() {
	EXE_HEADER tmpHead;
	struct iso_directory_record *dir;
	u8 time[4],*buf;
	u8 mdir[4096];
	s8 exename[256];
	int i;

	if (!Config.HLE) {
		psxRegs.pc = psxRegs.GPR.n.ra;
		return 0;
	}

	time[0] = itob(0); time[1] = itob(2); time[2] = itob(0x10);

	READTRACK();

	// skip head and sub, and go to the root directory record
	dir = (struct iso_directory_record*) &buf[12+156]; 

	mmssdd(dir->extent, (char*)time);

	READDIR(mdir);

	if (GetCdromFile(mdir, time, "SYSTEM.CNF;1") == -1) {
		if (GetCdromFile(mdir, time, "PSX.EXE;1") == -1) return -1;

		READTRACK();
	}
	else {
		READTRACK();

		sscanf((char*)buf+12, "BOOT = cdrom:\\%s", exename);
		if (GetCdromFile(mdir, time, exename) == -1) {
			sscanf((char*)buf+12, "BOOT = cdrom:%s", exename);
			if (GetCdromFile(mdir, time, exename) == -1) {
				char *ptr = strstr(buf+12, "cdrom:");
				for (i=0; i<32; i++) {
					if (ptr[i] == ' ') continue;
					if (ptr[i] == '\\') continue;
				}
				strcpy(exename, ptr);
				if (GetCdromFile(mdir, time, exename) == -1)
					return -1;
			}
		}

		READTRACK();
	}

	memcpy(&tmpHead, buf+12, sizeof(EXE_HEADER));

#ifdef __MACOSX__
	swapEXE_HEADER(&tmpHead);
#endif

	psxRegs.pc = tmpHead.pc0;
	psxRegs.GPR.n.gp = tmpHead.gp0;
	psxRegs.GPR.n.sp = tmpHead.s_addr; 
	if (psxRegs.GPR.n.sp == 0) psxRegs.GPR.n.sp = 0x801fff00;

	while (tmpHead.t_size) {
		void *ptr = (void *)PSXM(tmpHead.t_addr);

		incTime();
		READTRACK();

		if (ptr != NULL) memcpy(ptr, buf+12, 2048);

		tmpHead.t_size -= 2048;
		tmpHead.t_addr += 2048;
	}

	return 0;
}

int LoadCdromFile(char *filename, EXE_HEADER *head) {
	struct iso_directory_record *dir;
	u8 time[4],*buf;
	u8 mdir[4096], exename[256];
	u32 size, addr;

	sscanf(filename, "cdrom:\\%s", exename);

	time[0] = itob(0); time[1] = itob(2); time[2] = itob(0x10);

	READTRACK();

	// skip head and sub, and go to the root directory record
	dir = (struct iso_directory_record*) &buf[12+156]; 

	mmssdd(dir->extent, (char*)time);

	READDIR(mdir);

	if (GetCdromFile(mdir, time, exename) == -1) return -1;

	READTRACK();

	memcpy(head, buf+12, sizeof(EXE_HEADER));
	size = head->t_size;
	addr = head->t_addr;

	while (size) {
		incTime();
		READTRACK();

		memcpy((void *)PSXM(addr), buf+12, 2048);

		size -= 2048;
		addr += 2048;
	}

	return 0;
}

int CheckCdrom() {
	struct iso_directory_record *dir;
	unsigned char time[4],*buf;
	unsigned char mdir[4096];
	char exename[256];
	int i, c;

	time[0] = itob(0); time[1] = itob(2); time[2] = itob(0x10);

	READTRACK();

	strncpy(CdromLabel, buf+52, 11);

	// skip head and sub, and go to the root directory record
	dir = (struct iso_directory_record*) &buf[12+156]; 

	mmssdd(dir->extent, (char*)time);

	READDIR(mdir);

	if (GetCdromFile(mdir, time, "SYSTEM.CNF;1") != -1) {
		READTRACK();

		sscanf((char*)buf+12, "BOOT = cdrom:\\%s", exename);
		if (GetCdromFile(mdir, time, exename) == -1) {
			sscanf((char*)buf+12, "BOOT = cdrom:%s", exename);
			if (GetCdromFile(mdir, time, exename) == -1) {
				char *ptr = strstr(buf+12, "cdrom:");
				for (i=0; i<32; i++) {
					if (ptr[i] == ' ') continue;
					if (ptr[i] == '\\') continue;
				}
				strcpy(exename, ptr);
				if (GetCdromFile(mdir, time, exename) == -1)
					return 0;
			}
		}
	}

	i = strlen(exename);
	if (i >= 2) {
		if (exename[i - 2] == ';') i-= 2;
		c = 8; i--;
		while (i >= 0 && c >= 0) {
			if (isalnum(exename[i])) CdromId[c--] = exename[i];
			i--;
		}
	}

	if (Config.PsxAuto) { // autodetect system (pal or ntsc)
		if (strstr(exename, "ES") != NULL)
			Config.PsxType = 1; // pal
		else Config.PsxType = 0; // ntsc
	}
	psxUpdateVSyncRate();
	if (CdromLabel[0] == ' ') {
		strcpy(CdromLabel, CdromId);
	}
	SysPrintf("*PCSX*: CdromLabel: %s\n", CdromLabel);
	SysPrintf("*PCSX*: CdromId: %s\n", CdromId);

	return 0;
}

#define PSX_EXE     1
#define CPE_EXE     2
#define COFF_EXE    3
#define INVALID_EXE 4

static int PSXGetFileType(FILE *f) {
    unsigned long current;
    unsigned long mybuf[2048];
    EXE_HEADER *exe_hdr;
    FILHDR *coff_hdr;

    current = ftell(f);
    fseek(f,0L,SEEK_SET);
    fread(mybuf,2048,1,f);
    fseek(f,current,SEEK_SET);

    exe_hdr = (EXE_HEADER *)mybuf;
    if (memcmp(exe_hdr->id,"PS-X EXE",8)==0)
        return PSX_EXE;

    if (mybuf[0]=='C' && mybuf[1]=='P' && mybuf[2]=='E')
        return CPE_EXE;

    coff_hdr = (FILHDR *)mybuf;
    if (coff_hdr->f_magic == 0x0162)
        return COFF_EXE;

    return INVALID_EXE;
}

int Load(char *ExePath) {
	FILE *tmpFile;
	EXE_HEADER tmpHead;
	int type;

	strncpy(CdromId, "SLUS99999", 9);
	strncpy(CdromLabel, "SLUS_999.99", 11);

    tmpFile = fopen(ExePath,"rb");
	if (tmpFile == NULL) { SysMessage(_("Error opening file: %s"), ExePath); return 0; }

    type = PSXGetFileType(tmpFile);
    switch (type) {
    	case PSX_EXE:
	        fread(&tmpHead,sizeof(EXE_HEADER),1,tmpFile);
		    fseek(tmpFile, 0x800, SEEK_SET);		
			fread((void *)PSXM(tmpHead.t_addr), tmpHead.t_size,1,tmpFile);
			fclose(tmpFile);
			psxRegs.pc = tmpHead.pc0;
			psxRegs.GPR.n.gp = tmpHead.gp0;
			psxRegs.GPR.n.sp = tmpHead.s_addr; 
			if (psxRegs.GPR.n.sp == 0) psxRegs.GPR.n.sp = 0x801fff00;
	        break;
    	case CPE_EXE:
    		SysMessage(_("Pcsx found that you wanna use a CPE file. CPE files not supported"));
			break;
    	case COFF_EXE:
    		SysMessage(_("Pcsx found that you wanna use a COFF file. COFF files not supported"));
			break;
    	case INVALID_EXE:
    		SysMessage(_("This file is not a psx file"));
			break;
	}
	return 1;
}

// STATES

const char PcsxHeader[32] = "STv3 PCSX v" PCSX_VERSION;

int SaveState(char *file) {
	gzFile f;
	GPUFreeze_t *gpufP;
	SPUFreeze_t *spufP;
	int Size;
	unsigned char *pMem;

	f = gzopen(file, "wb");
	if (f == NULL) return -1;

	gzwrite(f, (void*)PcsxHeader, 32);

	pMem = (unsigned char *) malloc(128*96*3);
	if (pMem == NULL) return -1;
	GPU_getScreenPic(pMem);
	gzwrite(f, pMem, 128*96*3);
	free(pMem);

	gzwrite(f, psxM, 0x00200000);
	gzwrite(f, psxR, 0x00080000);
	gzwrite(f, psxH, 0x00010000);
	gzwrite(f, (void*)&psxRegs, sizeof(psxRegs));

	// gpu
	gpufP = (GPUFreeze_t *) malloc(sizeof(GPUFreeze_t));
	gpufP->ulFreezeVersion = 1;
	GPU_freeze(1, gpufP);
	gzwrite(f, gpufP, sizeof(GPUFreeze_t));
	free(gpufP);

	// spu
	spufP = (SPUFreeze_t *) malloc(16);
	SPU_freeze(2, spufP);
	Size = spufP->Size; gzwrite(f, &Size, 4);
	free(spufP);
	spufP = (SPUFreeze_t *) malloc(Size);
	SPU_freeze(1, spufP);
	gzwrite(f, spufP, Size);
	free(spufP);

	sioFreeze(f, 1);
	cdrFreeze(f, 1);
	psxHwFreeze(f, 1);
	psxRcntFreeze(f, 1);
	mdecFreeze(f, 1);

	gzclose(f);

	return 0;
}

int LoadState(char *file) {
	gzFile f;
	GPUFreeze_t *gpufP;
	SPUFreeze_t *spufP;
	int Size;
	char header[32];

	f = gzopen(file, "rb");
	if (f == NULL) return -1;

	psxCpu->Reset();

	gzread(f, header, 32);

	if (strncmp("STv3 PCSX", header, 9)) { gzclose(f); return -1; }

	gzseek(f, 128*96*3, SEEK_CUR);

	gzread(f, psxM, 0x00200000);
	gzread(f, psxR, 0x00080000);
	gzread(f, psxH, 0x00010000);
	gzread(f, (void*)&psxRegs, sizeof(psxRegs));

	// gpu
	gpufP = (GPUFreeze_t *) malloc (sizeof(GPUFreeze_t));
	gzread(f, gpufP, sizeof(GPUFreeze_t));
	GPU_freeze(0, gpufP);
	free(gpufP);

	// spu
	gzread(f, &Size, 4);
	spufP = (SPUFreeze_t *) malloc (Size);
	gzread(f, spufP, Size);
	SPU_freeze(0, spufP);
	free(spufP);

	sioFreeze(f, 0);
	cdrFreeze(f, 0);
	psxHwFreeze(f, 0);
	psxRcntFreeze(f, 0);
	mdecFreeze(f, 0);

	gzclose(f);

	return 0;
}

int CheckState(char *file) {
	gzFile f;
	char header[32];

	f = gzopen(file, "rb");
	if (f == NULL) return -1;

	psxCpu->Reset();

	gzread(f, header, 32);

	gzclose(f);

	if (strncmp("STv3 PCSX", header, 9)) return -1;

	return 0;
}

// NET Function Helpers

int SendPcsxInfo() {
	if (NET_recvData == NULL || NET_sendData == NULL)
		return 0;

//	SysPrintf("SendPcsxInfo\n");

	NET_sendData(&Config.Xa, sizeof(Config.Xa), PSE_NET_BLOCKING);
	NET_sendData(&Config.Sio, sizeof(Config.Sio), PSE_NET_BLOCKING);
	NET_sendData(&Config.SpuIrq, sizeof(Config.SpuIrq), PSE_NET_BLOCKING);
	NET_sendData(&Config.RCntFix, sizeof(Config.RCntFix), PSE_NET_BLOCKING);
	NET_sendData(&Config.PsxType, sizeof(Config.PsxType), PSE_NET_BLOCKING);
	NET_sendData(&Config.Cpu, sizeof(Config.Cpu), PSE_NET_BLOCKING);

//	SysPrintf("Send OK\n");

	return 0;
}

int RecvPcsxInfo() {
	int tmp;

	if (NET_recvData == NULL || NET_sendData == NULL)
		return 0;

//	SysPrintf("RecvPcsxInfo\n");

	NET_recvData(&Config.Xa, sizeof(Config.Xa), PSE_NET_BLOCKING);
	NET_recvData(&Config.Sio, sizeof(Config.Sio), PSE_NET_BLOCKING);
	NET_recvData(&Config.SpuIrq, sizeof(Config.SpuIrq), PSE_NET_BLOCKING);
	NET_recvData(&Config.RCntFix, sizeof(Config.RCntFix), PSE_NET_BLOCKING);
	NET_recvData(&Config.PsxType, sizeof(Config.PsxType), PSE_NET_BLOCKING);
	psxUpdateVSyncRate();

	SysUpdate();

	tmp = Config.Cpu;
	NET_recvData(&Config.Cpu, sizeof(Config.Cpu), PSE_NET_BLOCKING);
	if (tmp != Config.Cpu) {
		psxCpu->Shutdown();
#ifdef PSXREC
		if (Config.Cpu)	
			 psxCpu = &psxInt;
		else psxCpu = &psxRec;
#else
		psxCpu = &psxInt;
#endif
		if (psxCpu->Init() == -1) {
			SysClose(); return -1;
		}
		psxCpu->Reset();
	}

//	SysPrintf("Recv OK\n");

	return 0;
}


void __Log(char *fmt, ...) {
	va_list list;
#ifdef LOG_STDOUT
	char tmp[1024];
#endif

	va_start(list, fmt);
#ifndef LOG_STDOUT
	vfprintf(emuLog, fmt, list);
#else
	vsprintf(tmp, fmt, list);
	SysPrintf(tmp);
#endif
	va_end(list);
}

typedef struct {
	char id[8];
	char name[64];
} LangDef;

LangDef sLangs[] = {
	{ "ar", N_("Arabic") },
	{ "ca", N_("Catalan") },
	{ "de", N_("German") },
	{ "el", N_("Greek") },
	{ "en", N_("English") },
	{ "es", N_("Spanish") },
	{ "fr", N_("French") },
	{ "it", N_("Italian") },
	{ "pt", N_("Portuguese") },
	{ "ro", N_("Romanian") },
	{ "ru", N_("Russian") },
	{ "", "" },
};


char *ParseLang(char *id) {
	int i=0;

	while (sLangs[i].id[0] != 0) {
		if (!strcmp(id, sLangs[i].id))
			return _(sLangs[i].name);
		i++;
	}

	return id;
}

