/*
 * wasap.c - Another Slight Atari Player for Win32 systems
 *
 * Copyright (C) 2005-2026  Piotr Fusik
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
#include <commctrl.h>
#include <stdbool.h>
#include <stdio.h>
#include <shellapi.h>

#include "asap.h"
#include "info_dlg.h"
#include "wasap.h"

#define APP_TITLE        "WASAP"
#define WND_CLASS_NAME   "WASAP"
#define OPEN_TITLE       "Select Atari 8-bit music"
#define BITS_PER_SAMPLE  16
#define BUFFERS          2
#define BUFFERED_BLOCKS  4096

static ASAP *asap;
static int songs = 0;
static char current_filename[MAX_PATH] = "";
static int current_song;

static HWND hWnd;


/* WaveOut ---------------------------------------------------------------- */

static HANDLE waveEvent = NULL;
static HWAVEOUT hwo = INVALID_HANDLE_VALUE;
/* double-buffering, *2 for 16-bit, *2 for stereo */
static char buffer[BUFFERS][BUFFERED_BLOCKS * 2 * 2];
static WAVEHDR wh[BUFFERS];
static HANDLE waveThread = NULL;
static bool playing = false;

static bool WaveOut_Write(LPWAVEHDR pwh)
{
	int len = ASAP_Generate(asap, (PBYTE) pwh->lpData, pwh->dwBufferLength, BITS_PER_SAMPLE == 8 ? ASAPSampleFormat_U8 : ASAPSampleFormat_S16_L_E);
	if (len <= 0)
		return false;
	pwh->dwBufferLength = len;
	pwh->dwFlags &= ~WHDR_DONE;
	return waveOutWrite(hwo, pwh, sizeof(WAVEHDR)) == MMSYSERR_NOERROR;
}

static DWORD WINAPI WaveOut_Thread(LPVOID lpParameter)
{
	while (playing) {
		bool finished = true;
		for (int i = 0; i < BUFFERS; i++) {
			if ((wh[i].dwFlags & WHDR_DONE) == 0 || WaveOut_Write(&wh[i]))
				finished = false;
		}
		if (finished) {
			PostMessage(hWnd, WM_COMMAND, IDM_STOP, 0);
			break;
		}
		WaitForSingleObject(waveEvent, 200);
	}
	return 0;
}

static void WaveOut_Close(void)
{
	if (waveEvent != NULL) {
		if (hwo != INVALID_HANDLE_VALUE) {
			playing = false;
			if (waveThread != NULL) {
				SetEvent(waveEvent);
				WaitForSingleObject(waveThread, 300);
				CloseHandle(waveThread);
				waveThread = NULL;
			}
			waveOutReset(hwo);
			for (int i = 0; i < BUFFERS; i++) {
				if (wh[i].dwFlags & WHDR_PREPARED)
					waveOutUnprepareHeader(hwo, &wh[i], sizeof(WAVEHDR));
			}
			waveOutClose(hwo);
			hwo = INVALID_HANDLE_VALUE;
		}
		CloseHandle(waveEvent);
		waveEvent = NULL;
	}
}

