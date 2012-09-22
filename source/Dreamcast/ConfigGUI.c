#include <kos.h>
#include "Dcast.h"
#include "gui.h"
#include "filelst.h"
#include "PlugCD.h"

#define DISABLE 0
#define ENABLE  1

const char* c_dis[] = {"Disabled", "Enabled"};

int esel = 0;

static int select_config = 0;
static float throb = 0.2f, dthrob = 0.01f;

void change_config_cpu(int selected)
{
	switch(selected) {
		case 0: Config.Xa = !Config.Xa; break;
		case 1: Config.Sio = !Config.Sio; break;
		case 2: Config.SpuIrq = !Config.SpuIrq; break;
		case 3: Config.Mdec = !Config.Mdec; break;
		case 4: Config.Cdda = !Config.Cdda; break;
		case 5: Config.Cpu = !Config.Cpu; break;
		case 6: Config.PsxOut = !Config.PsxOut; break;
		case 7: Config.CdTiming = !Config.CdTiming; break;
		case 8: {
			if (Config.PsxAuto) Config.PsxAuto = 0;
			else if (Config.PsxType) Config.PsxType = 0;
			else { Config.PsxAuto = 1; Config.PsxType = 1; }
			}
			break;
	}
}

static uint8 mcont = 0;
int check_config_cpu() {
        static int up_moved = 0, down_moved = 0, a_pressed = 0, y_pressed = 0;

        cont_cond_t cond;

        if (!mcont) {
                mcont = maple_first_controller();
                if (!mcont) { return -1; }
        }
        if (cont_get_cond(mcont, &cond)) { return -1; }

        if (!(cond.buttons & CONT_DPAD_DOWN)) {
           if ((framecnt - down_moved) > 10) {
                  if (esel < 8) {
                       esel++;
                    }
                   down_moved = framecnt;
                }
        }

        if (!(cond.buttons & CONT_DPAD_UP)) {
            if ((framecnt - up_moved) > 10) {
                  if (esel > 0) {
                          esel--;
                   }
                   up_moved = framecnt;
            }
        }

        if (!(cond.buttons & CONT_A)) {
           if ((framecnt - a_pressed) > 10) {
           	change_config_cpu(esel);
               a_pressed = framecnt;
           }
        }

        return 0;
}


void render_config_cpu()
{
	char type_buf[32];
	
	float x1 = 30.0f, y1 = 80.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Disable Xa Decoding:    %s", (Config.Xa?c_dis[DISABLE]:c_dis[ENABLE]));
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Sio Irq Always Enabled: %s", (Config.Sio?c_dis[ENABLE]:c_dis[DISABLE]));
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Spu Irq Always Enabled: %s", (Config.SpuIrq?c_dis[ENABLE]:c_dis[DISABLE]));
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Black & White Movies:   %s", (Config.Mdec?c_dis[ENABLE]:c_dis[DISABLE]));
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Disable Cd Audio:       %s", (Config.Cdda?c_dis[DISABLE]:c_dis[ENABLE]));
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Enable Interpreter Cpu: %s", (Config.Cpu?c_dis[ENABLE]:c_dis[DISABLE]));
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Enable Console Output:  %s", (Config.PsxOut?c_dis[ENABLE]:c_dis[DISABLE]));
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Old Cdrom Timing:       %s", (Config.CdTiming?c_dis[ENABLE]:c_dis[DISABLE]));
	y1 += 28.0f;
	if (Config.PsxAuto) strcpy(type_buf, "Auto");
	else if (Config.PsxType) strcpy(type_buf, "PAL");
	else strcpy(type_buf, "NTSC");
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Psx System Type:        %s", type_buf);

	/* highlight selected */
	x1 = 30.0f; y1 = 80.0f;
    draw_poly_box(x1-10.0f, y1+esel*28.0f - 1.0f,
                  620.0f, y1+esel*28.0f + 25.0f, 95.0f,
                  throb, throb, 0.2f, 0.2f, throb, throb, 0.2f, 0.2f);

	throb += dthrob;
	if (throb < 0.2f || throb > 0.8f) {
		dthrob = -dthrob;
		throb += dthrob;
	}

	check_config_cpu();
}

