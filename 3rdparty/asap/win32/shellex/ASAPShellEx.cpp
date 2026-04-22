/*
 * ASAPShellEx.cpp - ASAP Property Handler shell extension
 *
 * Copyright (C) 2010-2026  Piotr Fusik
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

#include <stdio.h>
#define _WIN32_IE 0x500
#include <propvarutil.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "asap.h"

static const char extensions[][5] =
	{ ".sap", ".cmc", ".cm3", ".cmr", ".cms", ".dmc", ".dlt", ".mpt", ".md1", ".md2", ".mpd", ".rmt", ".tmc", ".tm8", ".tm2", ".fc", ".d15", ".d8" };

static HINSTANCE g_hDll;
static LONG g_cRef = 0;

static void DllAddRef()
{
	InterlockedIncrement(&g_cRef);
}

static void DllRelease()
{
	InterlockedDecrement(&g_cRef);
}

#define CLSID_ASAPMetadataHandler_str "{5AE26367-B5CF-444D-B163-2CBC99B41287}"
static const GUID CLSID_ASAPMetadataHandler =
	{ 0x5ae26367, 0xb5cf, 0x444d, { 0xb1, 0x63, 0x2c, 0xbc, 0x99, 0xb4, 0x12, 0x87 } };

static const SHCOLUMNID g_scids[] = {
	{ FMTID_SummaryInformation, PIDSI_TITLE },
	{ FMTID_SummaryInformation, PIDSI_AUTHOR },
	{ FMTID_MUSIC, PIDSI_ARTIST },
	{ FMTID_MUSIC, PIDSI_YEAR },
	{ FMTID_AudioSummaryInformation, PIDASI_TIMELENGTH },
	{ FMTID_AudioSummaryInformation, PIDASI_CHANNEL_COUNT },
	{ CLSID_ASAPMetadataHandler, 1 },
	{ CLSID_ASAPMetadataHandler, 2 },
	{ CLSID_ASAPMetadataHandler, 3 }
};

class CMyLock
{
	CRITICAL_SECTION &m_lock;

public:

	CMyLock(CRITICAL_SECTION &lck) : m_lock(lck)
	{
		EnterCriticalSection(&lck);
	}

	~CMyLock()
	{
		LeaveCriticalSection(&m_lock);
	}
};

class CASAPMetadataHandler final : IInitializeWithStream, IPropertyStore, IPropertyStoreCapabilities
{
	LONG m_cRef = 1;
	CRITICAL_SECTION m_lock;
	WCHAR m_filename[MAX_PATH];
	bool m_hasInfo = false;
	IStream *m_pstream = nullptr;
	ASAPInfo *m_info = ASAPInfo_New();

	~CASAPMetadataHandler()
	{
		if (m_pstream != nullptr)
			m_pstream->Release();
		DeleteCriticalSection(&m_lock);
		ASAPInfo_Delete(m_info);
		DllRelease();
	}

	HRESULT LoadFile(LPCWSTR wszFile, IStream *pstream, DWORD grfMode)
	{
		m_hasInfo = false;
		if (m_pstream != nullptr) {
			m_pstream->Release();
			m_pstream = nullptr;
		}

		int cbFilename = WideCharToMultiByte(CP_UTF8, 0, wszFile, -1, nullptr, 0, nullptr, nullptr);
		if (cbFilename <= 0)
			return HRESULT_FROM_WIN32(GetLastError());
		char filename[cbFilename];
		if (WideCharToMultiByte(CP_UTF8, 0, wszFile, -1, filename, cbFilename, nullptr, nullptr) <= 0)
			return HRESULT_FROM_WIN32(GetLastError());

		uint8_t module[ASAPInfo_MAX_MODULE_LENGTH];
		int module_len;
		if (pstream != nullptr) {
			HRESULT hr = pstream->Read(module, sizeof(module), reinterpret_cast<ULONG *>(&module_len));
			if (FAILED(hr))
				return hr;
		}
		else {
			HANDLE fh = CreateFileW(wszFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
			if (fh == INVALID_HANDLE_VALUE)
				return HRESULT_FROM_WIN32(GetLastError());
			if (!ReadFile(fh, module, sizeof(module), reinterpret_cast<LPDWORD>(&module_len), nullptr)) {
				HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
				CloseHandle(fh);
				return hr;
			}
			CloseHandle(fh);
		}

		if (!ASAPInfo_Load(m_info, filename, module, module_len))
			return S_OK;

		if ((grfMode & STGM_READWRITE) != 0) {
			const char *ext = strrchr(filename, '.');
			if (ext != nullptr && _stricmp(ext, ".sap") == 0) {
				m_pstream = pstream;
				pstream->AddRef();
			}
		}
		m_hasInfo = true;
		return S_OK;
	}

	static LPSTR AllocString(const char *s, size_t len)
	{
		LPSTR result = static_cast<LPSTR>(CoTaskMemAlloc(len + 1));
		if (result == nullptr)
			return nullptr;
		memcpy(result, s, len);
		result[len] = '\0';
		return result;
	}

	static HRESULT GetString(PROPVARIANT *pvarData, const char *s)
	{
		pvarData->vt = VT_LPSTR;
		pvarData->pszVal = AllocString(s, strlen(s));
		return pvarData->pszVal == NULL ? E_OUTOFMEMORY : S_OK;
	}

	HRESULT GetAuthors(PROPVARIANT *pvarData)
	{
		const char *author = ASAPInfo_GetAuthor(m_info);
		if (author[0] == '\0') {
			pvarData->vt = VT_EMPTY;
			return S_OK;
		}
		// split on " & "
		int i;
		const char *s = author;
		for (i = 1; ; i++) {
			s = strstr(s, " & ");
			if (s == nullptr)
				break;
			s += 3;
		}
		pvarData->vt = VT_VECTOR | VT_LPSTR;
		pvarData->calpstr.cElems = i;
		LPSTR *pElems = static_cast<LPSTR *>(CoTaskMemAlloc(i * sizeof(LPSTR)));
		pvarData->calpstr.pElems = pElems;
		if (pElems == nullptr)
			return E_OUTOFMEMORY;
		s = author;
		for (i = 0; ; i++) {
			const char *e = strstr(s, " & ");
			size_t len = e != nullptr ? e - s : strlen(s);
			pElems[i] = AllocString(s, len);
			if (pElems[i] == nullptr)
				return E_OUTOFMEMORY;
			if (e == nullptr)
				break;
			s = e + 3;
		}
		return S_OK;
	}

	HRESULT GetData(LPCSHCOLUMNID pscid, PROPVARIANT *pvarData)
	{
		if (!m_hasInfo)
			return S_FALSE;

		if (pscid->fmtid == FMTID_SummaryInformation) {
			if (pscid->pid == PIDSI_TITLE)
				return GetString(pvarData, ASAPInfo_GetTitle(m_info));
			if (pscid->pid == PIDSI_AUTHOR)
				return GetAuthors(pvarData);
		}
		else if (pscid->fmtid == FMTID_MUSIC) {
			if (pscid->pid == PIDSI_ARTIST)
				return GetAuthors(pvarData);
			if (pscid->pid == PIDSI_YEAR) {
				int year = ASAPInfo_GetYear(m_info);
				if (year < 0) {
					pvarData->vt = VT_EMPTY;
					return S_OK;
				}
				return InitPropVariantFromUInt32(year, pvarData);
			}
		}
		else if (pscid->fmtid == FMTID_AudioSummaryInformation) {
			if (pscid->pid == PIDASI_TIMELENGTH) {
				int duration = ASAPInfo_GetDuration(m_info, ASAPInfo_GetDefaultSong(m_info));
				if (duration < 0) {
					pvarData->vt = VT_EMPTY;
					return S_OK;
				}
				pvarData->vt = VT_UI8;
				pvarData->uhVal.QuadPart = 10000ULL * duration;
				return S_OK;
			}
			if (pscid->pid == PIDASI_CHANNEL_COUNT)
				return InitPropVariantFromUInt32(ASAPInfo_GetChannels(m_info), pvarData);
		}
		else if (pscid->fmtid == CLSID_ASAPMetadataHandler) {
			if (pscid->pid == 1)
				return InitPropVariantFromUInt32(ASAPInfo_GetSongs(m_info), pvarData);
			if (pscid->pid == 2)
				return GetString(pvarData, ASAPInfo_IsNtsc(m_info) ? "NTSC" : "PAL");
			if (pscid->pid == 3)
				return GetString(pvarData, ASAPInfo_GetDate(m_info));
		}
		return S_FALSE;
	}

	static HRESULT AppendString(char *dest, int *offset, LPCWSTR wszVal)
	{
		int i = *offset;
		while (*wszVal != 0) {
			if (i >= ASAPInfo_MAX_TEXT_LENGTH) {
				dest[i] = '\0';
				return INPLACE_S_TRUNCATED;
			}
			WCHAR c = *wszVal++;
			if (c < ' ' || c > 'z')
				return E_FAIL;
			dest[i++] = static_cast<char>(c);
		}
		dest[i] = '\0';
		*offset = i;
		return S_OK;
	}

	HRESULT SetString(bool (*psetfunc)(ASAPInfo *info, const char *), REFPROPVARIANT propvar)
	{
		char s[ASAPInfo_MAX_TEXT_LENGTH + 1];
		int offset = 0;
		HRESULT hr = S_OK;
		switch (propvar.vt) {
		case VT_EMPTY:
			s[0] = '\0';
			break;
		case VT_UI4:
			if (sprintf(s, "%lu", propvar.ulVal) != 4)
				return E_FAIL;
			break;
		case VT_LPWSTR:
			hr = AppendString(s, &offset, propvar.pwszVal);
			if (FAILED(hr))
				return hr;
			break;
		case VT_VECTOR | VT_LPWSTR:
			s[0] = '\0';
			for (ULONG i = 0; i < propvar.calpwstr.cElems; i++) {
				if (i > 0) {
					hr = AppendString(s, &offset, L" & ");
					if (FAILED(hr))
						return hr;
				}
				hr = AppendString(s, &offset, propvar.calpwstr.pElems[i]);
				if (FAILED(hr))
					return hr;
			}
			break;
		default:
			return E_NOTIMPL;
		}
		return psetfunc(m_info, s) ? hr : E_FAIL;
	}

public:

	CASAPMetadataHandler()
	{
		DllAddRef();
		InitializeCriticalSection(&m_lock);
		m_filename[0] = '\0';
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == nullptr)
			return E_POINTER;
		if (riid == __uuidof(IUnknown) || riid == __uuidof(IInitializeWithStream)) {
			*ppv = static_cast<IInitializeWithStream *>(this);
			AddRef();
			return S_OK;
		}
		if (riid == __uuidof(IPropertyStore)) {
			*ppv = static_cast<IPropertyStore *>(this);
			AddRef();
			return S_OK;
		}
		if (riid == __uuidof(IPropertyStoreCapabilities)) {
			*ppv = static_cast<IPropertyStoreCapabilities *>(this);
			AddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		ULONG r = InterlockedDecrement(&m_cRef);
		if (r == 0)
			delete this;
		return r;
	}

	// IInitializeWithStream

	STDMETHODIMP Initialize(IStream *pstream, DWORD grfMode)
	{
		STATSTG statstg;
		HRESULT hr = pstream->Stat(&statstg, STATFLAG_DEFAULT);
		if (FAILED(hr))
			return hr;
		CMyLock lck(m_lock);
		hr = LoadFile(statstg.pwcsName, pstream, grfMode);
		CoTaskMemFree(statstg.pwcsName);
		return hr;
	}

	// IPropertyStore

	STDMETHODIMP GetCount(DWORD *cProps)
	{
		CMyLock lck(m_lock);
		return m_hasInfo ? ARRAYSIZE(g_scids) : 0;
	}

	STDMETHODIMP GetAt(DWORD iProp, PROPERTYKEY *pkey)
	{
		CMyLock lck(m_lock);
		if (m_hasInfo && iProp >= 0 && iProp < ARRAYSIZE(g_scids)) {
			*pkey = g_scids[iProp];
			return S_OK;
		}
		return E_INVALIDARG;
	}

	STDMETHODIMP GetValue(REFPROPERTYKEY key, PROPVARIANT *pv)
	{
		CMyLock lck(m_lock);
		HRESULT hr = GetData(&key, pv);
		if (hr == S_FALSE) {
			pv->vt = VT_EMPTY;
			return S_OK;
		}
		return hr;
	}

	STDMETHODIMP SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
	{
		CMyLock lck(m_lock);
		if (m_pstream == nullptr)
			return STG_E_ACCESSDENIED;
		if (key.fmtid == FMTID_SummaryInformation) {
			if (key.pid == PIDSI_TITLE)
				return SetString(ASAPInfo_SetTitle, propvar);
			if (key.pid == PIDSI_AUTHOR)
				return SetString(ASAPInfo_SetAuthor, propvar);
		}
		else if (key.fmtid == FMTID_MUSIC) {
			if (key.pid == PIDSI_ARTIST)
				return SetString(ASAPInfo_SetAuthor, propvar);
			if (key.pid == PIDSI_YEAR)
				return SetString(ASAPInfo_SetDate, propvar);
		}
		else if (key.fmtid == CLSID_ASAPMetadataHandler) {
			if (key.pid == 3)
				return SetString(ASAPInfo_SetDate, propvar);
		}
		return E_FAIL;
	}

	STDMETHODIMP Commit()
	{
		CMyLock lck(m_lock);
		if (m_pstream == nullptr)
			return STG_E_ACCESSDENIED;
		LARGE_INTEGER liZero;
		liZero.LowPart = 0;
		liZero.HighPart = 0;
		HRESULT hr = m_pstream->Seek(liZero, STREAM_SEEK_SET, nullptr);
		if (SUCCEEDED(hr)) {
			uint8_t module[ASAPInfo_MAX_MODULE_LENGTH];
			int module_len;
			hr = m_pstream->Read(module, sizeof(module), reinterpret_cast<ULONG *>(&module_len));
			if (SUCCEEDED(hr)) {
				ASAPWriter *writer = ASAPWriter_New();
				const uint8_t *output = ASAPWriter_WriteSap(writer, m_info, module, module_len);
				if (output == nullptr)
					hr = E_FAIL;
				else {
					hr = m_pstream->Seek(liZero, STREAM_SEEK_SET, nullptr);
					if (SUCCEEDED(hr)) {
						int output_len = ASAPWriter_GetOutputLength(writer);
						ULARGE_INTEGER liSize;
						liSize.LowPart = output_len;
						liSize.HighPart = 0;
						hr = m_pstream->SetSize(liSize);
						if (SUCCEEDED(hr)) {
							hr = m_pstream->Write(output, output_len, nullptr);
							if (SUCCEEDED(hr))
								hr = m_pstream->Commit(STGC_DEFAULT);
						}
					}
				}
				ASAPWriter_Delete(writer);
			}
		}
		m_pstream->Release();
		m_pstream = nullptr;
		return hr;
	}

	// IPropertyStoreCapabilities

	STDMETHODIMP IsPropertyWritable(REFPROPERTYKEY key)
	{
		if (key.fmtid == FMTID_SummaryInformation) {
			if (key.pid == PIDSI_TITLE || key.pid == PIDSI_AUTHOR)
				return S_OK;
		}
		else if (key.fmtid == FMTID_MUSIC) {
			if (key.pid == PIDSI_ARTIST || key.pid == PIDSI_YEAR)
				return S_OK;
		}
		else if (key.fmtid == CLSID_ASAPMetadataHandler) {
			if (key.pid == 3)
				return S_OK;
		}
		return S_FALSE;
	}
};

class CASAPMetadataHandlerFactory : public IClassFactory
{
public:

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == nullptr)
			return E_POINTER;
		if (riid == __uuidof(IUnknown) || riid == __uuidof(IClassFactory)) {
			*ppv = static_cast<IClassFactory *>(this);
			DllAddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		DllAddRef();
		return 2;
	}

	STDMETHODIMP_(ULONG) Release()
	{
		DllRelease();
		return 1;
	}

	STDMETHODIMP CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppv)
	{
		*ppv = nullptr;
		if (punkOuter != nullptr)
			return CLASS_E_NOAGGREGATION;
		CASAPMetadataHandler *punk = new CASAPMetadataHandler;
		if (punk == nullptr)
			return E_OUTOFMEMORY;
		HRESULT hr = punk->QueryInterface(riid, ppv);
		punk->Release();
		return hr;
	}

	STDMETHODIMP LockServer(BOOL fLock)
	{
		if (fLock)
			DllAddRef();
		else
			DllRelease();
		return S_OK;
	};
};

STDAPI_(BOOL) __declspec(dllexport) DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		g_hDll = hInstance;
	return TRUE;
}

static HRESULT GetPropdescFilename(LPWSTR szSchemaPath)
{
	if (!GetModuleFileNameW(g_hDll, szSchemaPath, MAX_PATH)
	 || !PathRemoveFileSpecW(szSchemaPath)
	 || !PathAppendW(szSchemaPath, L"ASAPShellEx.propdesc"))
		return E_FAIL;
	return CoInitialize(nullptr);
}

STDAPI __declspec(dllexport) InstallPropertySchema()
{
	WCHAR szSchemaPath[MAX_PATH];
	HRESULT hr = GetPropdescFilename(szSchemaPath);
	if (FAILED(hr))
		return hr;
	hr = PSRegisterPropertySchema(szSchemaPath);
	CoUninitialize();
	if (hr == INPLACE_S_TRUNCATED) // returned on Windows 10/11, no idea why
		hr = S_OK;
	if (SUCCEEDED(hr))
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return hr;
}

STDAPI __declspec(dllexport) UninstallPropertySchema()
{
	WCHAR szSchemaPath[MAX_PATH];
	HRESULT hr = GetPropdescFilename(szSchemaPath);
	if (FAILED(hr))
		return hr;
	hr = PSUnregisterPropertySchema(szSchemaPath);
	CoUninitialize();
	return hr;
}

static LSTATUS RegSetString(HKEY hKey, LPCSTR lpValueName, LPCSTR lpStr)
{
	return RegSetValueEx(hKey, lpValueName, 0, REG_SZ, reinterpret_cast<const BYTE *>(lpStr), strlen(lpStr) + 1);
}

STDAPI __declspec(dllexport) DllRegisterServer()
{
	HKEY hk1;
	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "CLSID\\" CLSID_ASAPMetadataHandler_str, 0, nullptr, 0, KEY_WRITE, nullptr, &hk1, nullptr) != ERROR_SUCCESS)
		return E_FAIL;
	HKEY hk2;
	if (RegCreateKeyEx(hk1, "InProcServer32", 0, nullptr, 0, KEY_WRITE, nullptr, &hk2, nullptr) != ERROR_SUCCESS) {
		RegCloseKey(hk1);
		return E_FAIL;
	}
	char szModulePath[MAX_PATH];
	if (GetModuleFileName(g_hDll, szModulePath, MAX_PATH) == 0
	 || RegSetString(hk2, nullptr, szModulePath) != ERROR_SUCCESS
	 || RegSetString(hk2, "ThreadingModel", "Both") != ERROR_SUCCESS) {
		RegCloseKey(hk2);
		RegCloseKey(hk1);
		return E_FAIL;
	}
	RegCloseKey(hk2);
	RegCloseKey(hk1);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers", 0, KEY_WRITE, &hk1) != ERROR_SUCCESS)
		return E_FAIL;
	for (LPCSTR ext : extensions) {
		if (RegCreateKeyEx(hk1, ext, 0, nullptr, 0, KEY_WRITE, nullptr, &hk2, nullptr) != ERROR_SUCCESS) {
			RegCloseKey(hk1);
			return E_FAIL;
		}
		if (RegSetString(hk2, nullptr, CLSID_ASAPMetadataHandler_str) != ERROR_SUCCESS) {
			RegCloseKey(hk2);
			RegCloseKey(hk1);
			return E_FAIL;
		}
		RegCloseKey(hk2);
	}
	RegCloseKey(hk1);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 0, KEY_SET_VALUE, &hk1) != ERROR_SUCCESS)
		return E_FAIL;
	if (RegSetString(hk1, CLSID_ASAPMetadataHandler_str, "ASAP Metadata Handler") != ERROR_SUCCESS) {
		RegCloseKey(hk1);
		return E_FAIL;
	}
	RegCloseKey(hk1);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\KindMap", 0, KEY_SET_VALUE, &hk1) != ERROR_SUCCESS)
		return E_FAIL;
	for (LPCSTR ext : extensions) {
		if (RegSetString(hk1, ext, "music") != ERROR_SUCCESS) {
			RegCloseKey(hk1);
			return E_FAIL;
		}
	}
	RegCloseKey(hk1);

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, "SystemFileAssociations", 0, KEY_WRITE, &hk1) != ERROR_SUCCESS)
		return E_FAIL;
	for (LPCSTR ext : extensions) {
		if (RegCreateKeyEx(hk1, ext, 0, nullptr, 0, KEY_WRITE, nullptr, &hk2, nullptr) != ERROR_SUCCESS) {
			RegCloseKey(hk1);
			return E_FAIL;
		}
		LPCSTR fullDetails;
#define FULLDETAILS_COMMON "System.PropGroup.Audio;System.Media.Duration;System.Audio.ChannelCount;ASAP.Subsongs;ASAP.PALNTSC;" \
	"System.PropGroup.FileSystem;System.ItemNameDisplay;System.ItemType;System.ItemFolderPathDisplay;System.Size;System.DateCreated;System.DateModified;System.FileAttributes"
		switch (ext[1]) {
		case 's': // sap
			fullDetails = "prop:System.PropGroup.Description;System.Author;System.Title;ASAP.Date;" FULLDETAILS_COMMON;
			break;
		case 'r': // rmt
		case 't': // tmc, tm8, tm2
			fullDetails = "prop:System.PropGroup.Description;System.Title;" FULLDETAILS_COMMON;
			break;
		default:
			fullDetails = "prop:" FULLDETAILS_COMMON;
			break;
		}
		if (RegSetString(hk2, "InfoTip", "prop:System.ItemType;System.Size;System.Media.Duration") != ERROR_SUCCESS
		 || RegSetString(hk2, "ExtendedTileInfo", "prop:System.ItemType;System.Size;System.Media.Duration") != ERROR_SUCCESS
		 || RegSetString(hk2, "PreviewDetails", "prop:*System.Title;*System.Media.Duration;*System.Size;*System.Author;*ASAP.Date") != ERROR_SUCCESS
		 || RegSetString(hk2, "FullDetails", fullDetails) != ERROR_SUCCESS
		 || RegSetString(hk2, "PerceivedType", "audio") != ERROR_SUCCESS)
		RegCloseKey(hk2);
	}
	RegCloseKey(hk1);
	return InstallPropertySchema();
}

STDAPI __declspec(dllexport) DllUnregisterServer()
{
	UninstallPropertySchema();

	HKEY hk1;
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, "SystemFileAssociations", 0, DELETE, &hk1) == ERROR_SUCCESS) {
		for (LPCSTR ext : extensions)
			RegDeleteKey(hk1, ext);
		RegCloseKey(hk1);
	}
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\KindMap", 0, KEY_SET_VALUE, &hk1) == ERROR_SUCCESS) {
		for (LPCSTR ext : extensions)
			RegDeleteValue(hk1, ext);
		RegCloseKey(hk1);
	}
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 0, KEY_SET_VALUE, &hk1) == ERROR_SUCCESS) {
		RegDeleteValue(hk1, CLSID_ASAPMetadataHandler_str);
		RegCloseKey(hk1);
	}
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers", 0, DELETE, &hk1) == ERROR_SUCCESS) {
		for (LPCSTR ext : extensions)
			RegDeleteKey(hk1, ext);
		RegCloseKey(hk1);
	}
	RegDeleteKey(HKEY_CLASSES_ROOT, "CLSID\\" CLSID_ASAPMetadataHandler_str "\\InProcServer32");
	RegDeleteKey(HKEY_CLASSES_ROOT, "CLSID\\" CLSID_ASAPMetadataHandler_str);
	return S_OK;
}

STDAPI __declspec(dllexport) DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
	if (ppv == nullptr)
		return E_INVALIDARG;
	if (rclsid == CLSID_ASAPMetadataHandler) {
		static CASAPMetadataHandlerFactory g_ClassFactory;
		return g_ClassFactory.QueryInterface(riid, ppv);
	}
	*ppv = nullptr;
	return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI __declspec(dllexport) DllCanUnloadNow()
{
	return g_cRef == 0 ? S_OK : S_FALSE;
}
