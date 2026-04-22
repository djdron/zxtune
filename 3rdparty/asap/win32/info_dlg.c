/*
 * info_dlg.c - file information dialog box
 *
 * Copyright (C) 2007-2026  Piotr Fusik
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

#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commctrl.h>

#include "asap.h"
#include "astil.h"
#include "info_dlg.h"
#ifdef WINAMP
#include "aatr-stdio.h"
#endif

typedef struct {
	int (*load)(const ASAPFileLoader *self, const char *filename, uint8_t *buffer, int length);
} ASAPFileLoaderVtbl;

struct ASAPFileLoader {
	const ASAPFileLoaderVtbl *vtbl;
#ifdef WINAMP
	AATR *disk;
#endif
};

static int ASAPFileLoader_Load(const ASAPFileLoader *self, const char *filename, uint8_t *buffer, int length)
{
#ifdef WINAMP
	if (self->disk != NULL) {
		AATRDirectory *directory = AATRDirectory_New();
		if (directory == NULL)
			return -1;
		AATRDirectory_OpenRoot(directory, self->disk);
		int result = -1;
		if (AATRDirectory_FindEntryRecursively(directory, filename) && !AATRDirectory_IsEntryDirectory(directory)) {
			AATRFileStream *stream = AATRFileStream_New();
			if (stream != NULL) {
				AATRFileStream_Open(stream, directory);
				result = AATRFileStream_Read(stream, buffer, 0, length);
				AATRFileStream_Delete(stream);
			}
		}
		AATRDirectory_Delete(directory);
		return result;
	}
#endif
	HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return -1;
	BOOL ok = ReadFile(fh, buffer, length, (LPDWORD) &length, NULL);
	CloseHandle(fh);
	return ok ? length : -1;
}

static ASAPFileLoader *ASAPFileLoader_Open(const char **filename)
{
	static const ASAPFileLoaderVtbl loader_vtbl = { ASAPFileLoader_Load };
	static ASAPFileLoader loader = { &loader_vtbl };
#ifdef WINAMP
	loader.disk = NULL;
	const char *f = *filename;
	for (int i = 0; i < MAX_PATH - 4 && f[i] != '\0'; i++) {
		if (_strnicmp(f + i, ".atr#", 5) == 0) {
			i += 4;
			char atr_filename[MAX_PATH];
			memcpy(atr_filename, f, i);
			atr_filename[i] = '\0';
			AATR *disk = AATRStdio_New(atr_filename);
			if (disk == NULL)
				return NULL;
			*filename = f + i + 1;
			loader.disk = disk;
			break;
		}
	}
#endif
	return &loader;
}

static void ASAPFileLoader_Close(ASAPFileLoader *self)
{
#ifdef WINAMP
	if (self->disk != NULL)
		AATRStdio_Delete(self->disk);
#endif
}

static bool loadInfoInternal(ASAPInfo *info, const char *filename, uint8_t *module, int *module_len)
{
	ASAPFileLoader *loader = ASAPFileLoader_Open(&filename);
	if (loader == NULL)
		return false;
	*module_len = ASAPFileLoader_Load(loader, filename, module, ASAPInfo_MAX_MODULE_LENGTH);
	ASAPFileLoader_Close(loader);
	return *module_len >= 0 && ASAPInfo_Load(info, filename, module, *module_len);
}

bool loadInfo(ASAPInfo *info, const char *filename)
{
	uint8_t module[ASAPInfo_MAX_MODULE_LENGTH];
	int module_len;
	return loadInfoInternal(info, filename, module, &module_len);
}

bool loadFiles(ASAP *asap, const char *filename)
{
	ASAPFileLoader *loader = ASAPFileLoader_Open(&filename);
	if (loader == NULL)
		return false;
	bool ok = ASAP_LoadFiles(asap, filename, loader);
	ASAPFileLoader_Close(loader);
	return ok;
}

HWND infoDialog = NULL;

static char playing_filename[MAX_PATH];
static int playing_song = 0;
bool playing_info = false;
static uint8_t saved_module[ASAPInfo_MAX_MODULE_LENGTH];
static int saved_module_len;
static ASAPInfo *edited_info = NULL;
static int edited_song;
static char saved_author[ASAPInfo_MAX_TEXT_LENGTH + 1];
static char saved_title[ASAPInfo_MAX_TEXT_LENGTH + 1];
static char saved_date[ASAPInfo_MAX_TEXT_LENGTH + 1];
static bool saved_ntsc;
static int saved_durations[ASAPInfo_MAX_SONGS];
static bool saved_loops[ASAPInfo_MAX_SONGS];
static bool can_save;
static int invalid_fields;
#define INVALID_FIELD_AUTHOR      1
#define INVALID_FIELD_NAME        2
#define INVALID_FIELD_DATE        4
#define INVALID_FIELD_TIME        8
#define INVALID_FIELD_TIME_SHOW  16
static HWND monthcal = NULL;
static WNDPROC monthcalOriginalWndProc;
static ASTIL *astil = NULL;

static void enableDlgItem(int id, bool enable)
{
	EnableWindow(GetDlgItem(infoDialog, id), enable);
}

static char *appendStilString(char *p, const char *s)
{
	for (;;) {
		char c = *s++;
		switch (c) {
		case '\0':
			return p;
		case '\n':
			c = ' ';
			break;
		default:
			break;
		}
		*p++ = c;
	}
}

static char *appendStil(char *p, const char *prefix, const char *value)
{
	if (value[0] != '\0') {
		p = appendStilString(p, prefix);
		p = appendStilString(p, value);
		*p++ = '\r';
		*p++ = '\n';
	}
	return p;
}

static char *appendAddress(char *p, const char *format, int value)
{
	if (value >= 0)
		p += sprintf(p, format, value);
	return p;
}

static void chomp(char *s)
{
	size_t i = strlen(s);
	if (i >= 2 && s[i - 2] == '\r' && s[i - 1] == '\n')
		s[i - 2] = '\0';
}

static void updateTech(void)
{
	char buf[16000];
	char *p = buf;
	const char *ext = ASAPInfo_GetOriginalModuleExt(edited_info);
	if (ext != NULL)
		p += sprintf(p, "Composed in %s\r\n", ASAPInfo_GetExtDescription(ext));
	int i = ASAPInfo_GetSongs(edited_info);
	if (i > 1) {
		p += sprintf(p, "SONGS %d\r\n", i);
		i = ASAPInfo_GetDefaultSong(edited_info);
		if (i > 0)
			p += sprintf(p, "DEFSONG %d (song %d)\r\n", i, i + 1);
	}
	p += sprintf(p, ASAPInfo_GetChannels(edited_info) > 1 ? "STEREO\r\n" : "MONO\r\n");
	// p += sprintf(p, ASAPInfo_IsNtsc(edited_info) ? "NTSC\r\n" : "PAL\r\n");
	int type = ASAPInfo_GetTypeLetter(edited_info);
	if (type != 0)
		p += sprintf(p, "TYPE %c\r\n", type);
	p += sprintf(p, "FASTPLAY %d (%d Hz)\r\n", ASAPInfo_GetPlayerRateScanlines(edited_info), ASAPInfo_GetPlayerRateHz(edited_info));
	if (type == 'C')
		p += sprintf(p, "MUSIC %04X\r\n", ASAPInfo_GetMusicAddress(edited_info));
	if (type != 0) {
		p = appendAddress(p, "INIT %04X\r\n", ASAPInfo_GetInitAddress(edited_info));
		p = appendAddress(p, "PLAYER %04X\r\n", ASAPInfo_GetPlayerAddress(edited_info));
		p = appendAddress(p, "COVOX %04X\r\n", ASAPInfo_GetCovoxAddress(edited_info));
	}
	i = ASAPInfo_GetSapHeaderLength(edited_info);
	if (i >= 0) {
		while (p < buf + sizeof(buf) - 17 && i + 4 < saved_module_len) {
			int start = saved_module[i] + (saved_module[i + 1] << 8);
			if (start == 0xffff) {
				i += 2;
				start = saved_module[i] + (saved_module[i + 1] << 8);
			}
			int end = saved_module[i + 2] + (saved_module[i + 3] << 8);
			p += sprintf(p, "LOAD %04X-%04X\r\n", start, end);
			i += 5 + end - start;
		}
	}
	chomp(buf);
	SendDlgItemMessageA(infoDialog, IDC_TECHINFO, WM_SETTEXT, 0, (LPARAM) buf);
}

static void updateStil(void)
{
	char buf[16000];
	char *p = buf;
	p = appendStil(p, "", ASTIL_GetTitle(astil));
	p = appendStil(p, "by ", ASTIL_GetAuthor(astil));
	p = appendStil(p, "Directory comment: ", ASTIL_GetDirectoryComment(astil));
	p = appendStil(p, "File comment: ", ASTIL_GetFileComment(astil));
	p = appendStil(p, "Song comment: ", ASTIL_GetSongComment(astil));
	for (int i = 0; ; i++) {
		const ASTILCover *cover = ASTIL_GetCover(astil, i);
		if (cover == NULL)
			break;
		int startSeconds = ASTILCover_GetStartSeconds(cover);
		if (startSeconds >= 0) {
			int endSeconds = ASTILCover_GetEndSeconds(cover);
			if (endSeconds >= 0)
				p += sprintf(p, "At %d:%02d-%d:%02d c", startSeconds / 60, startSeconds % 60, endSeconds / 60, endSeconds % 60);
			else
				p += sprintf(p, "At %d:%02d c", startSeconds / 60, startSeconds % 60);
		}
		else
			*p++ = 'C';
		const char *s = ASTILCover_GetTitleAndSource(cover);
		p = appendStil(p, "overs: ", s[0] != '\0' ? s : "<?>");
		p = appendStil(p, "by ", ASTILCover_GetArtist(cover));
		p = appendStil(p, "Comment: ", ASTILCover_GetComment(cover));
	}
	*p = '\0';
	chomp(buf);
	if (ASTIL_IsUTF8(astil)) {
		WCHAR wBuf[16000];
		if (MultiByteToWideChar(CP_UTF8, 0, buf, -1, wBuf, 16000) > 0) {
			SendDlgItemMessageW(infoDialog, IDC_STILINFO, WM_SETTEXT, 0, (LPARAM) wBuf);
			return;
		}
	}
	SendDlgItemMessageA(infoDialog, IDC_STILINFO, WM_SETTEXT, 0, (LPARAM) buf);
}

static void updateTime(void)
{
	unsigned char str[ASAPWriter_MAX_DURATION_LENGTH + 1];
	int len = ASAPWriter_DurationToString(str, ASAPInfo_GetDuration(edited_info, edited_song));
	str[len] = '\0';
	SendDlgItemMessageA(infoDialog, IDC_TIME, WM_SETTEXT, 0, (LPARAM) str);
}

static void setEditedSong(int song)
{
	edited_song = song;
	updateTime();
	CheckDlgButton(infoDialog, IDC_LOOP, ASAPInfo_GetLoop(edited_info, song) ? BST_CHECKED : BST_UNCHECKED);
	enableDlgItem(IDC_LOOP, ASAPInfo_GetDuration(edited_info, song) > 0);

	char filename[MAX_PATH];
	SendDlgItemMessageA(infoDialog, IDC_FILENAME, WM_GETTEXT, MAX_PATH, (LPARAM) filename);
	ASTIL_Load(astil, filename, song);
	SendDlgItemMessageA(infoDialog, IDC_STILFILE, WM_SETTEXT, 0, (LPARAM) ASTIL_GetStilFilename(astil));
	updateStil();
}

static void showEditTip(int nID, LPCWSTR title, LPCWSTR message)
{
	EDITBALLOONTIP ebt = { sizeof(EDITBALLOONTIP), title, message, TTI_ERROR };
	SendDlgItemMessageW(infoDialog, nID, EM_SHOWBALLOONTIP, 0, (LPARAM) &ebt);
}

static bool isExt(const char *filename, const char *ext)
{
	return _stricmp(filename + strlen(filename) - 4, ext) == 0;
}

static void setSaved(void)
{
	strcpy(saved_author, ASAPInfo_GetAuthor(edited_info));
	strcpy(saved_title, ASAPInfo_GetTitle(edited_info));
	strcpy(saved_date, ASAPInfo_GetDate(edited_info));
	saved_ntsc = ASAPInfo_IsNtsc(edited_info);
	for (int i = 0; i < ASAPInfo_GetSongs(edited_info); i++) {
		saved_durations[i] = ASAPInfo_GetDuration(edited_info, i);
		saved_loops[i] = ASAPInfo_GetLoop(edited_info, i);
	}
}

static bool infoChanged(void)
{
	if (strcmp(ASAPInfo_GetAuthor(edited_info), saved_author) != 0
	 || strcmp(ASAPInfo_GetTitle(edited_info), saved_title) != 0
	 || strcmp(ASAPInfo_GetDate(edited_info), saved_date) != 0
	 || ASAPInfo_IsNtsc(edited_info) != saved_ntsc)
		return true;
	for (int i = 0; i < ASAPInfo_GetSongs(edited_info); i++) {
		if (ASAPInfo_GetDuration(edited_info, i) != saved_durations[i])
			return true;
		if (saved_durations[i] >= 0 && ASAPInfo_GetLoop(edited_info, i) != saved_loops[i])
			return true;
	}
	return false;
}

static void updateSaveButtons(int mask, bool ok)
{
	if (ok) {
		invalid_fields &= ~mask;
		ok = invalid_fields == 0;
	}
	else
		invalid_fields |= mask;
	if (can_save)
		enableDlgItem(IDC_SAVE, ok && infoChanged());
	enableDlgItem(IDC_SAVEAS, ok);
}

static void updateInfoString(HWND hDlg, int nID, int mask, bool (*func)(ASAPInfo *, const char *))
{
	char str[ASAPInfo_MAX_TEXT_LENGTH + 1];
	SendDlgItemMessageA(hDlg, nID, WM_GETTEXT, ASAPInfo_MAX_TEXT_LENGTH + 1, (LPARAM) str);
	bool ok = func(edited_info, str);
	updateSaveButtons(mask, ok);
	if (!ok)
		showEditTip(nID, L"Invalid characters", L"Avoid national characters and quotation marks");
}

static LRESULT CALLBACK MonthCalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_KILLFOCUS:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_DESTROY:
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR) monthcalOriginalWndProc);
		break;
	default:
		break;
	}
	return CallWindowProc(monthcalOriginalWndProc, hWnd, uMsg, wParam, lParam);
}

static void toggleCalendar(HWND hDlg)
{
	if (monthcal == NULL) {
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof(icex);
		icex.dwICC = ICC_DATE_CLASSES;
		InitCommonControlsEx(&icex);
		monthcal = CreateWindowExA(0, MONTHCAL_CLASSA, "", WS_BORDER | WS_POPUP, 0, 0, 0, 0, hDlg, NULL, NULL, NULL); // TODO
		/* subclass month calendar, so that it hides when looses focus */
		monthcalOriginalWndProc = (WNDPROC) SetWindowLongPtr(monthcal, GWLP_WNDPROC, (LONG_PTR) MonthCalWndProc);
	}
	if (IsWindowVisible(monthcal))
		ShowWindow(monthcal, SW_HIDE);
	else {
		RECT rc;
		GetWindowRect(GetDlgItem(hDlg, IDC_PICKDATE), &rc);
		int x = rc.left;
		int y = rc.bottom;
		MonthCal_GetMinReqRect(monthcal, &rc);
		SetWindowPos(monthcal, NULL, x, y, rc.right, rc.bottom, SWP_SHOWWINDOW | SWP_NOZORDER);
		y = ASAPInfo_GetYear(edited_info);
		if (y > 0) {
			SYSTEMTIME st;
			st.wYear = y;
			int month = ASAPInfo_GetMonth(edited_info);
			DWORD view;
			if (month > 0) {
				int day = ASAPInfo_GetDayOfMonth(edited_info);
				st.wMonth = month;
				if (day > 0) {
					st.wDay = day;
					view = MCMV_MONTH;
				}
				else {
					st.wDay = 1;
					view = MCMV_YEAR;
				}
			}
			else {
				st.wMonth = 1;
				st.wDay = 1;
				view = MCMV_DECADE;
			}
			(void) MonthCal_SetCurSel(monthcal, &st);
			(void) MonthCal_SetCurrentView(monthcal, view);
		}
		SetFocus(monthcal);
	}
}

