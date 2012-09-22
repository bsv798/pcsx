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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Linux.h"

#define GetValue(name, var) \
	tmp = strstr(data, name); \
	if (tmp != NULL) { \
		tmp+=strlen(name); \
		while ((*tmp == ' ') || (*tmp == '=')) tmp++; \
		if (*tmp != '\n') sscanf(tmp, "%s", var); \
	}

#define GetValuel(name, var) \
	tmp = strstr(data, name); \
	if (tmp != NULL) { \
		tmp+=strlen(name); \
		while ((*tmp == ' ') || (*tmp == '=')) tmp++; \
		if (*tmp != '\n') sscanf(tmp, "%lx", &var); \
	}

#define SetValue(name, var) \
	fprintf (f,"%s = %s\n", name, var);

#define SetValuel(name, var) \
	fprintf (f,"%s = %lx\n", name, var);

int LoadConfig(PcsxConfig *Conf) {
	struct stat buf;
	FILE *f;
	int size;
	char *data,*tmp;

	if (stat(cfgfile, &buf) == -1) {
		char file[256];

		strcpy(file, getenv("HOME")); strcat(file, "/");
		strcat(file, cfgfile);
		strcpy(cfgfile, file);
		if (stat(cfgfile, &buf) == -1) {
			return -1;
		}
	}
	size = buf.st_size;

	f = fopen(cfgfile,"r");
	if (f == NULL) return -1;

	data = (char*)malloc(size);
	if (data == NULL) return -1;

	fread(data, 1, size, f);
	fclose(f);

	GetValue("Bios", Config.Bios);
	GetValue("Gpu",  Config.Gpu);
	GetValue("Spu",  Config.Spu);
	GetValue("Cdr",  Config.Cdr);
	GetValue("Pad1", Config.Pad1);
	GetValue("Pad2", Config.Pad2);
	GetValue("Net",  Config.Net);
	GetValue("Mcd1", Config.Mcd1);
	GetValue("Mcd2", Config.Mcd2);
	GetValue("PluginsDir", Config.PluginsDir);
	GetValue("BiosDir",    Config.BiosDir);
	GetValuel("Xa",      Config.Xa);
	GetValuel("Sio",     Config.Sio);
	GetValuel("Mdec",    Config.Mdec);
	GetValuel("PsxAuto", Config.PsxAuto);
	GetValuel("PsxType", Config.PsxType);
	GetValuel("Cdda",    Config.Cdda);
	GetValuel("Cpu",     Config.Cpu);
	GetValuel("PsxOut",  Config.PsxOut);
	GetValuel("SpuIrq",  Config.SpuIrq);
	GetValuel("RCntFix", Config.RCntFix);
	GetValuel("VSyncWA", Config.VSyncWA);
	Config.Lang[0] = 0;
	GetValue("Lang", Config.Lang);

	free(data);

#ifdef ENABLE_NLS
	if (Config.Lang[0]) {
		extern int _nl_msg_cat_cntr;

		setenv("LANGUAGE", Config.Lang, 1);
		++_nl_msg_cat_cntr;
	}
#endif

	return 0;
}

/////////////////////////////////////////////////////////

void SaveConfig() {
	FILE *f;

	f = fopen(cfgfile,"w");
	if (f == NULL) return;

	SetValue("Bios", Config.Bios);
	SetValue("Gpu",  Config.Gpu);
	SetValue("Spu",  Config.Spu);
	SetValue("Cdr",  Config.Cdr);
	SetValue("Net",  Config.Net);
	SetValue("Pad1", Config.Pad1);
	SetValue("Pad2", Config.Pad2);
	SetValue("Mcd1", Config.Mcd1);
	SetValue("Mcd2", Config.Mcd2);
	SetValue("PluginsDir", Config.PluginsDir);
	SetValue("BiosDir",    Config.BiosDir);
	SetValuel("Xa",      Config.Xa);
	SetValuel("Sio",     Config.Sio);
	SetValuel("Mdec",    Config.Mdec);
	SetValuel("PsxAuto", Config.PsxAuto);
	SetValuel("PsxType", Config.PsxType);
	SetValuel("Cdda",    Config.Cdda);
	SetValuel("Cpu",     Config.Cpu);
	SetValuel("PsxOut",  Config.PsxOut);
	SetValuel("SpuIrq",  Config.SpuIrq);
	SetValuel("RCntFix", Config.RCntFix);
	SetValuel("VSyncWA", Config.VSyncWA);
	SetValue("Lang",    Config.Lang);

	fclose(f);
}

