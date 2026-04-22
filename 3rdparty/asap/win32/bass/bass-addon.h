/*
	BASS 2.4 add-on C/C++ header file
	Copyright (c) 2003-2025 Un4seen Developments Ltd.
*/

#ifndef BASS_ADDON_H
#define BASS_ADDON_H

#include "bass.h"

#ifdef __ANDROID__
#include <jni.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASSFILE_DEFINED
typedef void *BASSFILE;
#define BASSFILE_DEFINED
#endif

typedef struct {
	DWORD flags; // ADDON_xxx flags
	void (WINAPI *Free)(void *inst);
	QWORD (WINAPI *GetLength)(void *inst, DWORD mode);
	const char *(WINAPI *GetTags)(void *inst, DWORD tags); // optional
	QWORD (WINAPI *GetFilePosition)(void *inst, DWORD mode); // optional
	void (WINAPI *GetInfo)(void *inst, BASS_CHANNELINFO *info);
	BOOL (WINAPI *CanSetPosition)(void *inst, QWORD pos, DWORD mode);
	QWORD (WINAPI *SetPosition)(void *inst, QWORD pos, DWORD mode);
	QWORD (WINAPI *GetPosition)(void *inst, QWORD pos, DWORD mode); // optional
	HSYNC (WINAPI *SetSync)(void *inst, DWORD type, QWORD param, SYNCPROC *proc, void *user); // optional
	void (WINAPI *RemoveSync)(void *inst, HSYNC sync); // optional
	BOOL (WINAPI *CanResume)(void *inst); // optional
	DWORD (WINAPI *SetFlags)(void *inst, DWORD flags); // optional
	BOOL (WINAPI *Attribute)(void *inst, DWORD attrib, float *value, BOOL set); // optional
	DWORD (WINAPI *AttributeEx)(void *inst, DWORD attrib, void *value, DWORD typesize, BOOL set); // optional
} ADDON_FUNCTIONS;

#define ADDON_OWNPOS		1 // handles all position tracking (including POS/END syncs)
#define ADDON_DECODETO		2 // supports BASS_POS_DECODETO seeking
#define ADDON_ATTIBUTEX		4 // supports AttributeEx with size param
#define ADDON_LOCK			8 // create stream in locked state (2.4.16)
#define ADDON_ATTIBUTEXTYPE	16 // supports AttributeEx with typesize param (2.4.18)

typedef struct {
	void (WINAPI *Free)(void *inst);
#ifdef __ANDROID__
	BOOL (WINAPI *SetParameters)(void *inst, const void *param, JNIEnv *env);
	BOOL (WINAPI *GetParameters)(void *inst, void *param, JNIEnv *env);
#else
	BOOL (WINAPI *SetParameters)(void *inst, const void *param);
	BOOL (WINAPI *GetParameters)(void *inst, void *param);
#endif
	BOOL (WINAPI *Reset)(void *inst);
	// only if BASS_FX_EX is set
	BOOL (WINAPI *Bypass)(void *inst); // skip this FX?
} ADDON_FUNCTIONS_FX;

typedef HSTREAM (CALLBACK STREAMCREATEPROC)(BASSFILE file, DWORD flags); // BASS_StreamCreateFile/User/URL plugin function
typedef HSTREAM (CALLBACK STREAMCREATEURLPROC)(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user); // BASS_StreamCreateURL plugin function (unsupported URI scheme)
typedef BOOL (CALLBACK BASSCONFIGPROC)(DWORD option, DWORD flags, void *value); // config plugin function
typedef HFX (CALLBACK BASSFXPROC)(DWORD chan, DWORD type, int priority); // FX plugin function

// BASSCONFIGPROC flags
#define BASSCONFIG_SET	1 // set the config (otherwise get it)
#define BASSCONFIG_PTR	2 // value is a pointer

