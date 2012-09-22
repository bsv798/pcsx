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
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <signal.h>
#include <sys/time.h>

#include "Linux.h"
#include "plugins.h"
#include "Sio.h"
#include "GladeGui.h"
#include "GladeFuncs.h"

extern int UseGui;
long LoadCdBios;
static int needreset = 1;

PSEgetLibType		PSE_getLibType = NULL;
PSEgetLibVersion	PSE_getLibVersion = NULL;
PSEgetLibName		PSE_getLibName = NULL;

// Helper Functions
void FindPlugins();
void FindNetPlugin();

// Functions Callbacks
/*make them getting the right params, will be used later*/
void OnFile_RunExe();
void OnFile_RunCd();
void OnFile_RunCdBios();
void OnFile_Exit();
void OnEmu_Run();
void OnEmu_Reset();
void OnConf_Gpu();
void OnConf_Spu();
void OnConf_Cdr();
void OnConf_Pads();
void OnConf_Mcds();
void OnConf_Cpu();
void OnConf_Conf();
void OnLanguage(GtkMenuItem *menuitem, gpointer user_data);
void OnHelp_Help();
void OnHelp_About();

GtkWidget *Window = NULL;
GtkWidget *ConfDlg;
GtkWidget *AboutDlg;
GtkWidget *FileSel;

GtkAccelGroup *AccelGroup;

typedef struct {
	GtkWidget *Combo;
	GList *glist;
	char plist[255][255];
	int plugins;
} PluginConf;

PluginConf GpuConfS;
PluginConf SpuConfS;
PluginConf CdrConfS;
PluginConf Pad1ConfS;
PluginConf Pad2ConfS;
PluginConf NetConfS;
PluginConf BiosConfS;

#define FindComboText(combo,list,conf) \
	if (strlen(conf) > 0) { \
		int i; \
		for (i=2;i<255;i+=2) { \
			if (!strcmp(conf, list[i-2])) { \
				gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), list[i-1]); \
				break; \
			} \
		} \
	}

#define GetComboText(combo,list,conf) \
	{ \
	int i; \
	const gchar *tmp = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry)); \
	for (i=2;i<255;i+=2) { \
		if (!strcmp(tmp, list[i-1])) { \
			strcpy(conf, list[i-2]); \
			break; \
		} \
	} \
	}

int Slots[5] = { -1, -1, -1, -1, -1 };

void ResetMenuSlots() {
	GtkWidget *Item;
	char str[256];
	int i;

	for (i=0; i<5; i++) {
		sprintf(str, "GtkMenuItem_LoadSlot%d", i+1);
		Item = lookup_widget(Window, str);
		if (Slots[i] == -1) 
			gtk_widget_set_sensitive(Item, FALSE);
		else
			gtk_widget_set_sensitive(Item, TRUE);
	}
}

void UpdateMenuSlots() {
	char str[256];
	int i;

	for (i=0; i<5; i++) {
		sprintf(str, "sstates/%10.10s.%3.3d", CdromLabel, i);
		Slots[i] = CheckState(str);
	}
}

void StartGui() {
#ifdef ENABLE_NLS
	GtkWidget *Menu;
	GtkWidget *Item;
	int i;
#endif

	add_pixmap_directory("./.pixmaps");
	Window = create_MainWindow();
	gtk_window_set_title(GTK_WINDOW(Window), "PCSX");
	ResetMenuSlots();

#ifdef ENABLE_NLS
	Item = lookup_widget(Window, "GtkMenuItem_Language");
	Menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(Item), Menu);

	for (i=0; i<langsMax; i++) {
		Item = gtk_check_menu_item_new_with_label(ParseLang(langs[i].lang));
		gtk_widget_show(Item);
		gtk_container_add(GTK_CONTAINER(Menu), Item);
		gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(Item), TRUE);
		if (!strcmp(Config.Lang, langs[i].lang))
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(Item), TRUE);

		gtk_signal_connect(GTK_OBJECT(Item), "activate",
                           GTK_SIGNAL_FUNC(OnLanguage),
                           (gpointer)i);
	}
#endif

	gtk_widget_show_all(Window);
	gtk_main();
}

void RunGui() {
	StartGui();
}

int destroy=0;

void OnLanguage(GtkMenuItem *menuitem, gpointer user_data) {
	ChangeLanguage(langs[(int)user_data].lang);
	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	StartGui();
}


void OnDestroy() {
	if (!destroy) OnFile_Exit();
}

void ConfigurePlugins() {
	if (!UseGui) return;
	OnConf_Conf();
}

void ConfigureMemcards() {
	OnConf_Mcds();
}

void OnRunExe_Ok() {
	const gchar *File;
	char exe[256];

	File = gtk_file_selection_get_filename(GTK_FILE_SELECTION(FileSel));
	strcpy(exe, File);
	gtk_widget_destroy(FileSel);
	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	SysReset();
	needreset = 0;
	Load(exe);
	psxCpu->Execute();
}

void OnRunExe_Cancel() {
	gtk_widget_destroy(FileSel);
}

void OnFile_RunExe() {
	GtkWidget *Ok,*Cancel;

	FileSel = gtk_file_selection_new(_("Select Psx Exe File"));

	Ok = GTK_FILE_SELECTION(FileSel)->ok_button;
	gtk_signal_connect (GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnRunExe_Ok), NULL);
	gtk_widget_show(Ok);

	Cancel = GTK_FILE_SELECTION(FileSel)->cancel_button;
	gtk_signal_connect (GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnRunExe_Cancel), NULL);
	gtk_widget_show(Cancel);

	gtk_widget_show(FileSel);
	gdk_window_raise(FileSel->window);
}

void OnFile_RunCd() {
	LoadCdBios = 0;
	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	SysReset();
	needreset = 0;
	CheckCdrom();
	if (LoadCdrom() == -1) {
		ClosePlugins();
		SysMessage(_("Could not load Cdrom\n"));
		return;
	}
	psxCpu->Execute();
}

void OnFile_RunCdBios() {
	LoadCdBios = 1;
	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	SysReset();
	needreset = 0;
	CheckCdrom();
	psxCpu->Execute();
}

void OnFile_Exit() {
	DIR *dir;
	struct dirent *ent;
	void *Handle;
	char plugin[256];

	// with this the problem with plugins that are linked with the pthread
	// library is solved

	dir = opendir(Config.PluginsDir);
	if (dir != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			sprintf (plugin, "%s%s", Config.PluginsDir, ent->d_name);

			if (strstr(plugin, ".so") == NULL) continue;
			Handle = dlopen(plugin, RTLD_NOW);
			if (Handle == NULL) continue;
		}
	}

	printf(_("PCSX Quitting\n"));
	if (UseGui) gtk_main_quit();
	SysClose();
	if (UseGui) gtk_exit(0);
	else exit(0);
}

void States_Load(int num) {
	char Text[256];
	int ret;

	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	SysReset();
	needreset = 0;

	sprintf(Text, "sstates/%10.10s.%3.3d", CdromLabel, num);
	ret = LoadState(Text);
	if (ret == 0)
		 sprintf(Text, _("*PCSX*: Loaded State %d"), num+1);
	else sprintf(Text, _("*PCSX*: Error Loading State %d"), num+1);
	
	GPU_displayText(Text);

	psxCpu->Execute();
}

