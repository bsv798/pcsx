#ifndef __FILELST_H
#define __FILELST_H

typedef struct {
	char	curdir[128];
	char	add[128];	// additional entry in the root directory
	char	fn[16];	// selected filename
	char	dn[128];	// selected dirname
	int	 size;	// selected size
} flist;

flist FileList;

void load_file_list();
void draw_file_list();

#endif
