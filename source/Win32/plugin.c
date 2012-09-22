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

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "plugins.h"
#include "resource.h"
#include <time.h>
#include <stdio.h>

#include "R3000A.h"
#include "Win32.h"
#include "NoPic.h"

int ShowPic=0;

void gpuShowPic() {
	char Text[255];
	gzFile f;

	if (!ShowPic) {
		unsigned char *pMem;

		pMem = (unsigned char *) malloc(128*96*3);
		if (pMem == NULL) return;
		sprintf(Text, "sstates\\%10.10s.%3.3d", CdromLabel, StatesC);

		GPU_freeze(2, (GPUFreeze_t *)&StatesC);

		f = gzopen(Text, "rb");
		if (f != NULL) {
			gzseek(f, 32, SEEK_SET); // skip header
			gzread(f, pMem, 128*96*3);
			gzclose(f);
		} else {
			memcpy(pMem, NoPic_Image.pixel_data, 128*96*3);
			DrawNumBorPic(pMem, StatesC+1);
		}
		GPU_showScreenPic(pMem);

		free(pMem);
		ShowPic = 1;
	} else { GPU_showScreenPic(NULL); ShowPic = 0; }
}

void PADhandleKey(int key) {
	char Text[255];
	int ret;

	if (Running == 0) return;
	switch (key) {
		case 0: break;
		case VK_F1:
			sprintf(Text, "sstates\\%10.10s.%3.3d", CdromLabel, StatesC);
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			ret = SaveState(Text);
			if (ret == 0)
				 sprintf(Text, _("*PCSX*: Saved State %d"), StatesC+1);
			else sprintf(Text, _("*PCSX*: Error Saving State %d"), StatesC+1);
			GPU_displayText(Text);
			if (ShowPic) { ShowPic = 0; gpuShowPic(); }
			break;

		case VK_F2:
			if (StatesC < 4) StatesC++;
			else StatesC = 0;
			GPU_freeze(2, (GPUFreeze_t *)&StatesC);
			if (ShowPic) { ShowPic = 0; gpuShowPic(); }
			break;

		case VK_F3:
			sprintf (Text, "sstates\\%10.10s.%3.3d", CdromLabel, StatesC);
			ret = LoadState(Text);
			if (ret == 0)
				 sprintf(Text, _("*PCSX*: Loaded State %d"), StatesC+1);
			else sprintf(Text, _("*PCSX*: Error Loading State %d"), StatesC+1);
			GPU_displayText(Text);
			break;

		case VK_F4:
			gpuShowPic();
			break;

		case VK_F5:
			if (Config.QKeys) break;
			Config.Sio ^= 0x1;
			if (Config.Sio)
				 sprintf(Text, _("*PCSX*: Sio Irq Always Enabled"));
			else sprintf(Text, _("*PCSX*: Sio Irq Not Always Enabled"));
			GPU_displayText(Text);
			break;

		case VK_F6:
			if (Config.QKeys) break;
			Config.Mdec ^= 0x1;
			if (Config.Mdec)
				 sprintf(Text, _("*PCSX*: Black&White Mdecs Only Enabled"));
			else sprintf(Text, _("*PCSX*: Black&White Mdecs Only Disabled"));
			GPU_displayText(Text);
			break;

		case VK_F7:
			if (Config.QKeys) break;
			Config.Xa ^= 0x1;
			if (Config.Xa == 0)
				 sprintf (Text, _("*PCSX*: Xa Enabled"));
			else sprintf (Text, _("*PCSX*: Xa Disabled"));
			GPU_displayText(Text);
			break;

		case VK_F8:
			if (Config.QKeys) break;
			GPU_makeSnapshot();
			return;

		case VK_F9:
			GPU_displayText(_("*PCSX*: CdRom Case Opened"));
			cdOpenCase = 1;
			break;

		case VK_F10:
			GPU_displayText(_("*PCSX*: CdRom Case Closed"));
			cdOpenCase = 0;
			break;

		case VK_F12:
			SysPrintf("*PCSX*: CpuReset\n");
			psxCpu->Reset();
			break;

		case VK_ESCAPE:
			Running = 0;
			ClosePlugins();
			SysRunGui();
			break;
	}
}

void CALLBACK SPUirq(void);

char charsTable[4] = { "|/-\\" };

BOOL CALLBACK ConnectDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char str[256];
	static int waitState;

	switch(uMsg) {
		case WM_INITDIALOG:
			SetWindowText(hW, _("Connecting..."));

			sprintf(str, _("Please wait while connecting... %c\n"), charsTable[waitState]);
			Static_SetText(GetDlgItem(hW, IDC_CONNECTSTR), str);
			SetTimer(hW, 0, 100, NULL);
			return TRUE;

		case WM_TIMER:
			if (++waitState == 4) waitState = 0;
			sprintf(str, _("Please wait while connecting... %c\n"), charsTable[waitState]);
			Static_SetText(GetDlgItem(hW, IDC_CONNECTSTR), str);
			return TRUE;

/*		case WM_COMMAND:
			switch (LOWORD(wParam)) {
       			case IDCANCEL:
					WaitCancel = 1;
					return TRUE;
			}*/
	}

	return FALSE;
}