typedef struct {
	bool (*save)(ASAPWriter *self, const char *filename, const uint8_t *buffer, int offset, int length);
} ASAPWriterVtbl;

static bool ASAPWriter_Save(ASAPWriter *self, const char *filename, const uint8_t *buffer, int offset, int length)
{
	HANDLE fh = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return false;
	bool ok = WriteFile(fh, buffer, length, (LPDWORD) &length, NULL) != 0;
	CloseHandle(fh);
	return ok;
}

static bool doSaveFile(const char *filename, bool tag)
{
	char sourceFilename[MAX_PATH];
	SendDlgItemMessageA(infoDialog, IDC_FILENAME, WM_GETTEXT, MAX_PATH, (LPARAM) sourceFilename);
	const char *sourceFilenamePtr = sourceFilename;
	ASAPFileLoader *loader = ASAPFileLoader_Open(&sourceFilenamePtr);
	if (loader == NULL)
		return false;
	ASAPWriter *writer = ASAPWriter_New();
	if (writer == NULL) {
		ASAPFileLoader_Close(loader);
		return false;
	}
	static const ASAPWriterVtbl writer_vtbl = { ASAPWriter_Save };
	*(const ASAPWriterVtbl **) writer = &writer_vtbl;
	ASAPWriter_SetInput(writer, sourceFilenamePtr, loader);
	bool ok = ASAPWriter_Write(writer, filename, edited_info, saved_module, saved_module_len, tag);
	ASAPWriter_Delete(writer);
	ASAPFileLoader_Close(loader);
	return ok;
}