// RegisterPlugin modes
#define PLUGIN_CONFIG_ADD		0 // add a config plugin
#define PLUGIN_CONFIG_REMOVE	1 // remove a config plugin
#define PLUGIN_FX_ADD			2 // add an FX plugin
#define PLUGIN_FX_REMOVE		3 // remove an FX plugin

typedef struct {
	void (WINAPI *SetError)(int error); // set error code
	void (WINAPI *RegisterPlugin)(void *proc, DWORD mode); // add/remove a plugin function
	HSTREAM (WINAPI *CreateStream)(DWORD freq, DWORD chans, DWORD flags, STREAMPROC *proc, void *inst, const ADDON_FUNCTIONS *funcs); // create a stream
	HFX (WINAPI *SetFX)(DWORD handle, DSPPROC *proc, void *inst, int priority, const ADDON_FUNCTIONS_FX *funcs); // set DSP/FX on a channel
	void *(WINAPI *GetInst)(HSTREAM handle, const ADDON_FUNCTIONS *funcs); // get stream instance data
	void *reserved1;
	HSYNC (WINAPI *NewSync)(HSTREAM handle, DWORD type, SYNCPROC *proc, void *user); // add a sync to a stream
	union {
		BOOL (WINAPI *TriggerSync)(HSTREAM handle, HSYNC sync, QWORD pos, DWORD data); // trigger a sync
		BOOL (WINAPI *TriggerSyncs)(HSTREAM handle, DWORD type, QWORD pos, DWORD data); // trigger all syncs of a type (2.4.17)
	};
	QWORD (WINAPI *GetCount)(DWORD handle, BOOL output); // get raw count (output=0) or current output position (output=1)
	QWORD (WINAPI *GetPosition)(DWORD handle, QWORD count, DWORD mode); // get raw "count" translated to source position

	struct { // file functions
		BASSFILE (WINAPI *Open)(DWORD filetype, const void *file, QWORD offset, QWORD length, DWORD flags, DWORD exflags); // open a file
		BASSFILE (WINAPI *OpenURL)(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user, DWORD exflags); // open a URL
		BASSFILE (WINAPI *OpenUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *proc, void *user, DWORD exflags); // open a custom file
		void (WINAPI *Close)(BASSFILE file); // close an opened file
		const char *(WINAPI *GetFileName)(BASSFILE file, BOOL *unicode); // get the filename/url
		BOOL (WINAPI *SetStream)(BASSFILE file, HSTREAM handle); // set stream handle (for auto closing & META/DOWNLOAD syncs)
		DWORD (WINAPI *GetFlags)(BASSFILE file); // get BASSFILE_xxx flags
		void (WINAPI *SetFlags)(BASSFILE file, DWORD flags); // set BASSFILE_xxx flags (BASSFILE_RESTRATE/NOLIMIT/NOWAIT/NOBUF/RECONNECT only)
		DWORD (WINAPI *Read)(BASSFILE file, void *buf, DWORD len); // read from file
		BOOL (WINAPI *Seek)(BASSFILE file, QWORD pos); // seek in file
		QWORD (WINAPI *GetPos)(BASSFILE file, DWORD mode); // get file position (mode=BASS_FILEPOS_xxx)
		BOOL (WINAPI *Eof)(BASSFILE file); // End of file?
		const char *(WINAPI *GetTags)(BASSFILE file, DWORD tags); // get tags
		BOOL (WINAPI *StartThread)(BASSFILE file, DWORD bitrate, DWORD offset); // start download thread (bitrate in bytes/sec)
		BOOL (WINAPI *CanResume)(BASSFILE file); // enough data buffered to resume?
	} file;

	struct { // sample data processing functions (len=samples, dst=src is ok)
		void (WINAPI *Float2Int)(const float *src, void *dst, DWORD len, DWORD res); // convert floating-point data to 8/16/24/32-bit (res=1/2/3/4)
		void (WINAPI *Int2Float)(const void *src, float *dst, DWORD len, DWORD res); // convert 8/16/24/32-bit (res=1/2/3/4) to floating-point
		void (WINAPI *Swap)(const void *src, void *dst, DWORD len, DWORD res); // swap byte order of 16/32/64-bit (res=2/4/8) data
	} data;
} BASS_FUNCTIONS;