int NetOpened = 0;

int OpenPlugins(HWND hWnd) {
	int ret;

	GPU_clearDynarec(clearDynarec);

	ret = CDR_open();
	if (ret < 0) { SysMessage (_("Error Opening CDR Plugin")); return -1; }

	SetCurrentDirectory(PcsxDir);
	if (Config.UseNet && NetOpened == 0) {
		netInfo info;
		char path[256];

		strcpy(info.EmuName, "PCSX v" PCSX_VERSION);
		strncpy(info.CdromID, CdromId, 9);
		strncpy(info.CdromLabel, CdromLabel, 9);
		info.psxMem = psxM;
		info.GPU_showScreenPic = GPU_showScreenPic;
		info.GPU_displayText = GPU_displayText;
		info.GPU_showScreenPic = GPU_showScreenPic;
		info.PAD_setSensitive = PAD1_setSensitive;
		sprintf(path, "%s%s", Config.BiosDir, Config.Bios);
		strcpy(info.BIOSpath, path);
		strcpy(info.MCD1path, Config.Mcd1);
		strcpy(info.MCD2path, Config.Mcd2);
		sprintf(path, "%s%s", Config.PluginsDir, Config.Gpu);
		strcpy(info.GPUpath, path);
		sprintf(path, "%s%s", Config.PluginsDir, Config.Spu);
		strcpy(info.SPUpath, path);
		sprintf(path, "%s%s", Config.PluginsDir, Config.Cdr);
		strcpy(info.CDRpath, path);
		NET_setInfo(&info);

		ret = NET_open(hWnd);
		if (ret < 0) Config.UseNet = 0;
		else {
			HWND hW = CreateDialog(gApp.hInstance, MAKEINTRESOURCE(IDD_CONNECT), gApp.hWnd, ConnectDlgProc);
			ShowWindow(hW, SW_SHOW);

			if (NET_queryPlayer() == 1) {
				if (SendPcsxInfo() == -1) Config.UseNet = 0;
			} else {
				if (RecvPcsxInfo() == -1) Config.UseNet = 0;
			}

			DestroyWindow(hW);
		}
		NetOpened = 1;
	} else if (Config.UseNet) {
		NET_resume();
	}

	ret = GPU_open(hWnd);
	if (ret < 0) { SysMessage (_("Error Opening GPU Plugin (%d)"), ret); return -1; }
	ret = SPU_open(hWnd);
	if (ret < 0) { SysMessage (_("Error Opening SPU Plugin (%d)"), ret); return -1; }
	SPU_registerCallback(SPUirq);
	ret = PAD1_open(hWnd);
	if (ret < 0) { SysMessage (_("Error Opening PAD1 Plugin (%d)"), ret); return -1; }
	ret = PAD2_open(hWnd);
	if (ret < 0) { SysMessage (_("Error Opening PAD2 Plugin (%d)"), ret); return -1; }

	SetCurrentDirectory(PcsxDir);
	ShowCursor(FALSE);
	return 0;
}

void ClosePlugins() {
	int ret;

	UpdateMenuSlots();
	ret = CDR_close();
	if (ret < 0) { SysMessage (_("Error Closing CDR Plugin")); return; }
	ret = GPU_close();
	if (ret < 0) { SysMessage (_("Error Closing GPU Plugin")); return; }
	ret = SPU_close();
	if (ret < 0) { SysMessage (_("Error Closing SPU Plugin")); return; }
	ret = PAD1_close();
	if (ret < 0) { SysMessage (_("Error Closing PAD1 Plugin")); return; }
	ret = PAD2_close();
	if (ret < 0) { SysMessage (_("Error Closing PAD2 Plugin")); return; }

	if (Config.UseNet) {
		NET_pause();
	}
}

void ResetPlugins() {
	int ret;

	CDR_shutdown();
	GPU_shutdown();
	SPU_shutdown();
	PAD1_shutdown();
	PAD2_shutdown();
	if (Config.UseNet) NET_shutdown(); 

	ret = CDR_init();
	if (ret != 0) { SysMessage (_("CDRinit error: %d"), ret); return; }
	ret = GPU_init();
	if (ret != 0) { SysMessage (_("GPUinit error: %d"), ret); return; }
	ret = SPU_init();
	if (ret != 0) { SysMessage (_("SPUinit error: %d"), ret); return; }
	ret = PAD1_init(1);
	if (ret != 0) { SysMessage (_("PAD1init error: %d"), ret); return; }
	ret = PAD2_init(2);
	if (ret != 0) { SysMessage (_("PAD2init error: %d"), ret); return; }
	if (Config.UseNet) {
		ret = NET_init();
		if (ret < 0) { SysMessage (_("NETinit error: %d"), ret); return; }
	}

	NetOpened = 0;
}