void States_Save(int num) {
	char Text[256];
	int ret;

	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	if (needreset) {
		SysReset();
		needreset = 0;
	}
	GPU_updateLace();

	sprintf(Text, "sstates/%10.10s.%3.3d", CdromLabel, num);
	GPU_freeze(2, (GPUFreeze_t *)&num);
	ret = SaveState(Text);
	if (ret == 0)
		 sprintf(Text, _("*PCSX*: Saved State %d"), num+1);
	else sprintf(Text, _("*PCSX*: Error Saving State %d"), num+1);
	
	GPU_displayText(Text);

	psxCpu->Execute();
}

void OnStates_Load1() { States_Load(0); } 
void OnStates_Load2() { States_Load(1); } 
void OnStates_Load3() { States_Load(2); } 
void OnStates_Load4() { States_Load(3); } 
void OnStates_Load5() { States_Load(4); } 

void OnLoadOther_Ok() {
	const gchar *File;
	char str[256];
	char Text[256];
	int ret;

	File = gtk_file_selection_get_filename(GTK_FILE_SELECTION(FileSel));
	strcpy(str, File);
	gtk_widget_destroy(FileSel);
	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	SysReset();
	needreset = 0;

	ret = LoadState(str);
	if (ret == 0)
		 sprintf(Text, _("*PCSX*: Loaded State %s"), str);
	else sprintf(Text, _("*PCSX*: Error Loading State %s"), str);
	GPU_displayText(Text);

	psxCpu->Execute();
}

void OnLoadOther_Cancel() {
	gtk_widget_destroy(FileSel);
}

void OnStates_LoadOther() {
	GtkWidget *Ok,*Cancel;

	FileSel = gtk_file_selection_new(_("Select State File"));
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(FileSel), "sstates/");

	Ok = GTK_FILE_SELECTION(FileSel)->ok_button;
	gtk_signal_connect (GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnLoadOther_Ok), NULL);
	gtk_widget_show(Ok);

	Cancel = GTK_FILE_SELECTION(FileSel)->cancel_button;
	gtk_signal_connect (GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnLoadOther_Cancel), NULL);
	gtk_widget_show(Cancel);

	gtk_widget_show(FileSel);
	gdk_window_raise(FileSel->window);
} 

void OnStates_Save1() { States_Save(0); } 
void OnStates_Save2() { States_Save(1); } 
void OnStates_Save3() { States_Save(2); } 
void OnStates_Save4() { States_Save(3); } 
void OnStates_Save5() { States_Save(4); } 

void OnSaveOther_Ok() {
	const gchar *File;
	char str[256];
	char Text[256];
	int ret;

	File = gtk_file_selection_get_filename(GTK_FILE_SELECTION(FileSel));
	strcpy(str, File);
	gtk_widget_destroy(FileSel);
	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	if (needreset) {
		SysReset();
		needreset = 0;
	}
	GPU_updateLace();

	ret = SaveState(str);
	if (ret == 0)
		 sprintf(Text, _("*PCSX*: Saved State %s"), str);
	else sprintf(Text, _("*PCSX*: Error Saving State %s"), str);
	GPU_displayText(Text);

	psxCpu->Execute();
}

void OnSaveOther_Cancel() {
	gtk_widget_destroy(FileSel);
}

void OnStates_SaveOther() {
	GtkWidget *Ok,*Cancel;

	FileSel = gtk_file_selection_new(_("Select State File"));
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(FileSel), "sstates/");

	Ok = GTK_FILE_SELECTION(FileSel)->ok_button;
	gtk_signal_connect (GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnSaveOther_Ok), NULL);
	gtk_widget_show(Ok);

	Cancel = GTK_FILE_SELECTION(FileSel)->cancel_button;
	gtk_signal_connect (GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnSaveOther_Cancel), NULL);
	gtk_widget_show(Cancel);

	gtk_widget_show(FileSel);
	gdk_window_raise(FileSel->window);
} 

void OnEmu_Run() {
	destroy=1;
	gtk_widget_destroy(Window);
	destroy=0;
	gtk_main_quit();
	while (gtk_events_pending()) gtk_main_iteration();
	if (OpenPlugins() == -1) { SysRunGui(); return; }
	if (needreset) { SysReset(); needreset = 0; }
	psxCpu->Execute();
}

void OnEmu_Reset() {
	needreset = 1;
}

void OnConf_Gpu() {
	gtk_widget_set_sensitive(Window, FALSE);
	while (gtk_events_pending()) gtk_main_iteration();
	GPU_configure();
	gtk_widget_set_sensitive(Window, TRUE);
}

void OnConf_Spu() {
	gtk_widget_set_sensitive(Window, FALSE);
	while (gtk_events_pending()) gtk_main_iteration();
	SPU_configure();
	gtk_widget_set_sensitive(Window, TRUE);
}

void OnConf_Cdr() {
	gtk_widget_set_sensitive(Window, FALSE);
	while (gtk_events_pending()) gtk_main_iteration();
	CDR_configure();
	gtk_widget_set_sensitive(Window, TRUE);
}

void OnConf_Pads() {
	gtk_widget_set_sensitive(Window, FALSE);
	while (gtk_events_pending()) gtk_main_iteration();
	PAD1_configure();
	if (strcmp(Config.Pad1, Config.Pad2)) PAD2_configure();
	gtk_widget_set_sensitive(Window, TRUE);
}

GtkWidget *NetDlg;

void OnNet_Ok() {
	GetComboText(NetConfS.Combo, NetConfS.plist, Config.Net);

	SaveConfig();

	ReleasePlugins();
	LoadPlugins();
	needreset = 1;

	gtk_widget_destroy(NetDlg);
	gtk_widget_set_sensitive(Window, TRUE);
	gtk_main_quit();
}

void OnNet_Cancel() {
	gtk_widget_destroy(NetDlg);
	gtk_widget_set_sensitive(Window, TRUE);
	gtk_main_quit();
}

void OnConf_Net() {
	NetDlg = create_NetDlg();

	FindNetPlugin();

	gtk_widget_show_all(NetDlg);
	gtk_widget_set_sensitive(Window, FALSE);
	gtk_main();
}

GtkWidget *McdDlg;
GtkWidget *Entry1,*Entry2;
GtkWidget *List1,*List2;
GtkWidget *BtnPaste;
GTimer *Gtimer;
GtkTreeSelection *selection1;
GtkTreeSelection *selection2;
int timer;
McdBlock Blocks[2][15];
int IconC[2][15];
enum {
    CL_ICON,
    CL_TITLE,
    CL_STAT,
    CL_ID,
    CL_NAME,
    NUM_CL
};

