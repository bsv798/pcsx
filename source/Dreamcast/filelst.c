/* GhettoPlay: an S3M browser and playback util
   (c)2000 Dan Potter
*/

#include <string.h>
#include "gui.h"
#include "PsxCommon.h"
#include "filelst.h"

/* Takes care of the song menu */


/* Song menu choices */
typedef struct {
	char	fn[16];
	int	size;
} entry;

char workstring[256] = "";

static entry entries[200];
static int num_entries = 0;
static int selected = 0, top = 0;

static float throb = 0.2f, dthrob = 0.01f;

void load_file_list() {
	file_t d;

	d = fs_open(FileList.curdir, O_RDONLY | O_DIR);
	if (!d) {
		strcpy(FileList.curdir, "/");
		d = fs_open(FileList.curdir, O_RDONLY | O_DIR);
		if (!d) {
			num_entries = 1;
			strcpy(entries[0].fn,"Error!");
			entries[0].size = 0;
			return;
		}
	}
	{
		dirent_t *de;
		num_entries = 0;
		if (strcmp(FileList.curdir, "/")) {
			strcpy(entries[0].fn, "<..>"); entries[0].size = -1;
			num_entries++;
		} else {
			if (strlen(FileList.add)) {
				strcpy(entries[0].fn, FileList.add); entries[0].size = 0;
				num_entries++;
			}
		}
		while ( (de = fs_readdir(d)) && num_entries < 200) {
			printf("read entry '%s'\n", de->name);
			if (!strcmp("..", de->name)) continue;
			if (!strcmp(".", de->name)) continue;
			strcpy(entries[num_entries].fn, de->name);
			entries[num_entries].size = de->size;
			num_entries++;
		}
	}
	fs_close(d);
}

/* Handle controller input */
static uint8 mcont = 0;
void check_controller() {
	static int up_moved = 0, down_moved = 0, a_pressed = 0;
	cont_cond_t cond;

	if (!mcont) {
		mcont = maple_first_controller();
		if (!mcont) { return; }
	}
	if (cont_get_cond(mcont, &cond)) { return; }

	if (!(cond.buttons & CONT_DPAD_UP)) {
		if ((framecnt - up_moved) > 10) {
			if (selected > 0) {
				selected--;
				if (selected < top) {
					top = selected;
				}
			}
			up_moved = framecnt;
		}
	}
	if (!(cond.buttons & CONT_DPAD_DOWN)) {
		if ((framecnt - down_moved) > 10) {
			if (selected < (num_entries - 1)) {
				selected++;
//				if (selected >= (top+14)) {
				if (selected >= (top+10)) {
					top++;
				}
			}
			down_moved = framecnt;
		}
	}
	if (cond.ltrig > 0) {
		if ((framecnt - up_moved) > 10) {
//			selected -= 14;
			selected -= 10;

			if (selected < 0) selected = 0;
			if (selected < top) top = selected;
			up_moved = framecnt;
		}
	}
	if (cond.rtrig > 0) {
		if ((framecnt - down_moved) > 10) {
//			selected += 14;
			selected += 10;
			if (selected > (num_entries - 1))
				selected = num_entries - 1;
//			if (selected >= (top+14))
			if (selected >= (top+10))
				top = selected;
			down_moved = framecnt;
		}
	}
	if (!(cond.buttons & CONT_A)) {
		if ((framecnt - a_pressed) > 10)
		{
			if (!strcmp(entries[selected].fn, "Error!"))
			{
				num_entries = 0;
				load_file_list();
			}
			else if (entries[selected].size >= 0)
			{
				strcpy(FileList.dn,FileList.curdir);
				strcpy(FileList.fn,entries[selected].fn);
				FileList.size = entries[selected].size;
			}
			else
			{
				if (!strcmp(entries[selected].fn, "<..>"))
				{
					int i;
					for (i=strlen(FileList.curdir); i>0; i--)
					{
						if (FileList.curdir[i] == '/')
						{
							FileList.curdir[i] = 0;
							break;
						}
						else
						{
							FileList.curdir[i] = 0;
						}
					}
				} 
				else
				{
					if (strcmp(FileList.curdir, "/"))
					strcat(FileList.curdir, "/");
					strcat(FileList.curdir, entries[selected].fn);
				}
				selected = top = num_entries = 0;
				printf("current directory is now '%s'\n", FileList.curdir);
				load_file_list();
			}
		}
		a_pressed = framecnt;
	}
}

/* Draws the song listing */
void draw_file_list() {
	float y = 92.0f;
	int i, esel;

//	printf("DEBUG: List size %d\r\n",lst_size);

	/* Draw a background box */
	draw_poly_box(30.0f, 80.0f, 610.0f, 440.0f-96.0f, 90.0f, 
		0.2f, 0.8f, 0.5f, 0.0f, 0.2f, 0.8f, 0.8f, 0.2f);

	/* Draw all the song titles */	
//	for (i=0; i<14 && (top+i)<num_entries; i++) {
	for (i=0; i<10 && (top+i)<num_entries; i++) {
		draw_poly_strf(32.0f, y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			entries[top+i].fn);
		if (entries[top+i].size >= 0) {
			draw_poly_strf(32.0f+180.0f, y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"%d bytes", entries[top+i].size);
		} else {
			draw_poly_strf(32.0f+180.0f, y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f, "<DIR>");
		}
		y += 24.0f;
	}
	/* Put a highlight bar under one of them */
	esel = (selected - top);
	draw_poly_box(31.0f, 92.0f+esel*24.0f - 1.0f,
		609.0f, 92.0f+esel*24.0f + 25.0f, 95.0f,
		throb, throb, 0.2f, 0.2f, throb, throb, 0.2f, 0.2f);

	strcpy(workstring, "Selected: ");	
	strcat(workstring, FileList.dn);
	strcat(workstring, "/");
	strcat(workstring, FileList.fn);
	draw_poly_strf(32.0f, 356.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f, workstring);
	workstring[0] = 0;
		
	/* Adjust the throbber */
	throb += dthrob;
	if (throb < 0.2f || throb > 0.8f) {
		dthrob = -dthrob;
		throb += dthrob;
	}

	check_controller();
}