int check_config_end() {
	static int right_moved = 0, left_moved = 0, a_pressed = 0, y_pressed = 0;

        cont_cond_t cond;

        if (!mcont) {
                mcont = maple_first_controller();
                if (!mcont) { return -1; }
        }
        if (cont_get_cond(mcont, &cond)) { return -1; }

        if (!(cond.buttons & CONT_START)) {
                printf("Pressed start\n");
                return 2;
        }

        if (!(cond.buttons & CONT_B)) {
                return 2;
        }

        if (!(cond.buttons & CONT_DPAD_RIGHT)) {
			if ((framecnt - right_moved) > 10) {
				right_moved = framecnt;
				select_config++;
			}
    	    return 1;
        }

        if (!(cond.buttons & CONT_DPAD_LEFT)) {
			if ((framecnt - left_moved) > 10) {
				left_moved = framecnt;
				select_config--;
			}
    	    return 1;
        }

        return 0;
}

void menu_config_cpu()
{

	while (!check_config_end()) {
		pvr_wait_ready();
		pvr_scene_begin();
		pvr_list_begin(PVR_LIST_OP_POLY);

		/* Opaque list *************************************/
//		bkg_render();

		/* End of opaque list */
		pvr_list_finish();
		pvr_list_begin(PVR_LIST_TR_POLY);

		/* Translucent list ********************************/

		/* Top Banner */
		render_top("Configure CPU");
		render_config_cpu();
		draw_poly_strf(400.0f, 392.0f,100.0f,1.0f,1.0f,1.0f,1.0f, "B  Return");

		/* End of translucent list */
		pvr_list_finish();

		/* Finish the frame *******************************/
		pvr_scene_finish();

		framecnt++;
	}
}

void menu_config_bios()
{
	strcpy(FileList.curdir, Config.BiosDir);
	strcpy(FileList.dn, Config.BiosDir);
	strcpy(FileList.fn, Config.Bios);
	strcpy(FileList.add, "HLE");
	load_file_list();

	while (!check_config_end()) {
		pvr_wait_ready();
		pvr_scene_begin();
		pvr_list_begin(PVR_LIST_OP_POLY);

		/* Opaque list *************************************/
//		bkg_render();

		/* End of opaque list */
		pvr_list_finish();
		pvr_list_begin(PVR_LIST_TR_POLY);

		/* Translucent list ********************************/

		/* Top Banner */
		render_top("Choose BIOS");
		draw_file_list();
		draw_poly_strf(400.0f, 392.0f,100.0f,1.0f,1.0f,1.0f,1.0f, "B  Return");

		/* End of translucent list */
		pvr_list_finish();

		/* Finish the frame *******************************/
		pvr_scene_finish();

		framecnt++;
	}

	// User has chosen his BIOS file
	if (strstr(FileList.dn, "HLE")) strcpy(Config.Bios, "HLE");
	else strcpy(Config.Bios, FileList.fn);
	strcpy(Config.BiosDir, FileList.dn);
	psxReset();	// load new bios
}

void menu_config_cdrom()
{
	strcpy(FileList.curdir, CDConfiguration.dn);
	strcpy(FileList.dn, CDConfiguration.dn);
	strcpy(FileList.fn, CDConfiguration.fn);
	strcpy(FileList.add, "");
	load_file_list();

	while (!check_config_end()) {
		pvr_wait_ready();
		pvr_scene_begin();
		pvr_list_begin(PVR_LIST_OP_POLY);

		/* Opaque list *************************************/
//		bkg_render();

		/* End of opaque list */
		pvr_list_finish();
		pvr_list_begin(PVR_LIST_TR_POLY);

		/* Translucent list ********************************/

		/* Top Banner */
		render_top("Choose CDROM");

		draw_file_list();
		
		draw_poly_strf(400.0f, 392.0f,100.0f,1.0f,1.0f,1.0f,1.0f, "B  Return");

		/* End of translucent list */
		pvr_list_finish();

		/* Finish the frame *******************************/
		pvr_scene_finish();

		framecnt++;
	}

	// User has chosen his CDROM file
	strcpy(CDConfiguration.dn, FileList.dn);
	strcpy(CDConfiguration.fn, FileList.fn);
}

void menu_config()
{
	while (check_config_end()<2) {
		if (select_config > 2) select_config = 0;
		if (select_config < 0) select_config = 2;
		switch (select_config) {
			case 0: menu_config_cpu(); break;
			case 1: menu_config_bios(); break;
			case 2: menu_config_cdrom(); break;
		}
	}
}