// 2.4.18 functions
typedef struct {
	const struct BASS_FUNCTIONS_MISC *misc;
	const struct BASS_FUNCTIONS_FILE *file;
	const struct BASS_FUNCTIONS_DATA *data;
	const struct BASS_FUNCTIONS_JNI *jni;
} BASS_FUNCTIONS2;

struct BASS_FUNCTIONS_MISC {
	void (WINAPI *SetError)(int error); // set error code
	void (WINAPI *RegisterPlugin)(void *proc, DWORD mode); // add/remove a plugin function
	HSTREAM (WINAPI *CreateStream)(DWORD freq, DWORD chans, DWORD flags, STREAMPROC *proc, void *inst, const ADDON_FUNCTIONS *funcs); // create a stream
	HFX (WINAPI *SetFX)(DWORD handle, DSPPROC *proc, void *inst, int priority, DWORD flags, const ADDON_FUNCTIONS_FX *funcs); // set DSP/FX on a channel
	void *(WINAPI *GetInst)(HSTREAM handle, const ADDON_FUNCTIONS *funcs); // get stream instance data & inc REF2 (call LockRef(DECREF2) when done with it)
	HSYNC (WINAPI *NewSync)(HSTREAM handle, DWORD type, SYNCPROC *proc, void *user); // add a sync to a stream
	union {
		BOOL (WINAPI *TriggerSync)(HSTREAM handle, HSYNC sync, QWORD pos, DWORD data); // trigger a sync
		BOOL (WINAPI *TriggerSyncs)(HSTREAM handle, DWORD type, QWORD pos, DWORD data); // trigger all syncs of a type
	};
	QWORD (WINAPI *GetCount)(DWORD handle, BOOL output); // get raw count (output=0) or current output position (output=1)
	QWORD (WINAPI *GetPosition)(DWORD handle, QWORD count, DWORD mode); // get raw "count" translated to source position
	BOOL (WINAPI *LockRef)(DWORD handle, DWORD mode); // locking & reference counting
};

struct BASS_FUNCTIONS_FILE {
	BASSFILE (WINAPI *Open)(DWORD filetype, const void *file, QWORD offset, QWORD length, DWORD flags, DWORD exflags); // open a file
	BASSFILE (WINAPI *OpenURL)(const char *url, const char *heads, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user, DWORD exflags); // open a URL
	BASSFILE (WINAPI *OpenUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *proc, void *user, DWORD exflags); // open a custom file
	BOOL (WINAPI *Cancel)(void *user); // cancel OpenURL
	void (WINAPI *Close)(BASSFILE file); // close & free a file
	void (WINAPI *CloseNoFree)(BASSFILE file); // close a file but don't free its info
	const char *(WINAPI *GetFileName)(BASSFILE file, BOOL *unicode); // get the filename/url
	BOOL (WINAPI *SetStream)(BASSFILE file, HSTREAM handle); // set stream handle (for auto closing & META/DOWNLOAD syncs)
	DWORD (WINAPI *GetFlags)(BASSFILE file); // get BASSFILE_xxx flags
	void (WINAPI *SetFlags)(BASSFILE file, DWORD flags); // set BASSFILE_xxx flags (BASSFILE_RESTRATE/NOLIMIT/NOWAIT/NOBUF/RECONNECT only)
	DWORD (WINAPI *Read)(BASSFILE file, void *buf, DWORD len); // read from file
	BOOL (WINAPI *Seek)(BASSFILE file, QWORD pos); // seek in file
	BOOL (WINAPI *SetPos)(BASSFILE file, DWORD mode, QWORD pos); // set file position (mode=BASS_FILEPOS_CURRENT/START/END)
	QWORD (WINAPI *GetPos)(BASSFILE file, DWORD mode); // get file position (mode=BASS_FILEPOS_xxx)
	BOOL (WINAPI *Eof)(BASSFILE file); // End of file?
	const char *(WINAPI *GetTags)(BASSFILE file, DWORD tags); // get tags
	BOOL (WINAPI *StartThread)(BASSFILE file, DWORD bitrate, DWORD minbuf, DWORD backbuf); // start download thread (bitrate in bytes/sec)
	BOOL (WINAPI *CanResume)(BASSFILE file); // enough data buffered to resume?
};

