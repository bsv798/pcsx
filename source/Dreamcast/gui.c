#include <kos.h>
#include "Dcast.h"
#include "gui.h"
#include "PlugCD.h"

const char* cd_start[] = {"Run CD", "Run CD (Bios)", "Run %s"};

static uint8 mcont = 0;
int check_start() {
        cont_cond_t cond;

        if (!mcont) {
                mcont = maple_first_controller();
                if (!mcont) { return -1; }
        }
        if (cont_get_cond(mcont, &cond)) { return -1; }

        if (!(cond.buttons & CONT_START)) {// exit
                return -1;
        }

        if (!(cond.buttons & CONT_A)) {	// start
                return 1;
        }

        if (!(cond.buttons & CONT_Y)) {	// config
                return 2;
        }

        if (!(cond.buttons & CONT_X)) {	// about
                return 3;
        }

	return 0;
}
/*
int check_start() {
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
                if (st->buttons & CONT_START) {
                        printf("Pressed start\n");
                        return 1;
                }
        MAPLE_FOREACH_END()

        return 0;
}
*/

int check_about() {
        cont_cond_t cond;

        if (!mcont) {
                mcont = maple_first_controller();
                if (!mcont) { return -1; }
        }
        if (cont_get_cond(mcont, &cond)) { return -1; }

        if (!(cond.buttons & CONT_START)) {// exit
                return -1;
        }

        if (!(cond.buttons & CONT_B)) {	// return
                return 1;
        }

	return 0;
}

void menu_about()
{
	int mloop = 0;
	while (!mloop) {
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
		float x1 = (640.0f - strlen("Credits")*12.0) / 2, y1;
		
		draw_poly_strf(x1, 20.0f,100.0f,1.0f,1.0f,1.0f,1.0f, "Credits");

		x1 = 30.0f; y1 = 56.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"PCSX written by:");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"main coder: linuzappz");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"co-coders: shadow");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"ex-coders: Nocomp, Pete Bernett, nik3d");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"webmaster: AkumaX");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Greets to: Duddie, Tratax, Kazzuya, JNS,");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"           Bobbi, Psychojak and Shunt");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Special thanks to: Twin, Roor, calb, now3d,");
		y1 += 28.0f;
		draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"  Dan Potter and the other people from KOS");

		draw_poly_strf(456.0f, 308.0f,100.0f,1.0f,1.0f,1.0f,1.0f, "B  Return");
		
		/* End of translucent list */
		pvr_list_finish();

		/* Finish the frame *******************************/
		pvr_scene_finish();
		
		mloop = check_about();
	}
}

/* Draws the top banner (pvr_list_begin(PVR_LIST_TR_POLY) already called) */
void render_top(char *title)
{
	float x1 = (640.0f - strlen(title)*12.0) / 2;

	/* Top Banner */
	draw_poly_box(0.0f, 10.0f, 640.0f, 20.0f+(24.0f*1.0f)+10.0f, 90.0f, 
		0.3f, 0.2f, 0.5f, 0.0f, 0.5f, 0.1f, 0.8f, 0.2f);

	draw_poly_strf(x1, 20.0f,100.0f,1.0f,1.0f,1.0f,1.0f, title);
}

void render_startmenu()
{
	float x1 = 400.0f, y1 = 252.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"A  Start");
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Y  Configuration");
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"X  About");
}

void render_bottom()
{
	/* Configuration Information */
	draw_poly_box(20.0f, 440.0f-96.0f+4, 640.0f-20.0f, 440.0f, 90.0f, 
		0.3f, 0.2f, 0.5f, 0.0f, 0.5f, 0.1f, 0.8f, 0.2f);
                
	draw_poly_strf(30.0f,440.0f-96.0f+6,            100.0f,1.0f,1.0f,1.0f,1.0f,"Configuration");
	draw_poly_strf(30.0f,440.0f-96.0f+6+24.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"Start: %s", cd_start[0]);
	draw_poly_strf(320.0f,440.0f-96.0f+6+24.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"Memcards: ");
	draw_poly_strf(30.0f,440.0f-96.0f+6+48.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"BIOS:  %s", Config.Bios);
	draw_poly_strf(320.0f,440.0f-96.0f+6+48.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"CDROM: %s", CDConfiguration.fn);
}

void menu_start()
{
	framecnt = 0;
	int mloop = 0;
	while (!mloop) {
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
		render_top("PcSX Version " PCSX_VERSION);

		render_startmenu();

		render_bottom();
		
		/* End of translucent list */
		pvr_list_finish();

		/* Finish the frame *******************************/
		pvr_scene_finish();
		
		mloop = check_start();
		switch (mloop) {
			case 1: break;	// start
			case 2: menu_config(); mloop = 0; break;
			case 3: menu_about(); mloop = 0; break;
		}
	}
}

void gui_disclaimer()
{
	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_OP_POLY);

	/* Opaque list *************************************/
//		bkg_render();

	/* End of opaque list */
	pvr_list_finish();
	pvr_list_begin(PVR_LIST_TR_POLY);

	/* Translucent list ********************************/

	float x1 = 20.0f, y1 = 56.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"This program was not actually produced by");
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"or under license from Sega Enterprises(tm).");
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"It was produced using KallistiOS from");
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"Cryptic Allusion. For more information,");
	y1 += 28.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"please visit");
	y1 += 56.0f;
	draw_poly_strf(x1,y1,100.0f,1.0f,1.0f,1.0f,1.0f,"       http://dcdev.allusion.net/");
		
	/* End of translucent list */
	pvr_list_finish();
	pvr_scene_finish();

    pvr_wait_ready();	/* Flip screen to first buffer */
    pvr_scene_begin();
    pvr_scene_finish();
	
}
