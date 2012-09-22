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
#include <dlfcn.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <dirent.h>

#include "Linux.h"
#include "Sio.h"


int UseGui = 1;
int CancelQuit = 0;

int main(int argc, char *argv[]) {
	char *file = NULL;
	char *lang;
	int runcd = 0;
	int loadst = 0;
	int i;

#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, "./Langs");
	textdomain(PACKAGE);
#endif
	strcpy(cfgfile, "Pcsx.cfg");

	for (i=1; i<argc; i++) {
		if (!strcmp(argv[i], "-runcd")) runcd = 1;
		else if (!strcmp(argv[i], "-runcdbios")) runcd = 2;
		else if (!strcmp(argv[i], "-nogui")) UseGui = 0;
		else if (!strcmp(argv[i], "-psxout")) Config.PsxOut = 1;
		else if (!strcmp(argv[i], "-load")) loadst = atol(argv[++i]);
		else if (!strcmp(argv[i], "-cfg")) strcpy(cfgfile, argv[++i]);
		else if (!strcmp(argv[i], "-h") ||
			 !strcmp(argv[i], "-help")) {
			 printf("Pcsx " PCSX_VERSION "\n");
			 printf("%s\n", _(
			 				" pcsx [options] [file]\n"
							"\toptions:\n"
							"\t-runcd\t\tRuns CdRom\n"
							"\t-runcdbios\tRuns CdRom Through Bios\n"
							"\t-nogui\t\tDon't open GtkGui\n"
							"\t-cfg FILE\tLoads desired configuration file (def:Pcsx.cfg)\n"
							"\t-psxout\t\tEnable psx output\n"
							"\t-load STATENUM\tLoads savestate STATENUM (1-5)\n"
							"\t-h -help\tThis help\n"
							"\tfile\t\tLoads file\n"));
			 return 0;
		} else file = argv[i];
	}

	memset(&Config, 0, sizeof(PcsxConfig));
	strcpy(Config.Net, _("Disabled"));
	if (LoadConfig() == -1) {
		Config.PsxAuto = 1;
		strcpy(Config.PluginsDir, "Plugin/");
		strcpy(Config.BiosDir,    "Bios/");
		if (!UseGui) {
			printf(_("Pcsx is unable to configure pcsx settings without gtkgui, restart without -nogui\n"));
			return 0;
		}
		gtk_init(NULL, NULL);

		SysMessage(_("Pcsx needs to be configured\n"));
		ConfigurePlugins();
		ConfigureMemcards();
		return 0;
	}
	if (Config.Lang[0] == 0) {
		char *str = setlocale(LC_MESSAGES, NULL);
		char _lang[3];

		strncpy(_lang, str, 2); _lang[2] = 0;
		strcpy(Config.Lang, _lang);
	}

	langs = (_langs*)malloc(sizeof(_langs));
	strcpy(langs[0].lang, "en");
	InitLanguages(); i=1;
	while ((lang = GetLanguageNext()) != NULL) {
		langs = (_langs*)realloc(langs, sizeof(_langs)*(i+1));
		strcpy(langs[i].lang, lang);
		i++;
	}
	CloseLanguages();
	langsMax = i;

	if (UseGui) gtk_init(NULL, NULL);

	if (SysInit() == -1) return 1;

	if (UseGui) {
		StartGui();
		return 0;
	}

	if (OpenPlugins() == -1) return 1;
	SysReset();

	CheckCdrom();

	if (file != NULL) Load(file);
	else {
		if (runcd == 1) {
			LoadCdBios = 0;
			if (LoadCdrom() == -1) {
				ClosePlugins();
				printf(_("Could not load Cdrom\n"));
				return -1;
			}
		} else if (runcd == 2) LoadCdBios = 1;
	}

	if (loadst) {
		char Text[256];
		StatesC = loadst-1;
		sprintf (Text, "sstates/%10.10s.%3.3d", CdromLabel, StatesC);
		LoadState(Text);
	}

	psxCpu->Execute();

	return 0;
}

DIR *dir;

void InitLanguages() {
	dir = opendir("Langs");
}

char *GetLanguageNext() {
	struct dirent *ent;

	if (dir == NULL) return NULL;
	for (;;) {
		ent = readdir(dir);
		if (ent == NULL) return NULL;

		if (!strcmp(ent->d_name, ".")) continue;
		if (!strcmp(ent->d_name, "..")) continue;
		break;
	}

	return ent->d_name;
}

void CloseLanguages() {
	if (dir) closedir(dir);
}

void ChangeLanguage(char *lang) {
	strcpy(Config.Lang, lang);
	SaveConfig();
	LoadConfig();
}

int SysInit() {

#ifdef GTE_DUMP
	gteLog = fopen("gteLog.txt","wb");
	setvbuf(gteLog, NULL, _IONBF, 0);
#endif

#ifdef EMU_LOG
#ifndef LOG_STDOUT
	emuLog = fopen("emuLog.txt","wb");
#else
	emuLog = stdout;
#endif
	setvbuf(emuLog, NULL, _IONBF, 0);
#endif

	psxInit();
	while (LoadPlugins() == -1) {
		if (!UseGui) {
			printf (_("Pcsx is unable to configure pcsx settings without gtkgui, restart without -nogui\n"));
			exit(1);
		}

		CancelQuit = 1;
		ConfigurePlugins();
		CancelQuit = 0;
	}

	LoadMcds(Config.Mcd1, Config.Mcd2);

	return 0;
}

void SysReset() {
	psxReset();
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
#ifdef EMU_LOG
#ifndef LOG_STDOUT
	fprintf(emuLog, "%s", msg);
#endif
#endif
}

void *SysLoadLibrary(char *lib) {
	return dlopen(lib, RTLD_NOW);
}

void *SysLoadSym(void *lib, char *sym) {
	return dlsym(lib, sym);
}

const char *SysLibError() {
	return dlerror();
}

void SysCloseLibrary(void *lib) {
	dlclose(lib);
}

void SysUpdate() {
	PADhandleKey(PAD1_keypressed());
	PADhandleKey(PAD2_keypressed());
}

void SysRunGui() {
	RunGui();
}
