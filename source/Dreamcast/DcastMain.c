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
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <kos.h>
#include <png/png.h>

#include "Dcast.h"
#include "Sio.h"
#include "gui.h"

pvr_ptr_t splash_tex;

/* draw background */
void draw_splash(void)
{
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, 256, 256, splash_tex, PVR_FILTER_BILINEAR);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);    
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;
    
    vert.x = 1;
    vert.y = 1;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = 640;
    vert.y = 1;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = 1;
    vert.y = 480;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));
    
    vert.x = 640;
    vert.y = 480;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 1.0;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));
}

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

int UseGui = 1;
long LoadCdBios;

int main(int argc, char *argv[]) {
	char *file = NULL;
	int runcd = 0;
	int loadst = 0;
	int i;

//	vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);	
	vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);	
	pvr_init_defaults();

	/* Setup the mouse/font texture */
	setup_util_texture();

	gui_disclaimer();
	timer_spin_sleep(8000);

	/* Draws splashscreen */    
    splash_tex = pvr_mem_malloc(256*256*2);
    png_to_texture("/rd/pcsxAbout.png", splash_tex, PNG_NO_ALPHA);
	
    pvr_wait_ready();
    pvr_scene_begin();
    
    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_splash();
    pvr_list_finish();
    pvr_scene_finish();

    pvr_wait_ready();	/* Flip screen to first buffer */
    pvr_scene_begin();
    pvr_scene_finish();

	timer_spin_sleep(2000);
	
	/* Configure pcsx */
	memset(&Config, 0, sizeof(PcsxConfig));
//	strcpy(Config.Bios, "scph1001.bin"); // "HLE"); // 
	strcpy(Config.Bios, "HLE");
	strcpy(Config.BiosDir, "/rd");
//	Config.Cpu = 1;
	Config.Cpu = 0;
	Config.CdTiming = 0;
	Config.PsxOut = 1;
//	Config.HLE = 1;
	
    SysPrintf("start main()\r\n");

	if (SysInit() == -1) return 1;

	/* Start gui */
	menu_start();

	OpenPlugins();
	SysReset();

    SysPrintf("CheckCdrom()\r\n");
	CheckCdrom();
	
    SysPrintf("Load()\r\n");
//	Load("/rd/pdx-dlcm.psx");
	LoadCdrom();
	
    SysPrintf("Execute()\r\n");
	psxCpu->Execute();

	return 0;
}

int SysInit() {
    SysPrintf("start SysInit()\r\n");

    SysPrintf("psxInit()\r\n");
	psxInit();

    SysPrintf("LoadPlugins()\r\n");
	LoadPlugins();
    SysPrintf("LoadMcds()\r\n");
//	LoadMcds(Config.Mcd1, Config.Mcd2);

	SysPrintf("end SysInit()\r\n");
	return 0;
}

void SysReset() {
    SysPrintf("start SysReset()\r\n");
	psxReset();
	SysPrintf("end SysReset()\r\n");
}

void SysClose() {
	psxShutdown();
	ReleasePlugins();

	if (emuLog != NULL) fclose(emuLog);
}

void SysPrintf(char *fmt, ...) {
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	if (Config.PsxOut) printf ("%s", msg);
#if defined (CPU_LOG) || defined(DMA_LOG) || defined(CDR_LOG) || defined(HW_LOG) || \
	defined(BIOS_LOG) || defined(GTE_LOG) || defined(PAD_LOG)
	fprintf(emuLog, "%s", msg);
#endif
}

void *SysLoadLibrary(char *lib) {
//	return dlopen(lib, RTLD_NOW);
}

void *SysLoadSym(void *lib, char *sym) {
//	return dlsym(lib, sym);
}

char *SysLibError() {
//	return dlerror();
}

void SysCloseLibrary(void *lib) {
//	dlclose(lib);
}

void SysUpdate() {
//	PADhandleKey(PAD1_keypressed());
//	PADhandleKey(PAD2_keypressed());
}

void SysRunGui() {
//	RunGui();
}

void SysMessage(char *fmt, ...) {
	
}