static void
add_columns (GtkTreeView *treeview)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
//  g_print ("painting table");
  /* column for icon */
  renderer = gtk_cell_renderer_pixbuf_new ();

  column = gtk_tree_view_column_new_with_attributes (_("Icon"),
						     renderer,
						     "pixbuf", CL_ICON,
						     NULL);

   gtk_tree_view_append_column (treeview, column);

  /* column for title */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Title"),
						     renderer,
						     "text",
						     CL_TITLE,
						     NULL);
  gtk_tree_view_append_column (treeview, column);

  /* column for status */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Status"),
						     renderer,
						     "text",
						     CL_STAT,
						     NULL);
  gtk_tree_view_append_column (treeview, column);

  /* column for id */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("ID"),
						     renderer,
						     "text",
						     CL_ID,
						     NULL);
  gtk_tree_view_append_column (treeview, column);

  /* column for Name */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"),
						     renderer,
						     "text",
						     CL_NAME,
						     NULL);
  gtk_tree_view_append_column (treeview, column);
							    
}



GdkPixbuf *SetIcon(short *icon, int i) {
	GdkPixmap *pixmap;
	GdkImage  *image;
	GdkVisual *visual;
	GdkGC     *gc;
	int x, y, c;

	visual = gdk_window_get_visual(McdDlg->window);

	if (visual->depth == 8) return NULL;

	image = gdk_image_new(GDK_IMAGE_NORMAL, visual, 16, 16);

	for (y=0; y<16; y++) {
		for (x=0; x<16; x++) {
			c = icon[y*16+x];
			c = ((c&0x001f) << 10) | ((c&0x7c00) >> 10) | (c&0x03e0);
			if (visual->depth == 16)
				c = (c&0x001f) | ((c&0x7c00) << 1) | ((c&0x03e0) << 1);
			else if (visual->depth == 24 || visual->depth == 32)
				c = ((c&0x001f) << 3) | ((c&0x03e0) << 6) | ((c&0x7c00) << 9);
				
			gdk_image_put_pixel(image, x, y, c);
		}
	}

	pixmap = gdk_pixmap_new(McdDlg->window, 16, 16, visual->depth);

	gc = gdk_gc_new(pixmap);
	gdk_draw_image(pixmap, gc, image, 0, 0, 0, 0, 16, 16);
	gdk_gc_destroy(gc);
	gdk_image_destroy(image);
	
	return gdk_pixbuf_get_from_drawable    (NULL,
	                                        GDK_PIXMAP (pixmap),
		                                NULL,
						0,0,0,0,-1,-1);
//	gtk_clist_set_pixmap(GTK_CLIST(List), i-1, 0, pixmap, NULL);
}

void LoadListItems(int mcd, GtkWidget *List) {
	int i;
/*
	gtk_clist_clear(GTK_CLIST(List));
*/
	GtkListStore *store= gtk_list_store_new (NUM_CL,GDK_TYPE_PIXBUF, G_TYPE_STRING,
					     G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;

	for (i=1; i<16; i++) {
		McdBlock *Info;
		gchar *text[5];

		Info = &Blocks[mcd-1][i-1];
		IconC[mcd-1][i-1] = 0;

		if ((Info->Flags & 0xF0) == 0xA0) {
			if ((Info->Flags & 0xF) >= 1 &&
				(Info->Flags & 0xF) <= 3) {
				text[2] = _("Deleted");
			} else text[2] = _("Free");
		} else if ((Info->Flags & 0xF0) == 0x50)
			text[2] = _("Used");
		else { text[2] = _("Free"); }

		text[0] = "";
		text[1] = Info->Title;
		text[3] = Info->ID;
		text[4] = Info->Name;
/*
		gtk_clist_insert(GTK_CLIST(List), i-1, text);
*/
//		if (Info->IconCount == 0) continue;
		pixbuf = SetIcon(Info->Icon, i);
		
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				CL_ICON,pixbuf,
				CL_TITLE,Info->Title,
				CL_STAT,text[2],
				CL_NAME,Info->Name,
				CL_ID,Info->ID,
				-1);
	}

	gtk_tree_view_set_model (GTK_TREE_VIEW (List), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (List), TRUE);
	gtk_widget_show(List);
}


void UpdateListItems(int mcd, GtkWidget *List) {
	LoadListItems(mcd, List);
}


/*
{
	int i,j;

	for (i=1; i<16; i++) {
		McdBlock *Info;
		gchar *text[5];

		Info = &Blocks[mcd-1][i-1];
		IconC[mcd-1][i-1] = 0;

		if ((Info->Flags & 0xF0) == 0xA0) {
			if ((Info->Flags & 0xF) >= 1 &&
				(Info->Flags & 0xF) <= 3) {
				text[2] = "Deleted";
			} else text[2] = "Free";
		} else if ((Info->Flags & 0xF0) == 0x50)
			text[2] = "Used";
		else { text[2] = "Free"; }

		text[0] = "";
		text[1] = Info->Title;
		text[3] = Info->ID;
		text[4] = Info->Name;

		for (j=0; j<5; j++)
			gtk_clist_set_text(GTK_CLIST(List), i-1, j, text[j]);

		if (Info->IconCount == 0) continue;

		SetIcon(Info->Icon, i);
	}
}
*/
void LoadMcdDlg() {
	int i;

	for (i=1; i<16; i++) GetMcdBlockInfo(1, i, &Blocks[0][i-1]);
	for (i=1; i<16; i++) GetMcdBlockInfo(2, i, &Blocks[1][i-1]);
	LoadListItems(1, List1);
	LoadListItems(2, List2);
}

void UpdateMcdDlg() {
	int i;

	for (i=1; i<16; i++) GetMcdBlockInfo(1, i, &Blocks[0][i-1]);
	for (i=1; i<16; i++) GetMcdBlockInfo(2, i, &Blocks[1][i-1]);
	UpdateListItems(1, List1);
	UpdateListItems(2, List2);
}

void StopTimer() {
	g_timer_stop(Gtimer); timer = 0;
}

void OnMcd_Ok() {
	const char *tmp;

	StopTimer();

	tmp = gtk_entry_get_text(GTK_ENTRY(Entry1));
	strcpy(Config.Mcd1, tmp);
	tmp = gtk_entry_get_text(GTK_ENTRY(Entry2));
	strcpy(Config.Mcd2, tmp);
	
	SaveConfig();
	LoadMcds(Config.Mcd1, Config.Mcd2);

	gtk_widget_destroy(McdDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, TRUE);
//	gtk_main_quit();
}

void OnMcd_Cancel() {
	StopTimer();

	LoadMcds(Config.Mcd1, Config.Mcd2);

	gtk_widget_destroy(McdDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, TRUE);
//	gtk_main_quit();
}

void OnMcdFS1_Ok() {
	const gchar *File;

	File = gtk_file_selection_get_filename(GTK_FILE_SELECTION(FileSel));
	gtk_entry_set_text(GTK_ENTRY(Entry1), File);

	LoadMcd(1, (char*)File);
	UpdateMcdDlg();

	gtk_widget_destroy(FileSel);
}

void OnMcdFS2_Ok() {
	const gchar *File;

	File = gtk_file_selection_get_filename(GTK_FILE_SELECTION(FileSel));
	gtk_entry_set_text(GTK_ENTRY(Entry2), File);

	LoadMcd(2, (char*)File);
	UpdateMcdDlg();

	gtk_widget_destroy(FileSel);
}