static bool WaveOut_Open(int channels)
{
	waveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (waveEvent == NULL)
		return false;

	WAVEFORMATEX wfx;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = channels;
	wfx.nSamplesPerSec = ASAP_SAMPLE_RATE;
	wfx.nBlockAlign = channels * (BITS_PER_SAMPLE / 8);
	wfx.nAvgBytesPerSec = ASAP_SAMPLE_RATE * wfx.nBlockAlign;
	wfx.wBitsPerSample = BITS_PER_SAMPLE;
	wfx.cbSize = 0;
	if (waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD_PTR) waveEvent, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR)
		return false;

	ZeroMemory(&wh, sizeof(wh));
	for (int i = 0; i < BUFFERS; i++) {
		wh[i].lpData = buffer[i];
		wh[i].dwBufferLength = BUFFERED_BLOCKS * wfx.nBlockAlign;
		if (waveOutPrepareHeader(hwo, &wh[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
			WaveOut_Close();
			return false;
		}
		wh[i].dwFlags |= WHDR_DONE;
	}

	playing = true;
	DWORD waveThreadId;
	waveThread = CreateThread(NULL, 0, WaveOut_Thread, NULL, 0, &waveThreadId);
	if (waveThread == NULL) {
		WaveOut_Close();
		return false;
	}

	return true;
}


/* Tray ------------------------------------------------------------------- */

#define MYWM_NOTIFYICON  (WM_APP + 1)
static NOTIFYICONDATA nid = {
	sizeof(NOTIFYICONDATA),
	NULL,
	15, /* wince: "Values from 0 to 12 are reserved and should not be used." */
	NIF_ICON | NIF_MESSAGE | NIF_TIP,
	MYWM_NOTIFYICON,
	NULL,
	APP_TITLE
};
static UINT taskbarCreatedMessage;

static void Tray_Modify(HICON hIcon)
{
	nid.hIcon = hIcon;
	if (songs > 0) {
		const char *pb;
		const char *pe;
		for (pb = pe = current_filename; *pe != '\0'; pe++) {
			if (*pe == '\\' || *pe == '/')
				pb = pe + 1;
		}
		/* we need to be careful because szTip is only 64 characters */
		/* 5 + 2 + max 33 */
		int len = snprintf(nid.szTip, sizeof(nid.szTip), pe - pb <= 33 ? APP_TITLE ": %s" : APP_TITLE ": %.30s...", pb);
		if (songs > 1) {
			/* 7 + max 3 + 4 + max 3 + 1*/
			snprintf(nid.szTip + len, sizeof(nid.szTip) - len, " (song %d of %d)", current_song + 1, songs);
		}
	}
	else
		strcpy_s(nid.szTip, sizeof(nid.szTip), APP_TITLE);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}


/* GUI -------------------------------------------------------------------- */

static bool popupShown = false;
static bool errorShown = false;
static HINSTANCE hInst;
static HICON hStopIcon;
static HICON hPlayIcon;
static HMENU hTrayMenu;
static HMENU hSongMenu;

static void ShowError(const char *message)
{
	errorShown = true;
	MessageBox(hWnd, message, APP_TITLE, MB_OK | MB_ICONERROR);
	errorShown = false;
}

static void ShowAbout(void)
{
	MSGBOXPARAMS mbp = {
		sizeof(MSGBOXPARAMS),
		hWnd,
		hInst,
		ASAPInfo_CREDITS
		"WASAP icons (C) 2005 Lukasz Sychowicz\n\n"
		ASAPInfo_COPYRIGHT,
		APP_TITLE " " ASAPInfo_VERSION,
		MB_OK | MB_USERICON,
		MAKEINTRESOURCE(IDI_APP),
		0,
		NULL,
		LANG_NEUTRAL
	};
	popupShown = true;
	MessageBoxIndirect(&mbp);
	popupShown = false;
}

static void SetSongsMenu(int n)
{
	int i = GetMenuItemCount(hSongMenu);
	while (i > n)
		DeleteMenu(hSongMenu, --i, MF_BYPOSITION);
	while (++i <= n) {
		char str[3];
		str[0] = i <= 9 ? '&' : '0' + i / 10;
		str[1] = '0' + i % 10;
		str[2] = '\0';
		AppendMenu(hSongMenu, MF_ENABLED | MF_STRING, IDM_SONG1 + i - 1, str);
	}
}

static void OpenMenu(HWND hWnd, HMENU hMenu)
{
	POINT pt;
	GetCursorPos(&pt);
	SetForegroundWindow(hWnd);
	TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
}

static void StopPlayback(void)
{
	WaveOut_Close();
	Tray_Modify(hStopIcon);
	EnableMenuItem(hTrayMenu, IDM_STOP, MF_BYCOMMAND | MF_GRAYED);
}

static bool DoLoad(ASAP *asap)
{
	if (!loadFiles(asap, current_filename)) {
		ShowError("Unsupported file format");
		return false;
	}
	return true;
}

static void LoadAndPlay(int song)
{
	if (songs > 0) {
		songs = 0;
		StopPlayback();
		EnableMenuItem(hTrayMenu, 1, MF_BYPOSITION | MF_GRAYED); /* songs */
		EnableMenuItem(hTrayMenu, IDM_FILE_INFO, MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hTrayMenu, IDM_SAVE_WAV, MF_BYCOMMAND | MF_GRAYED);
	}
	if (!DoLoad(asap))
		return;
	const ASAPInfo *info = ASAP_GetInfo(asap);
	if (song < 0)
		song = ASAPInfo_GetDefaultSong(info);
	songs = ASAPInfo_GetSongs(info);
	EnableMenuItem(hTrayMenu, 1, MF_BYPOSITION | MF_ENABLED); /* songs */
	EnableMenuItem(hTrayMenu, IDM_FILE_INFO, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hTrayMenu, IDM_SAVE_WAV, MF_BYCOMMAND | MF_ENABLED);
	updateInfoDialog(current_filename, song);
	SetSongsMenu(songs);
	CheckMenuRadioItem(hSongMenu, 0, songs - 1, song, MF_BYPOSITION);
	current_song = song;
	int duration;
	if (ASAPInfo_GetLoop(info, song))
		duration = -1;
	else
		duration = ASAPInfo_GetDuration(info, song);
	if (!ASAP_PlaySong(asap, song, duration)) {
		ShowError("This file is a bad rip!");
		return;
	}
	if (!WaveOut_Open(ASAPInfo_GetChannels(info))) {
		ShowError("Error initalizing WaveOut");
		return;
	}
	EnableMenuItem(hTrayMenu, IDM_STOP, MF_BYCOMMAND | MF_ENABLED);
	Tray_Modify(hPlayIcon);
}

static void SelectAndLoadFile(void)
{
	static OPENFILENAME ofn = {
		sizeof(OPENFILENAME),
		NULL,
		0,
		"All supported\0"
		"*.sap;*.cmc;*.cm3;*.cmr;*.cms;*.dmc;*.dlt;*.mpt;*.md1;*.md2;*.mpd;*.rmt;*.tmc;*.tm8;*.tm2;*.fc;*.d15;*.d8\0"
#define ASAP_FILTER(description, masks) description " (" masks ")\0" masks "\0"
		ASAP_FILTER("Slight Atari Player", "*.sap")
		ASAP_FILTER("Chaos Music Composer", "*.cmc;*.cm3;*.cmr;*.cms;*.dmc")
		ASAP_FILTER("Delta Music Composer", "*.dlt")
		ASAP_FILTER("Music ProTracker", "*.mpt;*.mpd;*.md1;*.md2")
		ASAP_FILTER("Raster Music Tracker", "*.rmt")
		ASAP_FILTER("Theta Music Composer 1.x", "*.tmc;*.tm8")
		ASAP_FILTER("Theta Music Composer 2.x", "*.tm2")
		ASAP_FILTER("Future Composer", "*.fc")
		ASAP_FILTER("Music ProTracker samples", "*.d15;*.d8")
		"\0",
		NULL,
		0,
		1,
		current_filename,
		MAX_PATH,
		NULL,
		0,
		NULL,
		OPEN_TITLE,
		OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		0,
		0,
		NULL,
		0,
		NULL,
		NULL
	};
	popupShown = true;
	ofn.hwndOwner = hWnd;
	if (GetOpenFileName(&ofn))
		LoadAndPlay(-1);
	popupShown = false;
}

/* Defined so that the progress bar is responsive, but isn't updated too often and doesn't overflow (65535 limit) */
#define WAV_PROGRESS_DURATION_SHIFT 10

static int wav_progressLimit;
static char wav_filename[MAX_PATH];

static INT_PTR CALLBACK WavProgressDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
		SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, wav_progressLimit));
	return FALSE;
}