struct BASS_FUNCTIONS_DATA {
	void (WINAPI *Float2Int)(const float *src, void *dst, DWORD len, DWORD res); // convert floating-point data to 8/16/24/32-bit (res=1/2/3/4)
	void (WINAPI *Int2Float)(const void *src, float *dst, DWORD len, DWORD res); // convert 8/16/24/32-bit (res=1/2/3/4) to floating-point
	void (WINAPI *Swap)(const void *src, void *dst, DWORD len, DWORD res); // swap byte order of 16/32/64-bit (res=2/4/8) data
};

// additional SetFX flags
#define BASS_FX_EX			0x100 // have extended FX functions

// LockRef modes
#define BASS_UNLOCK			0	// BASS_ChanneLock(false)
#define BASS_LOCK			1	// BASS_ChanneLock(true)
#define BASS_DECREF			2	// BASS_ChanneRef(false)
#define BASS_INCREF			3	// BASS_ChanneRef(true)
#define BASS_DECREF2		4	// decrement non-async freeing reference count
#define BASS_INCREF2		5	// increment non-async freeing reference count

// file "flags"
#define BASSFILE_BUFFERED	1	// net/buffered file
#define BASSFILE_NOLIMIT	2	// enable reading beyond end of audio data (into tags)
#define BASSFILE_NOWAIT		0x10	// don't wait to read full amount (net/buffered file)
#define BASSFILE_NOBUF		0x20	// don't buffer file
#define BASSFILE_PUSH		0x40	// STREAMFILE_BUFFERPUSH file (2.4.13)
#define BASSFILE_RECONNECT	0x80	// reconnect if download stops short (2.4.17)
#define BASSFILE_BLOCK		BASS_STREAM_BLOCK
#define BASSFILE_RESTRATE	BASS_STREAM_RESTRATE
#define BASSFILE_ASYNCFILE	BASS_ASYNCFILE
#define BASSFILE_UNICODE	BASS_UNICODE

// file "exflags"
#define BASSFILE_EX_TAGS		1	// read tags
#define BASSFILE_EX_MMAP		4	// memory-map file
#define BASSFILE_EX_IGNORESTAT	8	// ignore HTTP status
#define BASSFILE_EX_KEEPALIVE	16	// HTTP/1.1 keep-alive

// additional GetPos mode
#define BASS_FILEPOS_FD			11	// file descriptor

// additional GetTags tags
#define BASS_TAG_HTTP_REQUEST	15	// custom HTTP request headers
#define BASS_TAG_MEMORY			0xffffffff // file memory address
#define BASS_TAG_DOWNLOADPROC	0x80000001 // pointer to DOWNLOADPROC & user parameter

const void *WINAPI BASSplugin(DWORD face); // plugin interface function

// BASSplugin "faces"
#define BASSPLUGIN_INFO			0 // BASS_PLUGININFO
#define BASSPLUGIN_CREATE		1 // STREAMCREATEPROC
#define BASSPLUGIN_CREATEURL	2 // STREAMCREATEURLPROC
#define BASSPLUGIN_CREATEURL2	3 // STREAMCREATEURLPROC with custom headers support

#define BASS_FREQ_INIT		0x80000001 // BASS_Init freq
#define BASS_FREQ_CURRENT	0x80000002 // device's current rate