void OnMcdFS_Cancel() {
	gtk_widget_destroy(FileSel);
}

void OnMcd_FS1() {
	GtkWidget *Ok,*Cancel;

	FileSel = gtk_file_selection_new(_("Select Psx Mcd File"));
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(FileSel), gtk_entry_get_text(GTK_ENTRY(Entry1)));

	Ok = GTK_FILE_SELECTION(FileSel)->ok_button;
	gtk_signal_connect (GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnMcdFS1_Ok), NULL);
	gtk_widget_show(Ok);

	Cancel = GTK_FILE_SELECTION(FileSel)->cancel_button;
	gtk_signal_connect (GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnMcdFS_Cancel), NULL);
	gtk_widget_show(Cancel);

	gtk_widget_show(FileSel);
	gdk_window_raise(FileSel->window);
}

void OnMcd_FS2() {
	GtkWidget *Ok,*Cancel;

	FileSel = gtk_file_selection_new(_("Select Psx Mcd File"));
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(FileSel), gtk_entry_get_text(GTK_ENTRY(Entry2)));

	Ok = GTK_FILE_SELECTION(FileSel)->ok_button;
	gtk_signal_connect (GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnMcdFS2_Ok), NULL);
	gtk_widget_show(Ok);

	Cancel = GTK_FILE_SELECTION(FileSel)->cancel_button;
	gtk_signal_connect (GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnMcdFS_Cancel), NULL);
	gtk_widget_show(Cancel);

	gtk_widget_show(FileSel);
	gdk_window_raise(FileSel->window);
}

GtkWidget *MsgBoxDlg;

/*
void OnMsgBox_Yes() {
	yes = 1;
	gtk_widget_destroy(MsgBoxDlg);
	gtk_main_quit();
}

void OnMsgBox_No() {
	yes = 0;
	gtk_widget_destroy(MsgBoxDlg);
	gtk_main_quit();
}
*/
int MessageBox(char *msg, char *title) {
	GtkWidget *Txt;

	int w;

	if (msg[strlen(msg)-1] == '\n') msg[strlen(msg)-1] = 0;

	w = strlen(msg) * 6 + 20;

	MsgBoxDlg = gtk_dialog_new_with_buttons (title,
										 GTK_WINDOW(McdDlg),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_ACCEPT,
										 GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_REJECT,
                                         NULL);
	gtk_widget_set_usize(MsgBoxDlg, w, 70);
	gtk_window_set_position(GTK_WINDOW(MsgBoxDlg), GTK_WIN_POS_CENTER);
	
	Txt = gtk_label_new(msg);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(MsgBoxDlg)->vbox),
                      Txt);
	
	gtk_widget_show(Txt);

	gtk_widget_show(MsgBoxDlg);	

/*	g_signal_connect_swapped (GTK_OBJECT (MsgBoxDlg), 
                             "response", 
                             G_CALLBACK (gtk_widget_destroy),
                             GTK_OBJECT (MsgBoxDlg));*/
	w = (gtk_dialog_run (GTK_DIALOG(MsgBoxDlg))==GTK_RESPONSE_ACCEPT)? 1:0;
	gtk_widget_destroy(MsgBoxDlg);
	return w;
}

void OnMcd_Format1() {
	const char *str;

	if (MessageBox(_("Are you sure you want to format this Memory Card?"), _("Confirmation")) == 0) return;
	str = gtk_entry_get_text(GTK_ENTRY(Entry1));
	CreateMcd((char*)str);
	LoadMcd(1, (char*)str);
	UpdateMcdDlg();
}

void OnMcd_Format2() {
	const char *str;

	if (MessageBox(_("Are you sure you want to format this Memory Card?"), _("Confirmation")) == 0) return;
	str = gtk_entry_get_text(GTK_ENTRY(Entry2));
	CreateMcd((char*)str);
	LoadMcd(2, (char*)str);
	UpdateMcdDlg();
}

void OnMcd_Reload1() {
	const char *str;

	str = gtk_entry_get_text(GTK_ENTRY(Entry1));
	LoadMcd(1, (char*)str);
	UpdateMcdDlg();
}

void OnMcd_Reload2() {
	const char *str;

	str = gtk_entry_get_text(GTK_ENTRY(Entry2));
	LoadMcd(2, (char*)str);
	UpdateMcdDlg();
}

static int copy = 0, copymcd = 0;

void OnMcd_CopyTo1() {
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	gint *i;

	if (gtk_tree_selection_get_selected (selection1, &model, &iter)) {
		path = gtk_tree_model_get_path (model, &iter);
		i = gtk_tree_path_get_indices (path);
//		g_print("index1 %d\n",*i);
		copy    = *i;
		copymcd = 1;
		gtk_tree_path_free (path);
		gtk_widget_set_sensitive(BtnPaste, TRUE);
	}
}

void OnMcd_CopyTo2() {
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	gint *i;

	if (gtk_tree_selection_get_selected (selection1, &model, &iter)) {
		path = gtk_tree_model_get_path (model, &iter);
		i = gtk_tree_path_get_indices (path);
//		g_print("index2 %d\n",*i);
		copy    = *i;
		copymcd = 2;
		gtk_tree_path_free (path);
		gtk_widget_set_sensitive(BtnPaste, TRUE);
	}
}

void OnMcd_Paste() {
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	gint *i;
	const char *str;

	if (MessageBox(_("Are you sure you want to paste this selection?"), _("Confirmation")) == 0) return;

	if (copymcd == 1) {
		str = gtk_entry_get_text(GTK_ENTRY(Entry1));
		if (gtk_tree_selection_get_selected (selection1, &model, &iter)) {
			path = gtk_tree_model_get_path (model, &iter);
			i = gtk_tree_path_get_indices (path);

			//i = GTK_CLIST(List1)->focus_row;

			// save dir data + save data
			memcpy(Mcd1Data + (*i+1) * 128, Mcd2Data + (copy+1) * 128, 128);
			SaveMcd((char*)str, Mcd1Data, (*i+1) * 128, 128);
			memcpy(Mcd1Data + (*i+1) * 1024 * 8, Mcd2Data + (copy+1) * 1024 * 8, 1024 * 8);
			SaveMcd((char*)str, Mcd1Data, (*i+1) * 1024 * 8, 1024 * 8);
		} else return;
	} else { // 2
		str = gtk_entry_get_text(GTK_ENTRY(Entry2));
		if (gtk_tree_selection_get_selected (selection2, &model, &iter)) {
			path = gtk_tree_model_get_path (model, &iter);
			i = gtk_tree_path_get_indices (path);

			//i = GTK_CLIST(List2)->focus_row;

			// save dir data + save data
			memcpy(Mcd2Data + (*i+1) * 128, Mcd1Data + (copy+1) * 128, 128);
			SaveMcd((char*)str, Mcd2Data, (*i+1) * 128, 128);
			memcpy(Mcd2Data + (*i+1) * 1024 * 8, Mcd1Data + (copy+1) * 1024 * 8, 1024 * 8);
			SaveMcd((char*)str, Mcd2Data, (*i+1) * 1024 * 8, 1024 * 8);
		} else return;
	}
	UpdateMcdDlg();
}