static bool DoSaveWav(ASAP *asap)
{
	HANDLE fh = CreateFile(wav_filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return false;
	HWND progressWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_PROGRESS), hWnd, WavProgressDialogProc);
	int progressPos = 0;
	BYTE buffer[8192];
	DWORD len = ASAP_GetWavHeader(asap, buffer, BITS_PER_SAMPLE == 8 ? ASAPSampleFormat_U8 : ASAPSampleFormat_S16_L_E, false);
	while (len > 0) {
		if (!WriteFile(fh, buffer, len, &len, NULL)) {
			DestroyWindow(progressWnd);
			CloseHandle(fh);
			return false;
		}
		int newProgressPos = ASAP_GetPosition(asap) >> WAV_PROGRESS_DURATION_SHIFT;
		if (progressPos != newProgressPos) {
			progressPos = newProgressPos;
			SendDlgItemMessage(progressWnd, IDC_PROGRESS, PBM_SETPOS, progressPos, 0);
		}
		len = ASAP_Generate(asap, buffer, sizeof(buffer), BITS_PER_SAMPLE == 8 ? ASAPSampleFormat_U8 : ASAPSampleFormat_S16_L_E);
	}
	DestroyWindow(progressWnd);
	return CloseHandle(fh);
}

static void SaveWav(void)
{
	ASAP *asap = ASAP_New();
	if (asap == NULL)
		return;
	if (!DoLoad(asap))
		return;
	int duration = ASAPInfo_GetDuration(ASAP_GetInfo(asap), current_song);
	if (duration < 0) {
		if (MessageBox(hWnd,
			"This song has unknown duration.\n"
			"Use \"File information\" to update the source file with the correct duration.\n"
			"Do you want to save 3 minutes?",
			"Unknown duration", MB_YESNO | MB_ICONWARNING) != IDYES)
			return;
		duration = 180000;
	}
	ASAP_PlaySong(asap, current_song, duration);

	snprintf(wav_filename, sizeof(wav_filename), "%.*s.wav", (int) (strrchr(current_filename, '.') - current_filename), current_filename);
	popupShown = true;
	static OPENFILENAME ofn = {
		sizeof(OPENFILENAME),
		NULL,
		0,
		"WAV files (*.wav)\0*.wav\0\0",
		NULL,
		0,
		0,
		wav_filename,
		MAX_PATH,
		NULL,
		0,
		NULL,
		"Select output file",
		OFN_ENABLESIZING | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		0,
		0,
		"wav",
		0,
		NULL,
		NULL
	};
	ofn.hwndOwner = hWnd;
	if (GetSaveFileName(&ofn)) {
		wav_progressLimit = duration >> WAV_PROGRESS_DURATION_SHIFT;
		if (!DoSaveWav(asap))
			ShowError("Cannot save file");
	}
	popupShown = false;
}

