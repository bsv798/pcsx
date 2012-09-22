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

#include "plugins.h"
#include "Spu.h"

void RunGui();
void OnFile_Exit();

extern GPUopen GPU_open;

unsigned long gpuDisp;

extern CDRplay CDR_play;
extern CDRstop CDR_stop;

// extern SPUopen SPU_open;

extern PADopen PAD1_open;
extern PADreadPort1 PAD1_readPort1;
extern PADopen PAD2_open;
extern PADreadPort2 PAD2_readPort2;

int StatesC = 0;
extern char CdromLabel[256];
extern int UseGui;
int cdOpenCase = 0;

/*
void PADhandleKey(int key) {
	char Text[255];
	int ret;

	switch (key) {
		case 0: break;
		case XK_F1:
			sprintf (Text, "sstates/%s.%3.3d", CdromLabel, StatesC);
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			ret = SaveState(Text);
			sprintf (Text, "*PCSX*: %s State %d", !ret ? "Saved" : "Error Saving", StatesC+1);
			GPU_displayText(Text);
			break;
		case XK_F2:
			if (StatesC < 4) StatesC++;
			else StatesC = 0;
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			break;
		case XK_F3:			
			sprintf (Text, "sstates/%s.%3.3d", CdromLabel, StatesC);
			ret = LoadState(Text);
			sprintf (Text, "*PCSX*: %s State %d", !ret ? "Loaded" : "Error Loading", StatesC+1);
			GPU_displayText(Text);
			break;
		case XK_F4:
			{
			gzFile f;
			static int ShowPic;

			if (!ShowPic) {
				unsigned char *pMem;

				sprintf (Text, "sstates/%s.%3.3d", CdromLabel, StatesC);
				f = gzopen(Text, "rb");
				if (f == NULL) break;

				gzseek(f, 32, SEEK_SET); // skip header

				pMem = (unsigned char *) malloc(128*96*3);
				gzread(f, pMem, 128*96*3);
				gzclose(f);
				GPU_freeze(2, (GPUFreeze_t *)&StatesC);
				GPU_showScreenPic(pMem);
				free(pMem);
				ShowPic = 1;
			}
			else { GPU_showScreenPic(NULL); ShowPic = 0; }
			}
			break;
		case XK_F5:
			Config.Sio ^= 0x1;
			sprintf (Text, "*PCSX*: Sio Irq %sAlways Enabled", Config.Sio ? "" : "Not ");
			GPU_displayText(Text);
			break;
		case XK_F6:
			Config.Mdec ^= 0x1;
			sprintf (Text, "*PCSX*: Black&White Mdecs Only %sabled", Config.Mdec ? "En" : "Dis");
			GPU_displayText(Text);
			break;
		case XK_F7:
			Config.Xa ^= 0x1;
			sprintf (Text, "*PCSX*: Xa %sabled", !Config.Xa ? "En" : "Dis");
			GPU_displayText(Text);
			break;
		case XK_F8:
			GPU_makeSnapshot();
			break;
		case XK_F9:
			cdOpenCase = 1;
			break;
		case XK_F10:
			cdOpenCase = 0;
			break;
		case XK_Escape:
			ClosePlugins();
			if (!UseGui) OnFile_Exit();
			RunGui();
			break;
		default: GPU_keypressed(key);
	}
}
*/

long PAD1__open(void) {
	return PAD1_open(&gpuDisp);
}

long PAD2__open(void) {
	return PAD2_open(&gpuDisp);
}

void SignalExit(int sig) {
	ClosePlugins();
//	OnFile_Exit();
}

void SPUirq(void);

void OpenPlugins() {
	int ret;

    SysPrintf("start OpenPlugins()\r\n");

	SysPrintf("CDR_open()\r\n");
	ret = CDR_open();
	if (ret != 0) { SysMessage ("Error Opening CDR Plugin\n"); exit(1); }
	SysPrintf("SPU_open()\r\n");
	ret = SPU_open();
	if (ret != 0) { SysMessage ("Error Opening SPU Plugin\n"); exit(1); }
	SPU_registerCallback(SPUirq);
	SysPrintf("GPU_open()\r\n");
	ret = GPU_open(&gpuDisp, "P©SX", NULL);
	if (ret != 0) { SysMessage ("Error Opening GPU Plugin\n"); exit(1); }
	SysPrintf("PAD1_open()\r\n");
	ret = PAD1_open(&gpuDisp);
	if (ret != 0) { SysMessage ("Error Opening PAD1 Plugin\n"); exit(1); }
	SysPrintf("PAD2_open()\r\n");
	ret = PAD2_open(&gpuDisp);
	if (ret != 0) { SysMessage ("Error Opening PAD2 Plugin\n"); exit(1); }
	SysPrintf("end OpenPlugins()\r\n");
}

void ClosePlugins() {
	int ret;

	ret = CDR_close();
	if (ret != 0) { SysMessage ("Error Closing CDR Plugin\n"); exit(1); }
	ret = SPU_close();
	if (ret != 0) { SysMessage ("Error Closing SPU Plugin\n"); exit(1); }
	ret = PAD1_close();
	if (ret != 0) { SysMessage ("Error Closing PAD1 Plugin\n"); exit(1); }
	ret = PAD2_close();
	if (ret != 0) { SysMessage ("Error Closing PAD2 Plugin\n"); exit(1); }
	ret = GPU_close();
	if (ret != 0) { SysMessage ("Error Closing GPU Plugin\n"); exit(1); }
}

void ResetPlugins() {
	int ret;

	CDR_shutdown();
	GPU_shutdown();
	SPU_shutdown();
	PAD1_shutdown();
	PAD2_shutdown();

	ret = CDR_init();
	if (ret != 0) { SysMessage ("CDRinit error : %d\n",ret); exit(1); }
	ret = GPU_init();
	if (ret != 0) { SysMessage ("GPUinit error : %d\n",ret); exit(1); }
	ret = SPU_init();
	if (ret != 0) { SysMessage ("SPUinit error : %d\n",ret); exit(1); }
	ret = PAD1_init(1);
	if (ret != 0) { SysMessage ("PAD1init error : %d\n",ret); exit(1); }
	ret = PAD2_init(2);
	if (ret != 0) { SysMessage ("PAD2init error : %d\n",ret); exit(1); }
}