void OnMcd_Delete1() {
	McdBlock *Info;
	int mcd = 1;
	int i, xor = 0, j;
	unsigned char *data, *ptr;
	const char *str;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;

	if (gtk_tree_selection_get_selected (selection1, &model, &iter)) {
		path = gtk_tree_model_get_path (model, &iter);
		i = *gtk_tree_path_get_indices (path);

		str = gtk_entry_get_text(GTK_ENTRY(Entry1));
		data = Mcd1Data;

		i++;

		ptr = data + i * 128;

		Info = &Blocks[mcd-1][i-1];

		if ((Info->Flags & 0xF0) == 0xA0) {
			if ((Info->Flags & 0xF) >= 1 &&
				(Info->Flags & 0xF) <= 3) { // deleted
				*ptr = 0x50 | (Info->Flags & 0xF);
			} else return;
		} else if ((Info->Flags & 0xF0) == 0x50) { // used
				*ptr = 0xA0 | (Info->Flags & 0xF);
		} else { return; }

		for (j=0; j<127; j++) xor^=*ptr++;
		*ptr = xor;

		SaveMcd((char*)str, data, i * 128, 128);
		UpdateMcdDlg();
	}
}

void OnMcd_Delete2() {
	McdBlock *Info;
	int mcd = 2;
	int i, xor = 0, j;
	unsigned char *data, *ptr;
	const char *str;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;

	if (gtk_tree_selection_get_selected (selection2, &model, &iter)) {
		path = gtk_tree_model_get_path (model, &iter);
		i = *gtk_tree_path_get_indices (path);

		str = gtk_entry_get_text(GTK_ENTRY(Entry2));
//		i = GTK_CLIST(List2)->focus_row;
		data = Mcd2Data;

		i++;

		ptr = data + i * 128;

		Info = &Blocks[mcd-1][i-1];

		if ((Info->Flags & 0xF0) == 0xA0) {
			if ((Info->Flags & 0xF) >= 1 &&
				(Info->Flags & 0xF) <= 3) { // deleted
				*ptr = 0x50 | (Info->Flags & 0xF);
			} else return;
		} else if ((Info->Flags & 0xF0) == 0x50) { // used
				*ptr = 0xA0 | (Info->Flags & 0xF);
		} else { return; }

		for (j=0; j<127; j++) xor^=*ptr++;
		*ptr = xor;

		SaveMcd((char*)str, data, i * 128, 128);
		UpdateMcdDlg();
	}
}

void UpdateMcdIcon(int mcd, GtkWidget *List) {
#if 0
	int i;
	GtkListStore *store= gtk_list_store_new (NUM_CL,GDK_TYPE_PIXBUF, G_TYPE_STRING,
					     G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;

	gtk_tree_clear_items(GTK_TREE_VIEW (List));

	for (i=1; i<16; i++) {
		McdBlock *Info;
		int *count;

		Info = &Blocks[mcd-1][i-1];
		count = &IconC[mcd-1][i-1];

		if (Info->IconCount <= 1) continue;

		(*count)++;
		if (*count == Info->IconCount) *count = 0;

	{
		gchar *text[5];

		Info = &Blocks[mcd-1][i-1];
		IconC[mcd-1][i-1] = 0;

		if ((Info->Flags & 0xF0) == 0xA0) {
			if ((Info->Flags & 0xF) >= 1 &&
				(Info->Flags & 0xF) <= 3) {
				text[2] = _("Deleted");
			} else text[2] = _("Free");
		} else if ((Info->Flags & 0xF0) == 0x50)
			text[2] = _("Used");
		else { text[2] = _("Free"); }

		text[0] = "";
		text[1] = Info->Title;
		text[3] = Info->ID;
		text[4] = Info->Name;

//		pixbuf=SetIcon(Info->Icon, i);
		pixbuf=SetIcon(&Info->Icon[*count*16*16], i);
		
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				CL_ICON,pixbuf,
				CL_TITLE,Info->Title,
				CL_STAT,text[2],
				CL_NAME,Info->Name,
				CL_ID,Info->ID,
				-1);
	}

//		SetIcon(&Info->Icon[*count*16*16], i);
	}
/*	gtk_tree_view_set_model (GTK_TREE_VIEW (List), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (List), TRUE);*/
//	gtk_widget_show(List);

#endif
}

void OnConf_Mcds() {
	McdDlg = create_McdsDlg();
	gtk_window_set_title(GTK_WINDOW(McdDlg), _("PCSX Memcard Manager"));
	
	Entry1 = lookup_widget(McdDlg, "GtkEntry_Mcd1");
	if (!strlen(Config.Mcd1)) strcpy(Config.Mcd1, "memcards/Mcd001.mcr");
	gtk_entry_set_text(GTK_ENTRY(Entry1), Config.Mcd1);

	Entry2 = lookup_widget(McdDlg, "GtkEntry_Mcd2");
	if (!strlen(Config.Mcd2)) strcpy(Config.Mcd2, "memcards/Mcd002.mcr");
	gtk_entry_set_text(GTK_ENTRY(Entry2), Config.Mcd2);

	List1 = lookup_widget(McdDlg, "GtkCList_McdList1");
	List2 = lookup_widget(McdDlg, "GtkCList_McdList2");

	BtnPaste = lookup_widget(McdDlg, "GtkButton_McdPaste");
	gtk_widget_set_sensitive(BtnPaste, FALSE);
	add_columns(GTK_TREE_VIEW (List1));
	add_columns(GTK_TREE_VIEW (List2));
/*
	gtk_clist_set_column_justification(GTK_CLIST(List1), 0, GTK_JUSTIFY_CENTER);
	gtk_clist_set_column_justification(GTK_CLIST(List2), 0, GTK_JUSTIFY_CENTER);
	gtk_clist_set_column_justification(GTK_CLIST(List1), 2, GTK_JUSTIFY_CENTER);
	gtk_clist_set_column_justification(GTK_CLIST(List2), 2, GTK_JUSTIFY_CENTER);
*/
	/* memory vs cpu */
	selection1 = gtk_tree_view_get_selection (GTK_TREE_VIEW (List1));
	gtk_tree_selection_set_mode (selection1, GTK_SELECTION_SINGLE);
	
	selection2 = gtk_tree_view_get_selection (GTK_TREE_VIEW (List2));
	gtk_tree_selection_set_mode (selection2, GTK_SELECTION_SINGLE);
	
	
	gtk_widget_show(List1);
	gtk_widget_show(List2);
	gtk_widget_show_all(McdDlg);

	LoadMcdDlg();

	if (Window != NULL) gtk_widget_set_sensitive(Window, FALSE);

	Gtimer = g_timer_new(); timer = 1;

	while (gtk_events_pending()) gtk_main_iteration();

	while (timer) {
		unsigned long usecs;

		g_timer_elapsed(Gtimer, &usecs);
		if (usecs > 250000) {
			UpdateMcdIcon(1, List1);
			UpdateMcdIcon(2, List2);
			g_timer_reset(Gtimer);
		}

		while (gtk_events_pending()) gtk_main_iteration();
		usleep(10000);
	}
}