static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int idc;
	PCOPYDATASTRUCT pcds;
	switch (msg) {
	case WM_COMMAND:
		idc = LOWORD(wParam);
		switch (idc) {
		case IDM_OPEN:
			SelectAndLoadFile();
			break;
		case IDM_STOP:
			StopPlayback();
			break;
		case IDM_FILE_INFO:
			showInfoDialog(hInst, hWnd, current_filename, current_song);
			break;
		case IDM_SAVE_WAV:
			SaveWav();
			break;
		case IDM_ABOUT:
			ShowAbout();
			break;
		case IDM_EXIT:
			PostQuitMessage(0);
			break;
		default:
			if (idc >= IDM_SONG1 && idc < IDM_SONG1 + songs)
				LoadAndPlay(idc - IDM_SONG1);
			break;
		}
		break;
	case WM_APPCOMMAND:
		switch (GET_APPCOMMAND_LPARAM(lParam)) {
		case APPCOMMAND_MEDIA_STOP:
			StopPlayback();
			return TRUE;
		case APPCOMMAND_MEDIA_PLAY:
			if (current_song < songs) {
				LoadAndPlay(current_song);
				return TRUE;
			}
			break;
		case APPCOMMAND_MEDIA_PREVIOUSTRACK:
			if (songs > 0 && current_song > 0) {
				LoadAndPlay(current_song - 1);
				return TRUE;
			}
			break;
		case APPCOMMAND_MEDIA_NEXTTRACK:
			if (current_song + 1 < songs) {
				LoadAndPlay(current_song + 1);
				return TRUE;
			}
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case MYWM_NOTIFYICON:
		if (popupShown) {
			SetForegroundWindow(hWnd);
			break;
		}
		switch (lParam) {
		case WM_LBUTTONDOWN:
			SelectAndLoadFile();
			break;
		case WM_MBUTTONDOWN:
			if (songs > 1)
				OpenMenu(hWnd, hSongMenu);
			break;
		case WM_RBUTTONUP:
			OpenMenu(hWnd, hTrayMenu);
			break;
		default:
			break;
		}
		break;
	case WM_COPYDATA:
		pcds = (PCOPYDATASTRUCT) lParam;
		if (pcds->dwData == 'O' && pcds->cbData <= sizeof(current_filename)) {
			if (errorShown) {
				HWND hChild = GetLastActivePopup(hWnd);
				if (hChild != hWnd)
					SendMessage(hChild, WM_CLOSE, 0, 0);
			}
			memcpy(current_filename, pcds->lpData, pcds->cbData);
			LoadAndPlay(-1);
			return TRUE;
		}
		break;
	default:
		if (msg == taskbarCreatedMessage)
			Shell_NotifyIcon(NIM_ADD, &nid);
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LPSTR pb;
	LPSTR pe;
	for (pb = lpCmdLine; *pb == ' ' || *pb == '\t'; pb++);
	for (pe = pb; *pe != '\0'; pe++);
	while (--pe > pb && (*pe == ' ' || *pe == '\t'));
	/* Now pb and pe point at respectively the first and last non-blank
	   character in lpCmdLine. If pb > pe then the command line is blank. */
	if (*pb == '"' && *pe == '"')
		pb++;
	else
		pe++;
	*pe = '\0';
	/* Now pb contains the filename, if any, specified on the command line. */

	hWnd = FindWindow(WND_CLASS_NAME, NULL);
	if (hWnd != NULL) {
		/* an instance of WASAP is already running */
		if (*pb != '\0') {
			/* pass the filename */
			COPYDATASTRUCT cds = { 'O', (DWORD) (pe + 1 - pb), pb };
			SendMessage(hWnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM) &cds);
		}
		else {
			SetForegroundWindow(hWnd);
			PostMessage(hWnd, MYWM_NOTIFYICON, 15, WM_LBUTTONDOWN);
		}
		return 0;
	}

	asap = ASAP_New();
	if (asap == NULL)
		return 1;

	hInst = hInstance;

	WNDCLASS wc;
	wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WND_CLASS_NAME;
	RegisterClass(&wc);

	hWnd = CreateWindow(WND_CLASS_NAME,
		APP_TITLE,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	hStopIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_STOP));
	hPlayIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLAY));
	HMENU hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_TRAYMENU));
	hTrayMenu = GetSubMenu(hMainMenu, 0);
	hSongMenu = GetSubMenu(hTrayMenu, 1);
	SetMenuDefaultItem(hTrayMenu, 0, TRUE);
	nid.hWnd = hWnd;
	nid.hIcon = hStopIcon;
	Shell_NotifyIcon(NIM_ADD, &nid);
	taskbarCreatedMessage = RegisterWindowMessage("TaskbarCreated");

	if (*pb != '\0' && pe + 1 - pb <= sizeof(current_filename)) {
		memcpy(current_filename, pb, pe + 1 - pb);
		LoadAndPlay(-1);
	}
	else
		SelectAndLoadFile();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (infoDialog == NULL || !IsDialogMessage(infoDialog, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WaveOut_Close();
	Shell_NotifyIcon(NIM_DELETE, &nid);
	DestroyMenu(hMainMenu);
	return 0;
}