static bool saveFile(const char *filename, bool tag)
{
	bool isSap = isExt(filename, ".sap");
	if (isSap) {
		int song = ASAPInfo_GetSongs(edited_info);
		while (--song >= 0 && ASAPInfo_GetDuration(edited_info, song) < 0);
		while (--song >= 0) {
			if (ASAPInfo_GetDuration(edited_info, song) < 0) {
				MessageBoxA(infoDialog, "Cannot save file because time not set for all songs", "Error", MB_OK | MB_ICONERROR);
				return false;
			}
		}
	}
	if (!doSaveFile(filename, tag)) {
		MessageBoxA(infoDialog, "Cannot save file", "Error", MB_OK | MB_ICONERROR);
		// FIXME: delete file
		return false;
	}
	if (isSap) {
		setSaved();
		enableDlgItem(IDC_SAVE, false);
	}
	return true;
}

static bool saveInfo(void)
{
	char filename[MAX_PATH];
	SendDlgItemMessageA(infoDialog, IDC_FILENAME, WM_GETTEXT, MAX_PATH, (LPARAM) filename);
	return saveFile(filename, false);
}

static void setSaveFilters(char *p)
{
	const char *exts[ASAPWriter_MAX_SAVE_EXTS];
	int exts_len = ASAPWriter_GetSaveExts(exts, edited_info);
	for (int i = 0; i < exts_len; i++) {
		const char *ext = exts[i];
		p += sprintf(p, "%s (*.%s)", ASAPInfo_GetExtDescription(ext), ext) + 1;
		p += sprintf(p, "*.%s", ext) + 1;
	}
	*p = '\0';
}