GtkWidget *CpuDlg;
GtkWidget *PsxCombo;
GList *psxglist;
char *psxtypes[] = {
	"NTSC",
	"PAL"
};

void OnCpu_Ok() {
	GtkWidget *Btn;
	const char *tmp;
	long t;

	tmp = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(PsxCombo)->entry));
	
	if (!strcmp("NTSC",tmp)) Config.PsxType = 0;
	else Config.PsxType = 1; // pal

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Xa");
	Config.Xa = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Sio");
	Config.Sio = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Mdec");
	Config.Mdec = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Cdda");
	Config.Cdda = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_PsxAuto");
	Config.PsxAuto = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	t = Config.Cpu;
	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Cpu");
	Config.Cpu = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));
	if (t != Config.Cpu) {
		psxCpu->Shutdown();
#ifdef PSXREC
		if (Config.Cpu)	
			 psxCpu = &psxInt;
		else psxCpu = &psxRec;
#else
		psxCpu = &psxInt;
#endif
		if (psxCpu->Init() == -1) {
			SysClose();
			exit(1);
		}
		psxCpu->Reset();
	}

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_PsxOut");
	Config.PsxOut = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_SpuIrq");
	Config.SpuIrq = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_RCntFix");
	Config.RCntFix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_VSyncWA");
	Config.VSyncWA = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Btn));

	gtk_widget_destroy(CpuDlg);

	SaveConfig();
	if (Window != NULL) gtk_widget_set_sensitive(Window, TRUE);
	gtk_main_quit();
}

void OnCpu_Cancel() {
	gtk_widget_destroy(CpuDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, TRUE);
	gtk_main_quit();
}


void OnConf_Cpu() {
	GtkWidget *Btn;
	int i;

	CpuDlg = create_CpuDlg();
	gtk_window_set_title(GTK_WINDOW(CpuDlg), _("PCSX Configuration"));

	psxglist = NULL;
	for (i=0;i<2;i++)
		psxglist = g_list_append(psxglist, psxtypes[i]);
	PsxCombo = lookup_widget(CpuDlg, "GtkCombo_PsxType");
	gtk_combo_set_popdown_strings(GTK_COMBO(PsxCombo), psxglist);
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(PsxCombo)->entry), psxtypes[Config.PsxType]);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Xa");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.Xa);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Sio");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.Sio);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Mdec");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.Mdec);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Cdda");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.Cdda);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_PsxAuto");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.PsxAuto);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_Cpu");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.Cpu);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_PsxOut");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.PsxOut);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_SpuIrq");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.SpuIrq);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_RCntFix");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.RCntFix);

	Btn = lookup_widget(CpuDlg, "GtkCheckButton_VSyncWA");
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(Btn), Config.VSyncWA);

	gtk_widget_show_all(CpuDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, FALSE);
	gtk_main();
}

void OnConfConf_Ok() {
	GetComboText(GpuConfS.Combo, GpuConfS.plist, Config.Gpu);
	GetComboText(SpuConfS.Combo, SpuConfS.plist, Config.Spu);
	GetComboText(CdrConfS.Combo, CdrConfS.plist, Config.Cdr);
	GetComboText(Pad1ConfS.Combo, Pad1ConfS.plist, Config.Pad1);
	GetComboText(Pad2ConfS.Combo, Pad2ConfS.plist, Config.Pad2);
	GetComboText(BiosConfS.Combo, BiosConfS.plist, Config.Bios);

	SaveConfig();

	ReleasePlugins();
	LoadPlugins();

	needreset = 1;
	gtk_widget_destroy(ConfDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, TRUE);
	gtk_main_quit();
}

void OnConfConf_Cancel() {
	gtk_widget_destroy(ConfDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, TRUE);
	gtk_main_quit();
}

#define ConfPlugin(src, confs, plugin, name, parent) \
	void *drv; \
	src conf; \
	char file[256]; \
 \
	GetComboText(confs.Combo, confs.plist, plugin); \
	strcpy(file, Config.PluginsDir); \
	strcat(file, plugin); \
	drv = SysLoadLibrary(file); \
	if (drv == NULL) return; \
 \
	gtk_widget_set_sensitive(parent, FALSE); \
	while (gtk_events_pending()) gtk_main_iteration(); \
	conf = (src) SysLoadSym(drv, name); \
	if (conf) { \
		conf(); \
	} else SysMessage(_("Plugin doesn't needs to be configured")); \
	SysCloseLibrary(drv); \
	gtk_widget_set_sensitive(parent, TRUE);

#define TestPlugin(src, confs, plugin, name, parent) \
	void *drv; \
	src conf; \
	int ret = 0; \
	char file[256]; \
 \
	GetComboText(confs.Combo, confs.plist, plugin); \
	strcpy(file, Config.PluginsDir); \
	strcat(file, plugin); \
	drv = SysLoadLibrary(file); \
	if (drv == NULL) return; \
 \
	gtk_widget_set_sensitive(parent, FALSE); \
	while (gtk_events_pending()) gtk_main_iteration(); \
	conf = (src) SysLoadSym(drv, name); \
	if (SysLibError() == NULL) ret = conf(); \
	SysCloseLibrary(drv); \
	if (ret == 0) \
		 SysMessage(_("This plugin reports that should work correctly")); \
	else SysMessage(_("This plugin reports that should not work correctly")); \
	gtk_widget_set_sensitive(parent, TRUE);

void OnConfConf_GpuConf() {
	ConfPlugin(GPUconfigure, GpuConfS, Config.Gpu, "GPUconfigure", ConfDlg);
}

void OnConfConf_GpuTest() {
	TestPlugin(GPUtest, GpuConfS, Config.Gpu, "GPUtest", ConfDlg);
}

void OnConfConf_GpuAbout() {
	ConfPlugin(GPUabout, GpuConfS, Config.Gpu, "GPUabout", ConfDlg);
}

void OnConfConf_SpuConf() {
	ConfPlugin(SPUconfigure, SpuConfS, Config.Spu, "SPUconfigure", ConfDlg);
}

void OnConfConf_SpuTest() {
	TestPlugin(SPUtest, SpuConfS, Config.Spu, "SPUtest", ConfDlg);
}

void OnConfConf_SpuAbout() {
	ConfPlugin(SPUabout, SpuConfS, Config.Spu, "SPUabout", ConfDlg);
}

void OnConfConf_CdrConf() {
	ConfPlugin(CDRconfigure, CdrConfS, Config.Cdr, "CDRconfigure", ConfDlg);
}

void OnConfConf_CdrTest() {
	TestPlugin(CDRtest, CdrConfS, Config.Cdr, "CDRtest", ConfDlg);
}

void OnConfConf_CdrAbout() {
	ConfPlugin(CDRabout, CdrConfS, Config.Cdr, "CDRabout", ConfDlg);
}

void OnConfConf_Pad1Conf() {
	ConfPlugin(PADconfigure, Pad1ConfS, Config.Pad1, "PADconfigure", ConfDlg);
}

