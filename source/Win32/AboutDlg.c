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
#include "resource.h"
#include "AboutDlg.h"
#include "PsxCommon.h"

LRESULT WINAPI AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			SetWindowText(hDlg, _("About"));

			Button_SetText(GetDlgItem(hDlg, IDOK), _("OK"));
			Static_SetText(GetDlgItem(hDlg, IDC_PCSX_ABOUT_TEXT), _("PCSX EMU\n"));
			Static_SetText(GetDlgItem(hDlg, IDC_PCSX_ABOUT_AUTHORS), _(LabelAuthors));
			Static_SetText(GetDlgItem(hDlg, IDC_PCSX_ABOUT_GREETS), _(LabelGreets));
			Button_SetText(GetDlgItem(hDlg,IDOK), _("OK"));
			return TRUE;

		case WM_COMMAND:
			switch(wParam) {
				case IDOK:
					EndDialog(hDlg, TRUE);
					return TRUE;
			}
			break;
	}
	return FALSE;
}