static bool saveInfoAs(void)
{
	char filename[MAX_PATH];
	char filter[1024];
	OPENFILENAMEA ofn = {
		sizeof(OPENFILENAMEA),
		infoDialog,
		0,
		filter,
		NULL,
		0,
		1,
		filename,
		MAX_PATH,
		NULL,
		0,
		NULL,
		"Select output file",
		OFN_ENABLESIZING | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		0,
		0,
		NULL,
		0,
		NULL,
		NULL
	};
	bool tag = false;
	setSaveFilters(filter);
	SendDlgItemMessageA(infoDialog, IDC_FILENAME, WM_GETTEXT, MAX_PATH, (LPARAM) filename);
	char *ext = strrchr(filename, '.');
	if (ext != NULL) {
		*ext = '\0';
		ofn.lpstrDefExt = ext + 1;
	}
	if (!GetSaveFileNameA(&ofn))
		return false;
	if (isExt(filename, ".xex")) {
		switch (MessageBoxA(infoDialog, "Do you want to display information during playback?", "ASAP", MB_YESNOCANCEL | MB_ICONQUESTION)) {
		case IDYES:
			tag = true;
			break;
		case IDCANCEL:
			return false;
		default:
			break;
		}
	}
	return saveFile(filename, tag);
}

static void closeInfoDialog(void)
{
	DestroyWindow(infoDialog);
	infoDialog = NULL;
	ASAPInfo_Delete(edited_info);
	edited_info = NULL;
	ASTIL_Delete(astil);
	astil = NULL;
}