void OnConfConf_Pad1Test() {
	TestPlugin(PADtest, Pad1ConfS, Config.Pad1, "PADtest", ConfDlg);
}

void OnConfConf_Pad1About() {
	ConfPlugin(PADabout, Pad1ConfS, Config.Pad1, "PADabout", ConfDlg);
}

void OnConfConf_Pad2Conf() {
	ConfPlugin(PADconfigure, Pad2ConfS, Config.Pad2, "PADconfigure", ConfDlg);
}

void OnConfConf_Pad2Test() {
	TestPlugin(PADtest, Pad2ConfS, Config.Pad2, "PADtest", ConfDlg);
}

void OnConfConf_Pad2About() {
	ConfPlugin(PADabout, Pad2ConfS, Config.Pad2, "PADabout", ConfDlg);
}

void OnNet_Conf() {
	ConfPlugin(NETconfigure, NetConfS, Config.Net, "NETconfigure", NetDlg);
}

void OnNet_Test() {
	TestPlugin(NETtest, NetConfS, Config.Net, "NETtest", NetDlg);
}

void OnNet_About() {
	ConfPlugin(NETabout, NetConfS, Config.Net, "NETabout", NetDlg);
}



void OnPluginsPath_Ok() {
	const gchar *File;

	File = gtk_file_selection_get_filename(GTK_FILE_SELECTION(FileSel));
	strcpy(Config.PluginsDir, File);
	if (Config.PluginsDir[strlen(Config.PluginsDir)-1] != '/')
		strcat(Config.PluginsDir, "/");

	FindPlugins();

	gtk_widget_destroy(FileSel);
}

void OnPluginsPath_Cancel() {
	gtk_widget_destroy(FileSel);
}

void OnConfConf_PluginsPath() {
	GtkWidget *Ok,*Cancel;

	FileSel = gtk_file_selection_new(_("Select Plugins Directory"));

	Ok = GTK_FILE_SELECTION(FileSel)->ok_button;
	gtk_signal_connect (GTK_OBJECT(Ok), "clicked", GTK_SIGNAL_FUNC(OnPluginsPath_Ok), NULL);
	gtk_widget_show(Ok);

	Cancel = GTK_FILE_SELECTION(FileSel)->cancel_button;
	gtk_signal_connect (GTK_OBJECT(Cancel), "clicked", GTK_SIGNAL_FUNC(OnPluginsPath_Cancel), NULL);
	gtk_widget_show(Cancel);

	gtk_widget_show(FileSel);
	gdk_window_raise(FileSel->window);
}

void OnBiosPath_Ok(GtkWidget *wdg, gpointer data) {
	const gchar *File;

	File = gtk_file_selection_get_filename(GTK_FILE_SELECTION(data));
	strcpy(Config.BiosDir, File);
	if (Config.BiosDir[strlen(Config.BiosDir)-1] != '/')
		strcat(Config.BiosDir, "/");

	FindPlugins();

	gtk_widget_destroy(GTK_WIDGET(data));
}

void OnBiosPath_Cancel(GtkWidget *wdg,gpointer data) {
	gtk_widget_destroy(GTK_WIDGET(data));
}

void OnConfConf_BiosPath() {
	GtkWidget *Ok,*Cancel;
	GtkWidget *Sel = gtk_file_selection_new(_("Select Bios Directory"));

	Ok = GTK_FILE_SELECTION(Sel)->ok_button;
	g_signal_connect (GTK_OBJECT (Ok), "clicked", G_CALLBACK (OnBiosPath_Ok), GTK_OBJECT (Sel));
	gtk_widget_show(Ok);

	Cancel = GTK_FILE_SELECTION(Sel)->cancel_button;
	g_signal_connect (GTK_OBJECT (Cancel), "clicked", G_CALLBACK (OnBiosPath_Cancel), GTK_OBJECT (Sel));
	gtk_widget_show(Cancel);

	gtk_widget_show(Sel);
	gdk_window_raise(Sel->window);
}

void OnConf_Conf() {
	ConfDlg = create_ConfDlg();
	gtk_window_set_title(GTK_WINDOW(ConfDlg), _("PCSX Configuration"));

	FindPlugins();

	gtk_widget_show_all(ConfDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, FALSE);
	gtk_main();
}

void OnHelp_Help() {
}

void OnHelpAbout_Ok() {
	gtk_widget_destroy(AboutDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, TRUE);
	gtk_main_quit();
}

void OnHelp_About() {
	GtkWidget *Label;
	char text[256];

	AboutDlg = create_AboutDlg();
	gtk_window_set_title(GTK_WINDOW(AboutDlg), _("About PCSX"));

	Label = lookup_widget(AboutDlg, "GtkAbout_LabelVersion");
	sprintf(text, _("PCSX For Linux\nVersion %s"), PCSX_VERSION);
	gtk_label_set_text(GTK_LABEL(Label), text);

	Label = lookup_widget(AboutDlg, "GtkAbout_LabelAuthors");
	gtk_label_set_text(GTK_LABEL(Label), _(LabelAuthors));

	Label = lookup_widget(AboutDlg, "GtkAbout_LabelGreets");
	gtk_label_set_text(GTK_LABEL(Label), _(LabelGreets));

	gtk_widget_show_all(AboutDlg);
	if (Window != NULL) gtk_widget_set_sensitive(Window, FALSE);
	gtk_main();
}

