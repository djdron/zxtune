/*
 * info_dlg.h - file information dialog box
 *
 * Copyright (C) 2007-2025  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <windows.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IDD_INFO       300
#define IDC_PLAYING    301
#define IDC_FILENAME   302
#define IDC_AUTHOR     303
#define IDC_NAME       304
#define IDC_DATE       305
#define IDC_PICKDATE   306
#define IDC_SONGNO     307
#define IDC_TIME       308
#define IDC_LOOP       309
#define IDC_NTSC       310
#define IDC_TECHINFO   311
#define IDC_STILFILE   312
#define IDC_STILINFO   313
#define IDC_SAVE       314
#define IDC_SAVEAS     315

#define IDD_PROGRESS   500
#define IDC_PROGRESS   501

bool loadInfo(ASAPInfo *info, const char *filename);
bool loadFiles(ASAP *asap, const char *filename);

extern HWND infoDialog;
void showInfoDialog(HINSTANCE hInstance, HWND hwndParent, const char *filename, int song);
void updateInfoDialog(const char *filename, int song);
void setPlayingSong(const char *filename, int song);
#ifdef FOOBAR2000
void setDarkInfoDialog(HWND hDlg);
void releaseDarkInfoDialog(void);
#endif
#ifdef XMPLAY
const ASTIL *getPlayingASTIL(void);
#endif
#if defined(WINAMP) || defined(FOOBAR2000) || defined(XMPLAY)
#define PLAYING_INFO
extern bool playing_info;
void onUpdatePlayingInfo(void);
#endif

#ifdef __cplusplus
}
#endif
