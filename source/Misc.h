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

#ifndef __MISC_H__
#define __MISC_H__

#undef s_addr

typedef struct {
	unsigned char id[8];
    unsigned long text;                   
    unsigned long data;                    
    unsigned long pc0;
    unsigned long gp0;                     
    unsigned long t_addr;
    unsigned long t_size;
    unsigned long d_addr;                  
    unsigned long d_size;                  
    unsigned long b_addr;                  
    unsigned long b_size;                  
    unsigned long s_addr;
    unsigned long s_size;
    unsigned long SavedSP;
    unsigned long SavedFP;
    unsigned long SavedGP;
    unsigned long SavedRA;
    unsigned long SavedS0;
} EXE_HEADER;

char CdromId[9];
char CdromLabel[11];

int LoadCdrom();
int LoadCdromFile(char *filename, EXE_HEADER *head);
int CheckCdrom();
int Load(char *ExePath);

int SaveState(char *file);
int LoadState(char *file);
int CheckState(char *file);

int SendPcsxInfo();
int RecvPcsxInfo();

extern char *LabelAuthors;
extern char *LabelGreets;

char *ParseLang(char *id);

#endif /* __MISC_H__ */