#define BASS_SYNC_EX 0x10000000
typedef void (CALLBACK SYNCPROCEX)(HSYNC handle, DWORD channel, DWORD data, void *user, QWORD pos);

#define BASS_POS_RESTART	0x80000000 // restarting

#define BASS_CONFIG_ADDON		0x8000 // get BASS_FUNCTIONS
#define BASS_CONFIG_INUPDATE	0x8001 // check if in update thread
#define BASS_CONFIG_ADDON_JNI	0x8002 // get BASSJNI_FUNCTIONS
#define BASS_CONFIG_CANCEL		0x8003 // check if device is being freed
#define BASS_CONFIG_ADDON2		0x8004 // get BASS_FUNCTIONS2 (2.4.18)

// BASS_CONFIG_INUPDATE values
#define INUPDATE_THREAD		1 // in update thread or BASS_Update
#define INUPDATE_CHANNEL	2 // in BASS_ChannelUpdate or BASS_ChannelPlay
#define INUPDATE_DEVICE		3 // in device thread
#define INUPDATE_OTHER		4 // other update thread

#ifndef NOBASSFUNCMACROS
extern const BASS_FUNCTIONS *bassfunc;
#define GetBassFunc() (bassfunc=(const BASS_FUNCTIONS*)BASS_GetConfigPtr(BASS_CONFIG_ADDON))
extern const BASS_FUNCTIONS2 *bassfunc2;
#define GetBassFunc2() (bassfunc2=(const BASS_FUNCTIONS2*)BASS_GetConfigPtr(BASS_CONFIG_ADDON2))

// SetError macros
#define noerror() return (bassfunc->SetError(BASS_OK),TRUE)
#define noerrorn(n) return (bassfunc->SetError(BASS_OK),n)
#define error(n) return (bassfunc->SetError(n),FALSE) // error = 0
#define errorn(n) return (bassfunc->SetError(n),-1) // error = -1
#define errorp(n,t) return (bassfunc->SetError(n),(t)NULL) // error = NULL
#endif

#ifdef __ANDROID__
typedef struct {
	jobject object;
	jobject user;
	jmethodID method;
} JCALLBACKSTUFF;

typedef struct BASS_FUNCTIONS_JNI {
	JNIEnv *(*GetEnv)();
	jstring (*NewString)(JNIEnv *env, const char *s);
	void *(*GetByteBuffer)(JNIEnv *env, jobject buffer, jbyteArray *barray);
	DWORD (*SetBufferFreeSync)(JNIEnv *env, DWORD handle, jobject buffer, jbyteArray barray, void *mem);
	// callback stuff
	struct {
		void *(*NewDownloadProc)(JNIEnv *env, jobject proc, jobject user, DOWNLOADPROC **nproc);
		void *(*NewFileProcs)(JNIEnv *env, jobject procs, jobject user, const BASS_FILEPROCS **nprocs);
		void (*Free)(void *callback);
		DWORD (*SetFreeSync)(JNIEnv *env, DWORD handle, void *callback);
		void *(*New)(JNIEnv *env, jobject proc, jobject user, jmethodID method);
	} callback;
} BASSJNI_FUNCTIONS;

#ifndef NOBASSFUNCMACROS
extern const BASSJNI_FUNCTIONS *jnifunc;
#define GetJniFunc() (jnifunc=(BASSJNI_FUNCTIONS*)BASS_GetConfigPtr(BASS_CONFIG_ADDON_JNI))
#endif

#define BASS_ERROR_JAVA_CLASS 500 // invalid Java class

#define BASS_FILE_JAVA 0x11111111 // file is a Java object

// additional GetTags flags
#define BASS_TAG_BYTEBUFFER	0x10000000 // return tags in a ByteBuffer
#define BASS_TAG_JAVA		0x80000000 // return tags in a Java object
#endif

#ifdef __cplusplus
}
#endif

#endif