static INT_PTR CALLBACK infoDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef FOOBAR2000
		setDarkInfoDialog(hDlg);
#endif
#ifdef PLAYING_INFO
		CheckDlgButton(hDlg, IDC_PLAYING, playing_info ? BST_CHECKED : BST_UNCHECKED);
#endif
		SendDlgItemMessage(hDlg, IDC_AUTHOR, EM_LIMITTEXT, ASAPInfo_MAX_TEXT_LENGTH, 0);
		SendDlgItemMessage(hDlg, IDC_NAME, EM_LIMITTEXT, ASAPInfo_MAX_TEXT_LENGTH, 0);
		SendDlgItemMessage(hDlg, IDC_DATE, EM_LIMITTEXT, ASAPInfo_MAX_TEXT_LENGTH, 0);
		SendDlgItemMessage(hDlg, IDC_TIME, EM_LIMITTEXT, ASAPWriter_MAX_DURATION_LENGTH, 0);
		return TRUE;
	case WM_COMMAND:
		switch (wParam) {
#ifdef PLAYING_INFO
		case MAKEWPARAM(IDC_PLAYING, BN_CLICKED):
			playing_info = (IsDlgButtonChecked(hDlg, IDC_PLAYING) == BST_CHECKED);
			if (playing_info && playing_filename[0] != '\0')
				updateInfoDialog(playing_filename, playing_song);
			onUpdatePlayingInfo();
			return TRUE;
#endif
		case MAKEWPARAM(IDC_AUTHOR, EN_CHANGE):
			updateInfoString(hDlg, IDC_AUTHOR, INVALID_FIELD_AUTHOR, ASAPInfo_SetAuthor);
			return TRUE;
		case MAKEWPARAM(IDC_NAME, EN_CHANGE):
			updateInfoString(hDlg, IDC_NAME, INVALID_FIELD_NAME, ASAPInfo_SetTitle);
			return TRUE;
		case MAKEWPARAM(IDC_DATE, EN_CHANGE):
			updateInfoString(hDlg, IDC_DATE, INVALID_FIELD_DATE, ASAPInfo_SetDate);
			return TRUE;
		case MAKEWPARAM(IDC_PICKDATE, BN_CLICKED):
			toggleCalendar(hDlg);
			return TRUE;
		case MAKEWPARAM(IDC_TIME, EN_CHANGE):
			{
				char str[ASAPWriter_MAX_DURATION_LENGTH + 1];
				SendDlgItemMessageA(hDlg, IDC_TIME, WM_GETTEXT, ASAPWriter_MAX_DURATION_LENGTH + 1, (LPARAM) str);
				int duration = ASAPInfo_ParseDuration(str);
				ASAPInfo_SetDuration(edited_info, edited_song, duration);
				enableDlgItem(IDC_LOOP, str[0] != '\0');
				updateSaveButtons(INVALID_FIELD_TIME | INVALID_FIELD_TIME_SHOW, duration >=0 || str[0] == '\0');
			}
			return TRUE;
		case MAKEWPARAM(IDC_TIME, EN_KILLFOCUS):
			if ((invalid_fields & INVALID_FIELD_TIME_SHOW) != 0) {
				invalid_fields &= ~INVALID_FIELD_TIME_SHOW;
				showEditTip(IDC_TIME, L"Invalid format", L"Please type MM:SS.mmm");
			}
			return TRUE;
		case MAKEWPARAM(IDC_LOOP, BN_CLICKED):
			ASAPInfo_SetLoop(edited_info, edited_song, IsDlgButtonChecked(hDlg, IDC_LOOP) == BST_CHECKED);
			updateSaveButtons(0, true);
			return TRUE;
		case MAKEWPARAM(IDC_NTSC, BN_CLICKED):
			ASAPInfo_SetNtsc(edited_info, IsDlgButtonChecked(hDlg, IDC_NTSC) == BST_CHECKED);
			updateTech();
			updateTime();
			updateSaveButtons(0, true);
			return TRUE;
		case MAKEWPARAM(IDC_SONGNO, CBN_SELCHANGE):
			setEditedSong((int) SendDlgItemMessage(hDlg, IDC_SONGNO, CB_GETCURSEL, 0, 0));
			updateSaveButtons(INVALID_FIELD_TIME | INVALID_FIELD_TIME_SHOW, true);
			return TRUE;
		case MAKEWPARAM(IDC_SAVE, BN_CLICKED):
			saveInfo();
			return TRUE;
		case MAKEWPARAM(IDC_SAVEAS, BN_CLICKED):
			saveInfoAs();
			return TRUE;
		case MAKEWPARAM(IDCANCEL, BN_CLICKED):
			if (invalid_fields == 0 && infoChanged()) {
				switch (MessageBoxA(hDlg, "Save changes?", "ASAP", MB_YESNOCANCEL | MB_ICONQUESTION)) {
				case IDYES:
					if (!saveInfoAs())
						return TRUE;
					break;
				case IDCANCEL:
					return TRUE;
				default:
					break;
				}
			}
			closeInfoDialog();
			return TRUE;
		}
		break;
	case WM_NOTIFY: {
			LPNMSELCHANGE psc = (LPNMSELCHANGE) lParam;
			if (psc->nmhdr.hwndFrom == monthcal && psc->nmhdr.code == MCN_SELECT) {
				ShowWindow(monthcal, SW_HIDE);
				char str[32];
				sprintf(str, "%02d/%02d/%d", psc->stSelStart.wDay, psc->stSelStart.wMonth, psc->stSelStart.wYear);
				SetDlgItemTextA(hDlg, IDC_DATE, str);
				ASAPInfo_SetDate(edited_info, str);
				updateSaveButtons(INVALID_FIELD_DATE, true);
			}
		}
		break;
	case WM_DESTROY:
#ifdef FOOBAR2000
		releaseDarkInfoDialog();
#endif
		if (monthcal != NULL) {
			DestroyWindow(monthcal);
			monthcal = NULL;
		}
		return 0;
	default:
		break;
	}
	return FALSE;
}

void showInfoDialog(HINSTANCE hInstance, HWND hwndParent, const char *filename, int song)
{
	if (infoDialog == NULL)
		infoDialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_INFO), hwndParent, infoDialogProc);
	if (playing_info || filename == NULL)
		updateInfoDialog(playing_filename, playing_song);
	else
		updateInfoDialog(filename, song);
}

void updateInfoDialog(const char *filename, int song)
{
	if (infoDialog == NULL)
		return;
	if (edited_info == NULL) {
		edited_info = ASAPInfo_New();
		if (astil == NULL)
			astil = ASTIL_New();
		if (edited_info == NULL || astil == NULL) {
			closeInfoDialog();
			return;
		}
	}
	else if (infoChanged())
		return;
	if (!loadInfoInternal(edited_info, filename, saved_module, &saved_module_len)) {
		closeInfoDialog();
		return;
	}
	setSaved();
	can_save = isExt(filename, ".sap");
	invalid_fields = 0;
	SendDlgItemMessageA(infoDialog, IDC_FILENAME, WM_SETTEXT, 0, (LPARAM) filename);
	SendDlgItemMessageA(infoDialog, IDC_AUTHOR, WM_SETTEXT, 0, (LPARAM) saved_author);
	SendDlgItemMessageA(infoDialog, IDC_NAME, WM_SETTEXT, 0, (LPARAM) saved_title);
	SendDlgItemMessageA(infoDialog, IDC_DATE, WM_SETTEXT, 0, (LPARAM) saved_date);
	SendDlgItemMessageA(infoDialog, IDC_SONGNO, CB_RESETCONTENT, 0, 0);
	int songs = ASAPInfo_GetSongs(edited_info);
	enableDlgItem(IDC_SONGNO, songs > 1);
	for (int i = 1; i <= songs; i++) {
		char str[16];
		sprintf(str, "%d", i);
		SendDlgItemMessageA(infoDialog, IDC_SONGNO, CB_ADDSTRING, 0, (LPARAM) str);
	}
	if (song < 0)
		song = ASAPInfo_GetDefaultSong(edited_info);
	SendDlgItemMessage(infoDialog, IDC_SONGNO, CB_SETCURSEL, song, 0);
	setEditedSong(song);
	CheckDlgButton(infoDialog, IDC_NTSC, ASAPInfo_IsNtsc(edited_info) ? BST_CHECKED : BST_UNCHECKED);
	enableDlgItem(IDC_NTSC, ASAPInfo_CanSetNtsc(edited_info));
	enableDlgItem(IDC_SAVE, false);
	updateTech();
}

void setPlayingSong(const char *filename, int song)
{
	if (filename != NULL && strlen(filename) < MAX_PATH - 1)
		strcpy(playing_filename, filename);
	playing_song = song;
	if (playing_info)
		updateInfoDialog(playing_filename, song);
}

const ASTIL *getPlayingASTIL(void)
{
	if (astil == NULL)
		astil = ASTIL_New();
	ASTIL_Load(astil, playing_filename, playing_song);
	return astil;
}