#define ComboAddPlugin(type) { \
	type##ConfS.plugins+=2; \
	strcpy(type##ConfS.plist[type##ConfS.plugins-1], name); \
	strcpy(type##ConfS.plist[type##ConfS.plugins-2], ent->d_name); \
	type##ConfS.glist = g_list_append(type##ConfS.glist, type##ConfS.plist[type##ConfS.plugins-1]); \
}

#define ConfCreatePConf(name, type) \
	if (type##ConfS.glist != NULL) { \
		type##ConfS.Combo = lookup_widget(ConfDlg, "GtkCombo_" name); \
    	gtk_combo_set_popdown_strings(GTK_COMBO(type##ConfS.Combo), type##ConfS.glist); \
		FindComboText(type##ConfS.Combo, type##ConfS.plist, Config.type); \
	}

void FindPlugins() {
	DIR *dir;
	struct dirent *ent;
	void *Handle;
	char plugin[256],name[256];

	GpuConfS.plugins  = 0; SpuConfS.plugins  = 0; CdrConfS.plugins  = 0;
	Pad1ConfS.plugins = 0; Pad2ConfS.plugins = 0; BiosConfS.plugins = 0;
	GpuConfS.glist  = NULL; SpuConfS.glist  = NULL; CdrConfS.glist  = NULL;
	Pad1ConfS.glist = NULL; Pad2ConfS.glist = NULL; BiosConfS.glist = NULL;

	dir = opendir(Config.PluginsDir);
	if (dir == NULL) {
		SysMessage(_("Could not open '%s' directory\n"), Config.PluginsDir);
		return;
	}
	while ((ent = readdir(dir)) != NULL) {
		long type,v;

		sprintf(plugin, "%s%s", Config.PluginsDir, ent->d_name);

		if (strstr(plugin, ".so") == NULL) continue;
		Handle = dlopen(plugin, RTLD_NOW);
		if (Handle == NULL) { printf("%s\n", dlerror()); continue; }

		PSE_getLibType = (PSEgetLibType) dlsym(Handle, "PSEgetLibType");
		if (dlerror() != NULL) {
			if (strstr(plugin, "gpu") != NULL) type = PSE_LT_GPU;
			else if (strstr(plugin, "cdr") != NULL) type = PSE_LT_CDR;
			else if (strstr(plugin, "spu") != NULL) type = PSE_LT_SPU;
			else if (strstr(plugin, "pad") != NULL) type = PSE_LT_PAD;
			else continue;
		}
		else type = PSE_getLibType();

		PSE_getLibName = (PSEgetLibName) dlsym(Handle, "PSEgetLibName");
		if (dlerror() == NULL) {
			sprintf(name, "%s", PSE_getLibName());
			PSE_getLibVersion = (PSEgetLibVersion) dlsym(Handle, "PSEgetLibVersion");
			if (dlerror() == NULL) {
				char ver[32];

				v = PSE_getLibVersion();
				sprintf(ver, " %ld.%ld.%ld",v>>16,(v>>8)&0xff,v&0xff);
				strcat(name, ver);
			}
		}
		else strcpy(name, ent->d_name);

		if (type & PSE_LT_CDR) {
			ComboAddPlugin(Cdr);
		}
		if (type & PSE_LT_GPU) {
			ComboAddPlugin(Gpu);
		}
		if (type & PSE_LT_SPU) {
			ComboAddPlugin(Spu);
		}
		if (type & PSE_LT_PAD) {
			PADquery query = (PADquery)dlsym(Handle, "PADquery");
			if (query() & 0x1) {
				ComboAddPlugin(Pad1);
			}
			if (query() & 0x2) {
				ComboAddPlugin(Pad2);
			}
		}
	}
	closedir(dir);

	ConfCreatePConf("Gpu", Gpu);
	ConfCreatePConf("Spu", Spu);
	ConfCreatePConf("Pad1", Pad1);
	ConfCreatePConf("Pad2", Pad2);
	ConfCreatePConf("Cdr", Cdr);

	BiosConfS.plugins+=2;
	strcpy(BiosConfS.plist[BiosConfS.plugins-1], _("Internal HLE Bios"));
	strcpy(BiosConfS.plist[BiosConfS.plugins-2], "HLE");
	BiosConfS.glist = g_list_append(BiosConfS.glist, BiosConfS.plist[BiosConfS.plugins-1]);

	dir = opendir(Config.BiosDir);
	if (dir == NULL) {
		SysMessage(_("Could not open '%s' directory\n"), Config.BiosDir);
		return;
	}

	while ((ent = readdir(dir)) != NULL) {
		struct stat buf;

		sprintf (plugin, "%s%s", Config.BiosDir, ent->d_name);
		if (stat(plugin, &buf) == -1) continue;
		if (buf.st_size != (1024*512)) continue;

		BiosConfS.plugins+=2;
		strcpy(BiosConfS.plist[BiosConfS.plugins-1], ent->d_name);
		strcpy(BiosConfS.plist[BiosConfS.plugins-2], ent->d_name);
		BiosConfS.glist = g_list_append(BiosConfS.glist, BiosConfS.plist[BiosConfS.plugins-1]);
	}
	closedir(dir);

	ConfCreatePConf("Bios", Bios);
}

#define ConfCreatePNet(name, type) \
	if (type##ConfS.glist != NULL) { \
		type##ConfS.Combo = lookup_widget(NetDlg, "GtkCombo_" name); \
    	gtk_combo_set_popdown_strings(GTK_COMBO(type##ConfS.Combo), type##ConfS.glist); \
		FindComboText(type##ConfS.Combo, type##ConfS.plist, Config.type); \
	}

void FindNetPlugin() {
	DIR *dir;
	struct dirent *ent;
	void *Handle;
	char plugin[256],name[256];

	NetConfS.plugins  = 0;
	NetConfS.glist = NULL; 

	NetConfS.plugins+=2;
	strcpy(NetConfS.plist[NetConfS.plugins-1], "Disabled");
	strcpy(NetConfS.plist[NetConfS.plugins-2], "Disabled");
	NetConfS.glist = g_list_append(NetConfS.glist, NetConfS.plist[NetConfS.plugins-1]);

	dir = opendir(Config.PluginsDir);
	if (dir == NULL) {
		SysMessage(_("Could not open '%s' directory\n"), Config.PluginsDir);
		return;
	}
	while ((ent = readdir(dir)) != NULL) {
		long type,v;

		sprintf(plugin, "%s%s", Config.PluginsDir, ent->d_name);

		if (strstr(plugin, ".so") == NULL) continue;
		Handle = dlopen(plugin, RTLD_NOW);
		if (Handle == NULL) continue;

		PSE_getLibType = (PSEgetLibType) dlsym(Handle, "PSEgetLibType");
		if (dlerror() != NULL) {
			if (strstr(plugin, "net") != NULL) type = PSE_LT_NET;
			else continue;
		}
		else type = PSE_getLibType();

		PSE_getLibName = (PSEgetLibName) dlsym(Handle, "PSEgetLibName");
		if (dlerror() == NULL) {
			sprintf(name, "%s", PSE_getLibName());
			PSE_getLibVersion = (PSEgetLibVersion) dlsym(Handle, "PSEgetLibVersion");
			if (dlerror() == NULL) {
				char ver[32];

				v = PSE_getLibVersion();
				sprintf(ver, " %ld.%ld.%ld",v>>16,(v>>8)&0xff,v&0xff);
				strcat(name, ver);
			}
		}
		else strcpy(name, ent->d_name);

		if (type & PSE_LT_NET) {
			ComboAddPlugin(Net);
		}
	}
	closedir(dir);

	ConfCreatePNet("Net", Net);
}

GtkWidget *MsgDlg;


void SysMessage(char *fmt, ...) {
	GtkWidget *Txt;
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	if (msg[strlen(msg)-1] == '\n') msg[strlen(msg)-1] = 0;

	if (!UseGui) { printf ("%s\n",msg); return; }
/* there is an handy function for that, maybe we can use it instead */
	MsgDlg =  gtk_dialog_new_with_buttons (_("Pcsx Msg"),
                                         NULL/*GTK_WINDOW(Window)*/,
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_NONE,
                                         NULL);
	
	gtk_window_set_position (GTK_WINDOW(MsgDlg), GTK_WIN_POS_CENTER);
/*	g_signal_connect_swapped (GTK_OBJECT (MsgDlg), 
                             "response", 
                             G_CALLBACK (gtk_widget_destroy),
                             GTK_OBJECT (MsgDlg));*/
	
	Txt = gtk_label_new (msg);
	gtk_label_set_line_wrap(GTK_LABEL(Txt), TRUE); 
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(MsgDlg)->vbox),
                      Txt);	
	
	gtk_widget_show (Txt);
	gtk_widget_show_all (MsgDlg);
	gtk_dialog_run(GTK_DIALOG(MsgDlg));
	gtk_widget_destroy(MsgDlg);
}

