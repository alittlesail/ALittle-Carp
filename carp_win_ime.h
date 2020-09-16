#ifndef CARP_WIN_IME_INCLUDED
#define CARP_WIN_IME_INCLUDED (1)
#include <malloc.h>
#include <tchar.h>

#ifdef _WIN32

#define STBDS_NO_SHORT_NAMES
#define STB_DS_IMPLEMENTATION
#include "stb/stb_ds.h"

#include <windows.h>
#include <imm.h>
#include <objbase.h>
#include <dimm.h>
#include <msctf.h>

#include <stdint.h>
#include <stdbool.h>

#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "Version.lib")

#define CHT_IMEFILENAME1    "TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2    "CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3    "MSTCIPHA.IME" // Phonetic 5.1
#define CHS_IMEFILENAME1    "PINTLGNT.IME" // MSPY1.5/2/3
#define CHS_IMEFILENAME2    "MSSCIPYA.IME" // MSPY3 for OfficeXP

#define LANG_CHT            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define _CHT_HKL            ( (HKL)(INT_PTR)0xE0080404 ) // New Phonetic
#define _CHT_HKL2           ( (HKL)(INT_PTR)0xE0090404 ) // New Chang Jie
#define _CHS_HKL            ( (HKL)(INT_PTR)0xE00E0804 ) // MSPY
#define MAKEIMEVERSION( major, minor ) \
    ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )

#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )	// New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )	// New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )	// New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )	// New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )	// New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )	// New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )	// New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHS_VER41	( LANG_CHS | MAKEIMEVERSION( 4, 1 ) )	// MSPY1.5	// SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42	( LANG_CHS | MAKEIMEVERSION( 4, 2 ) )	// MSPY2	// Win2k/WinME
#define IMEID_CHS_VER53	( LANG_CHS | MAKEIMEVERSION( 5, 3 ) )	// MSPY3	// WinXP

enum { INDICATOR_NON_IME, INDICATOR_CHS, INDICATOR_CHT, INDICATOR_KOREAN, INDICATOR_JAPANESE };
enum { IMEUI_STATE_OFF, IMEUI_STATE_ON, IMEUI_STATE_ENGLISH };

#define LCID_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

wchar_t		INDICATOR[5][3] =
{
	L"En",
	L"\x7B80",
	L"\x7E41",
	L"\xAC00",
	L"\x3042",
};

typedef void (*EditingCallback)();

typedef struct
{
	bool co_init;
	HWND hWnd;
	
	HINSTANCE hImm32;
	INPUTCONTEXT* (WINAPI* ImmLockIMC)(HIMC);
	BOOL(WINAPI* ImmUnlockIMC)(HIMC);
	LPVOID(WINAPI* ImmLockIMCC)(HIMCC);
	BOOL(WINAPI* ImmUnlockIMCC)(HIMCC);

	wchar_t*		input;
	wchar_t*		comp;
	wchar_t*		reading;
	BYTE*			candidate;

	int				ulStart;
	int				ulEnd;

	HKL				hkl;
	LANGID			langId;
	WORD			codePage;

	bool			bUnicodeIME;
	bool			bVerticalReading;
	bool			bVerticalCandidate;
	int				ImeState;
	wchar_t*		wszCurrIndicator;

	DWORD			dwId[2];

	HINSTANCE		hDllIme;
	ITfThreadMgr*	ptim;
	UINT(WINAPI* GetReadingString)(HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT);
	BOOL(WINAPI* ShowReadingWindow)(HIMC, BOOL);

	EditingCallback editing_callback;
} _carp_win_ime_data_t;

static _carp_win_ime_data_t _carp_win_ime_data = {0};

static void carp_win_ime_shutdown()
{
	if (_carp_win_ime_data.hImm32 != NULL)
	{
		FreeLibrary(_carp_win_ime_data.hImm32);
		_carp_win_ime_data.hImm32 = NULL;
	}
	_carp_win_ime_data.ImmLockIMC = NULL;
	_carp_win_ime_data.ImmUnlockIMC = NULL;
	_carp_win_ime_data.ImmLockIMCC = NULL;
	_carp_win_ime_data.ImmUnlockIMCC = NULL;

	if (_carp_win_ime_data.ptim) {
		struct ITfDocumentMgr* document_mgr = 0;
		if (SUCCEEDED(_carp_win_ime_data.ptim->lpVtbl->AssociateFocus(_carp_win_ime_data.ptim, _carp_win_ime_data.hWnd, NULL, &document_mgr))) {
			if (document_mgr)
				document_mgr->lpVtbl->Release(document_mgr);
		}
	}
	
	if (_carp_win_ime_data.hDllIme != NULL)
	{
		FreeLibrary(_carp_win_ime_data.hDllIme);
		_carp_win_ime_data.hDllIme = NULL;
	}

	if (_carp_win_ime_data.ptim != NULL)
	{
		_carp_win_ime_data.ptim->lpVtbl->Release(_carp_win_ime_data.ptim);
		_carp_win_ime_data.ptim = NULL;
	}

	if (_carp_win_ime_data.co_init) {
		CoUninitialize();
		_carp_win_ime_data.co_init = false;
	}

	stbds_arrfree(_carp_win_ime_data.input);
	stbds_arrfree(_carp_win_ime_data.comp);
	stbds_arrfree(_carp_win_ime_data.reading);
	stbds_arrfree(_carp_win_ime_data.candidate);

	memset(&_carp_win_ime_data, 0, sizeof(_carp_win_ime_data));
}

extern void _carp_win_ime_on_input_language_change(HWND hWnd, WPARAM wParam, LPARAM lParam);

static void carp_win_ime_setup(HWND hWnd, EditingCallback editing_callback)
{
	carp_win_ime_shutdown();

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (!SUCCEEDED(hr))
	{
		carp_win_ime_shutdown();
		return;
	}
	_carp_win_ime_data.co_init = true;
	_carp_win_ime_data.hWnd = hWnd;

	CoCreateInstance(&CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, &IID_ITfThreadMgr, (void**)&_carp_win_ime_data.ptim);
	if (_carp_win_ime_data.ptim == NULL)
	{
		carp_win_ime_shutdown();
		return;
	}
	
	_carp_win_ime_data.hImm32 = LoadLibrary("imm32.dll");
	if (_carp_win_ime_data.hImm32 == NULL)
	{
		carp_win_ime_shutdown();
		return;
	}
	_carp_win_ime_data.ImmLockIMC = (INPUTCONTEXT * (WINAPI*)(HIMC))GetProcAddress(_carp_win_ime_data.hImm32, "ImmLockIMC");
	_carp_win_ime_data.ImmUnlockIMC = (BOOL(WINAPI*)(HIMC))GetProcAddress(_carp_win_ime_data.hImm32, "ImmUnlockIMC");
	_carp_win_ime_data.ImmLockIMCC = (LPVOID(WINAPI*)(HIMCC))GetProcAddress(_carp_win_ime_data.hImm32, "ImmLockIMCC");
	_carp_win_ime_data.ImmUnlockIMCC = (BOOL(WINAPI*)(HIMCC)) GetProcAddress(_carp_win_ime_data.hImm32, "ImmUnlockIMCC");

	_carp_win_ime_data.ImeState = IMEUI_STATE_OFF;

	_carp_win_ime_data.editing_callback = editing_callback;

	_carp_win_ime_on_input_language_change(hWnd, 0, GetKeyboardLayout(0));
}


/*----------------------------------------------------------------------------*/
static int _carp_win_ime_wchar_to_char(UINT codePage, const wchar_t* wText, int wLen, char* text, int len)
{
	if (text == 0) return WideCharToMultiByte(codePage, 0, wText, wLen, NULL, 0, NULL, NULL);
	int tLen = WideCharToMultiByte(codePage, 0, wText, wLen, NULL, 0, NULL, NULL);
	if (tLen > len) return 0;
	return WideCharToMultiByte(codePage, 0, wText, wLen, text, tLen, NULL, NULL);
}

/*----------------------------------------------------------------------------*/
static int _carp_win_ime_char_to_wchar(UINT codePage, const char* text, int len, wchar_t* wText, int wLen)
{
	if (text == 0) return MultiByteToWideChar(codePage, 0, NULL, 0, wText, wLen);
	int tLen = MultiByteToWideChar(codePage, 0, NULL, 0, wText, wLen);
	if (tLen > len) return 0;
	return MultiByteToWideChar(codePage, 0, text, tLen, wText, wLen);
}

/*----------------------------------------------------------------------------*/
static void	_carp_win_ime_get_imeid()
{
	_carp_win_ime_data.dwId[0] = _carp_win_ime_data.dwId[1] = 0;

	if (!((_carp_win_ime_data.hkl == _CHT_HKL) || (_carp_win_ime_data.hkl == _CHT_HKL2) || (_carp_win_ime_data.hkl == _CHS_HKL)))
		return;

	char imeFileName[256];
	if (!ImmGetIMEFileNameA(_carp_win_ime_data.hkl, imeFileName, (sizeof(imeFileName) / sizeof(imeFileName[0])) - 1))
		return;

	if (!_carp_win_ime_data.GetReadingString)
	{
		if ((CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHT_IMEFILENAME1, -1) != CSTR_EQUAL) &&
			(CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHT_IMEFILENAME2, -1) != CSTR_EQUAL) &&
			(CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHT_IMEFILENAME3, -1) != CSTR_EQUAL) &&
			(CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHS_IMEFILENAME1, -1) != CSTR_EQUAL) &&
			(CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHS_IMEFILENAME2, -1) != CSTR_EQUAL))
		{
			return;
		}
	}

	TCHAR temp[1024];
	DWORD dwVerHandle;
	DWORD dwVerSize = GetFileVersionInfoSize(temp, &dwVerHandle);

	if (dwVerSize)
	{
		LPVOID  lpVerBuffer = alloca(dwVerSize);

		if (GetFileVersionInfo(temp, dwVerHandle, dwVerSize, lpVerBuffer))
		{
			LPVOID  lpVerData;
			UINT    cbVerData;

			if (VerQueryValue(lpVerBuffer, _T("\\"), &lpVerData, &cbVerData))
			{
				DWORD dwVer = ((VS_FIXEDFILEINFO*)lpVerData)->dwFileVersionMS;
				dwVer = (dwVer & 0x00ff0000) << 8 | (dwVer & 0x000000ff) << 16;
				if (_carp_win_ime_data.GetReadingString
					||
					(_carp_win_ime_data.langId == LANG_CHT &&
						(dwVer == MAKEIMEVERSION(4, 2) ||
							dwVer == MAKEIMEVERSION(4, 3) ||
							dwVer == MAKEIMEVERSION(4, 4) ||
							dwVer == MAKEIMEVERSION(5, 0) ||
							dwVer == MAKEIMEVERSION(5, 1) ||
							dwVer == MAKEIMEVERSION(5, 2) ||
							dwVer == MAKEIMEVERSION(6, 0)))
					||
					(_carp_win_ime_data.langId == LANG_CHS &&
						(dwVer == MAKEIMEVERSION(4, 1) ||
							dwVer == MAKEIMEVERSION(4, 2) ||
							dwVer == MAKEIMEVERSION(5, 3))))
				{
					_carp_win_ime_data.dwId[0] = dwVer | _carp_win_ime_data.langId;
					_carp_win_ime_data.dwId[1] = ((VS_FIXEDFILEINFO*)lpVerData)->dwFileVersionLS;
				}
			}
		}
	}
}


/*----------------------------------------------------------------------------*/
static void _carp_win_ime_setup_imeapi(HWND hWnd)
{
	char szImeFile[MAX_PATH + 1];

	_carp_win_ime_data.GetReadingString = NULL;
	_carp_win_ime_data.ShowReadingWindow = NULL;

	if (ImmGetIMEFileNameA(_carp_win_ime_data.hkl, szImeFile, sizeof(szImeFile) / sizeof(szImeFile[0]) - 1) != 0)
	{
		if (_carp_win_ime_data.hDllIme) FreeLibrary(_carp_win_ime_data.hDllIme);

		_carp_win_ime_data.hDllIme = LoadLibraryA(szImeFile);

		if (_carp_win_ime_data.hDllIme)
		{
			_carp_win_ime_data.GetReadingString = (UINT(WINAPI*)(HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT))(GetProcAddress(_carp_win_ime_data.hDllIme, "GetReadingString"));
			_carp_win_ime_data.ShowReadingWindow = (BOOL(WINAPI*)(HIMC, BOOL))(GetProcAddress(_carp_win_ime_data.hDllIme, "ShowReadingWindow"));

			if (_carp_win_ime_data.ShowReadingWindow)
			{
				HIMC hImc = ImmGetContext(hWnd);
				if (hImc)
				{
					_carp_win_ime_data.ShowReadingWindow(hImc, false);
					ImmReleaseContext(hWnd, hImc);
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
static bool	_carp_win_ime_get_reading_window_orientation()
{
	bool bHorizontalReading = (_carp_win_ime_data.hkl == _CHS_HKL) || (_carp_win_ime_data.hkl == _CHT_HKL2) || (_carp_win_ime_data.dwId[0] == 0);
	if (!bHorizontalReading && (_carp_win_ime_data.dwId[0] & 0x0000FFFF) == LANG_CHT)
	{
		char szRegPath[MAX_PATH];
		HKEY hKey;
		DWORD dwVer = _carp_win_ime_data.dwId[0] & 0xFFFF0000;
		strcpy_s(szRegPath, sizeof(szRegPath), "software\\microsoft\\windows\\currentversion\\");
		strcat_s(szRegPath, sizeof(szRegPath), (dwVer >= MAKEIMEVERSION(5, 1)) ? "MSTCIPH" : "TINTLGNT");
		LONG lRc = RegOpenKeyExA(HKEY_CURRENT_USER, szRegPath, 0, KEY_READ, &hKey);
		if (lRc == ERROR_SUCCESS)
		{
			DWORD dwSize = sizeof(DWORD), dwMapping, dwType;
			lRc = RegQueryValueExA(hKey, "Keyboard Mapping", NULL, &dwType, (PBYTE)&dwMapping, &dwSize);
			if (lRc == ERROR_SUCCESS)
			{
				if ((dwVer <= MAKEIMEVERSION(5, 0) &&
					((BYTE)dwMapping == 0x22 || (BYTE)dwMapping == 0x23))
					||
					((dwVer == MAKEIMEVERSION(5, 1) || dwVer == MAKEIMEVERSION(5, 2)) &&
						(BYTE)dwMapping >= 0x22 && (BYTE)dwMapping <= 0x24)
					)
				{
					bHorizontalReading = true;
				}
			}
			RegCloseKey(hKey);
		}
	}

	return bHorizontalReading;
}

/*----------------------------------------------------------------------------*/
static void	_carp_win_ime_get_private_reading_string(HWND hWnd)
{
	if (!_carp_win_ime_data.dwId[0])
	{
		stbds_arrsetlen(_carp_win_ime_data.reading, 0);
		return;
	}

	HIMC hImc = ImmGetContext(hWnd);
	if (!hImc)
	{
		stbds_arrsetlen(_carp_win_ime_data.reading, 0);
		return;
	}


	DWORD dwErr = 0;

	if (_carp_win_ime_data.GetReadingString)
	{
		UINT uMaxUiLen;
		BOOL bVertical;
		// Obtain the reading string size
		int wstrLen = _carp_win_ime_data.GetReadingString(hImc, 0, NULL, (PINT)&dwErr, &bVertical, &uMaxUiLen);

		if (wstrLen == 0)
		{
			stbds_arrsetlen(_carp_win_ime_data.reading, 0);
		}
		else
		{
			wchar_t* wstr = (wchar_t*)alloca(sizeof(wchar_t) * wstrLen);
			_carp_win_ime_data.GetReadingString(hImc, wstrLen, wstr, (PINT)&dwErr, &bVertical, &uMaxUiLen);
			stbds_arrsetlen(_carp_win_ime_data.reading, 0);
			for (int i = 0; i < wstrLen; ++i)
				stbds_arrpush(_carp_win_ime_data.reading, wstr[i]);
		}

		_carp_win_ime_data.bVerticalReading = bVertical ? true : false;

		ImmReleaseContext(hWnd, hImc);

	}
	else
	{
		// IMEs that doesn't implement Reading String API
		wchar_t* temp = NULL;
		DWORD tempLen = 0;
		bool bUnicodeIme = false;
		INPUTCONTEXT* lpIC = _carp_win_ime_data.ImmLockIMC(hImc);

		if (lpIC == NULL)
		{
			temp = NULL;
			tempLen = 0;
		}
		else
		{
			LPBYTE p = 0;
			switch (_carp_win_ime_data.dwId[0])
			{
			case IMEID_CHT_VER42: // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
			case IMEID_CHT_VER43: // New(Phonetic/ChanJie)IME98a : 4.3.x.x // WinMe, Win2k
			case IMEID_CHT_VER44: // New ChanJie IME98b          : 4.4.x.x // WinXP
				p = *(LPBYTE*)((LPBYTE)_carp_win_ime_data.ImmLockIMCC(lpIC->hPrivate) + 24);
				if (!p) break;
				tempLen = *(DWORD*)(p + 7 * 4 + 32 * 4);
				dwErr = *(DWORD*)(p + 8 * 4 + 32 * 4);
				temp = (wchar_t*)(p + 56);
				bUnicodeIme = true;
				break;

			case IMEID_CHT_VER50: // 5.0.x.x // WinME
				p = *(LPBYTE*)((LPBYTE)_carp_win_ime_data.ImmLockIMCC(lpIC->hPrivate) + 3 * 4);
				if (!p) break;
				p = *(LPBYTE*)((LPBYTE)p + 1 * 4 + 5 * 4 + 4 * 2);
				if (!p) break;
				tempLen = *(DWORD*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16);
				dwErr = *(DWORD*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 + 1 * 4);
				temp = (wchar_t*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4);
				bUnicodeIme = false;
				break;

			case IMEID_CHT_VER51: // 5.1.x.x // IME2002(w/OfficeXP)
			case IMEID_CHT_VER52: // 5.2.x.x // (w/whistler)
			case IMEID_CHS_VER53: // 5.3.x.x // SCIME2k or MSPY3 (w/OfficeXP and Whistler)
				p = *(LPBYTE*)((LPBYTE)_carp_win_ime_data.ImmLockIMCC(lpIC->hPrivate) + 4);
				if (!p) break;
				p = *(LPBYTE*)((LPBYTE)p + 1 * 4 + 5 * 4);
				if (!p) break;
				tempLen = *(DWORD*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 * 2);
				dwErr = *(DWORD*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 * 2 + 1 * 4);
				temp = (wchar_t*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4);
				bUnicodeIme = true;
				break;

				// the code tested only with Win 98 SE (MSPY 1.5/ ver 4.1.0.21)
			case IMEID_CHS_VER41:
			{
				int nOffset;
				nOffset = (_carp_win_ime_data.dwId[1] >= 0x00000002) ? 8 : 7;

				p = *(LPBYTE*)((LPBYTE)_carp_win_ime_data.ImmLockIMCC(lpIC->hPrivate) + nOffset * 4);
				if (!p) break;
				tempLen = *(DWORD*)(p + 7 * 4 + 16 * 2 * 4);
				dwErr = *(DWORD*)(p + 8 * 4 + 16 * 2 * 4);
				dwErr = min(dwErr, tempLen);
				temp = (wchar_t*)(p + 6 * 4 + 16 * 2 * 1);
				bUnicodeIme = true;
			}
			break;

			case IMEID_CHS_VER42: // 4.2.x.x // SCIME98 or MSPY2 (w/Office2k, Win2k, WinME, etc)
			{
				p = *(LPBYTE*)((LPBYTE)_carp_win_ime_data.ImmLockIMCC(lpIC->hPrivate) + 1 * 4 + 1 * 4 + 6 * 4);
				if (!p)
					break;

				tempLen = *(DWORD*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4 + 16 * 2);
				temp = (WCHAR*)(p + 1 * 4 + (16 * 2 + 2 * 4) + 5 * 4);
			}
			break;

			default:
				temp = NULL;
				tempLen = 0;
				break;
			}
		}

		if (tempLen == 0)
		{
			stbds_arrsetlen(_carp_win_ime_data.reading, 0);
		}
		else
		{
			if (bUnicodeIme)
			{
				stbds_arrsetlen(_carp_win_ime_data.reading, 0);
				for (DWORD i = 0; i < tempLen; ++i)
					stbds_arrpush(_carp_win_ime_data.reading, temp[i]);
			}
			else
			{
				int wstrLen = MultiByteToWideChar(_carp_win_ime_data.codePage, 0, (char*)temp, tempLen, NULL, 0);
				wchar_t* wstr = (wchar_t*)alloca(sizeof(wchar_t) * wstrLen);
				MultiByteToWideChar(_carp_win_ime_data.codePage, 0, (char*)temp, tempLen, wstr, wstrLen);
				stbds_arrsetlen(_carp_win_ime_data.reading, 0);
				for (int i = 0; i < wstrLen; ++i)
					stbds_arrpush(_carp_win_ime_data.reading, wstr[i]);
			}
		}

		_carp_win_ime_data.ImmUnlockIMCC(lpIC->hPrivate);
		_carp_win_ime_data.ImmUnlockIMC(hImc);

		_carp_win_ime_data.bVerticalReading = !_carp_win_ime_get_reading_window_orientation();
	}

	ImmReleaseContext(hWnd, hImc);
}

/*----------------------------------------------------------------------------*/
static void _carp_win_ime_check_toggle_state(HWND hWnd)
{
	/* Update Indicator */
	switch (PRIMARYLANGID(_carp_win_ime_data.langId))
	{
	case LANG_KOREAN:
		_carp_win_ime_data.bVerticalCandidate = false;
		_carp_win_ime_data.wszCurrIndicator = INDICATOR[INDICATOR_KOREAN];
		break;

	case LANG_JAPANESE:
		_carp_win_ime_data.bVerticalCandidate = true;
		_carp_win_ime_data.wszCurrIndicator = INDICATOR[INDICATOR_JAPANESE];
		break;

	case LANG_CHINESE:
		_carp_win_ime_data.bVerticalCandidate = true;
		switch (SUBLANGID(_carp_win_ime_data.langId))
		{
		case SUBLANG_CHINESE_SIMPLIFIED:
			_carp_win_ime_data.bVerticalCandidate = _carp_win_ime_data.dwId[0] == 0;
			_carp_win_ime_data.wszCurrIndicator = INDICATOR[INDICATOR_CHS];
			break;

		case SUBLANG_CHINESE_TRADITIONAL:
			_carp_win_ime_data.wszCurrIndicator = INDICATOR[INDICATOR_CHT];
			break;
		default:
			_carp_win_ime_data.wszCurrIndicator = INDICATOR[INDICATOR_NON_IME];
			break;
		}
		break;
	default:
		_carp_win_ime_data.wszCurrIndicator = INDICATOR[INDICATOR_NON_IME];
		break;
	}

	if (_carp_win_ime_data.wszCurrIndicator == INDICATOR[INDICATOR_NON_IME])
	{
		char szLang[10];
		GetLocaleInfoA(MAKELCID(_carp_win_ime_data.langId, SORT_DEFAULT), LOCALE_SABBREVLANGNAME, szLang, sizeof(szLang));
		_carp_win_ime_data.wszCurrIndicator[0] = szLang[0];
		_carp_win_ime_data.wszCurrIndicator[1] = towlower(szLang[1]);
	}



	/* Check Toggle State */
	bool bIme = ImmIsIME(_carp_win_ime_data.hkl) != 0;

	HIMC hImc = ImmGetContext(hWnd);
	if (hImc)
	{
		if ((PRIMARYLANGID(_carp_win_ime_data.langId) == LANG_CHINESE) && bIme)
		{
			DWORD dwConvMode, dwSentMode;
			ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);
			_carp_win_ime_data.ImeState = (dwConvMode & IME_CMODE_NATIVE) ? IMEUI_STATE_ON : IMEUI_STATE_ENGLISH;
		}
		else
		{
			_carp_win_ime_data.ImeState = (bIme && ImmGetOpenStatus(hImc) != 0) ? IMEUI_STATE_ON : IMEUI_STATE_OFF;
		}
		ImmReleaseContext(hWnd, hImc);

	}
	else
	{
		_carp_win_ime_data.ImeState = IMEUI_STATE_OFF;
	}
}

/*----------------------------------------------------------------------------*/
static int _carp_win_ime_get_charset_from_lang(LANGID langid)
{
	switch (PRIMARYLANGID(langid))
	{
	case LANG_JAPANESE:
		return SHIFTJIS_CHARSET;
	case LANG_KOREAN:
		return HANGEUL_CHARSET;
	case LANG_CHINESE:
		switch (SUBLANGID(langid))
		{
		case SUBLANG_CHINESE_SIMPLIFIED:
			return GB2312_CHARSET;
		case SUBLANG_CHINESE_TRADITIONAL:
			return CHINESEBIG5_CHARSET;
		default:
			return ANSI_CHARSET;
		}
	case LANG_GREEK:
		return GREEK_CHARSET;
	case LANG_TURKISH:
		return TURKISH_CHARSET;
	case LANG_HEBREW:
		return HEBREW_CHARSET;
	case LANG_ARABIC:
		return ARABIC_CHARSET;
	case LANG_ESTONIAN:
	case LANG_LATVIAN:
	case LANG_LITHUANIAN:
		return BALTIC_CHARSET;
	case LANG_THAI:
		return THAI_CHARSET;
	case LANG_CZECH:
	case LANG_HUNGARIAN:
	case LANG_POLISH:
	case LANG_CROATIAN:
	case LANG_MACEDONIAN:
	case LANG_ROMANIAN:
	case LANG_SLOVAK:
	case LANG_SLOVENIAN:
		return EASTEUROPE_CHARSET;
	case LANG_RUSSIAN:
	case LANG_BELARUSIAN:
	case LANG_BULGARIAN:
	case LANG_UKRAINIAN:
		return RUSSIAN_CHARSET;
	case LANG_VIETNAMESE:
		return VIETNAMESE_CHARSET;
	default:
		return ANSI_CHARSET;
	}
}

/*----------------------------------------------------------------------------*/
static int _carp_win_ime_get_codepage_from_charset(int charset)
{
	switch (charset)
	{
	case SHIFTJIS_CHARSET:
		return 932;
	case HANGUL_CHARSET:
		return 949;
	case GB2312_CHARSET:
		return 936;
	case CHINESEBIG5_CHARSET:
		return 950;
	case GREEK_CHARSET:
		return 1253;
	case TURKISH_CHARSET:
		return 1254;
	case HEBREW_CHARSET:
		return 1255;
	case ARABIC_CHARSET:
		return 1256;
	case BALTIC_CHARSET:
		return 1257;
	case THAI_CHARSET:
		return 874;
	case EASTEUROPE_CHARSET:
		return 1250;
	case VIETNAMESE_CHARSET:
		return 1258;
	default:
		return 1252;
	}
}

/*----------------------------------------------------------------------------*/
static int _carp_win_ime_get_codepage_from_lang(LANGID langid)
{
	return _carp_win_ime_get_codepage_from_charset(_carp_win_ime_get_charset_from_lang(langid));
}

static void _carp_win_ime_on_input_language_change(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	_carp_win_ime_data.hkl = (HKL)lParam;
	_carp_win_ime_data.langId = LOWORD(_carp_win_ime_data.hkl);
	_carp_win_ime_data.codePage = _carp_win_ime_get_codepage_from_lang(_carp_win_ime_data.langId);
	stbds_arrsetlen(_carp_win_ime_data.input, 0);

	/* Check Property */
	DWORD property = ImmGetProperty(GetKeyboardLayout(0), IGP_PROPERTY);

	_carp_win_ime_data.bUnicodeIME = (property & IME_PROP_UNICODE) ? true : false;

	/* Update Indicator */
	_carp_win_ime_check_toggle_state(hWnd);

	/* Update m_dwId[] */
	_carp_win_ime_get_imeid();

	/* Bind Proc */
	_carp_win_ime_setup_imeapi(hWnd);
}

/*----------------------------------------------------------------------------*/
static bool _carp_win_ime_on_composition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HIMC hImc;
	if (lParam & GCS_COMPSTR)
	{
		hImc = ImmGetContext(hWnd);
		if (hImc)
		{

			int tempSize = ImmGetCompositionStringW(hImc, GCS_COMPSTR, NULL, 0);
			wchar_t* temp = (wchar_t*)alloca(tempSize);
			ImmGetCompositionStringW(hImc, GCS_COMPSTR, temp, tempSize);

			stbds_arrsetlen(_carp_win_ime_data.comp, 0);
			for (int i = 0; i < tempSize / sizeof(wchar_t); ++i)
				stbds_arrpush(_carp_win_ime_data.comp, temp[i]);

			ImmReleaseContext(hWnd, hImc);
		}
	}
	if (lParam & GCS_RESULTSTR)
	{
		hImc = ImmGetContext(hWnd);
		if (hImc)
		{
			int tempSize = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);

			wchar_t* temp = (wchar_t*)alloca(tempSize);

			ImmGetCompositionStringW(hImc, GCS_RESULTSTR, temp, tempSize);

			stbds_arrsetlen(_carp_win_ime_data.input, 0);
			for (int i = 0; i < tempSize / sizeof(wchar_t); ++i)
				stbds_arrpush(_carp_win_ime_data.input, temp[i]);

			ImmReleaseContext(hWnd, hImc);
		}
	}
	if (lParam & GCS_COMPATTR)
	{
		hImc = ImmGetContext(hWnd);
		if (hImc)
		{
			int tempSize = ImmGetCompositionStringW(hImc, GCS_COMPATTR, NULL, 0);

			BYTE* temp = (BYTE*)alloca(tempSize);

			ImmGetCompositionStringW(hImc, GCS_COMPATTR, temp, tempSize);

			int start, end;
			for (start = 0; start < tempSize; ++start) if (temp[start] == ATTR_TARGET_CONVERTED || temp[start] == ATTR_TARGET_NOTCONVERTED) break;
			for (end = start; end < tempSize; ++end) if (temp[end] != temp[start]) break;

			_carp_win_ime_data.ulStart = start;
			_carp_win_ime_data.ulEnd = end;

			ImmReleaseContext(hWnd, hImc);
		}
	}

	return true;
}

/*----------------------------------------------------------------------------*/
static bool _carp_win_ime_on_end_composition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	stbds_arrsetlen(_carp_win_ime_data.comp, 0);
	_carp_win_ime_data.ulStart = _carp_win_ime_data.ulEnd = 0;
	stbds_arrsetlen(_carp_win_ime_data.reading, 0);
	return true;
}

/*----------------------------------------------------------------------------*/
static bool _carp_win_ime_on_notify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HIMC hImc;

	switch (wParam)
	{
	case IMN_OPENCANDIDATE:
	case IMN_CHANGECANDIDATE:
		hImc = ImmGetContext(hWnd);
		if (hImc)
		{
			stbds_arrsetlen(_carp_win_ime_data.reading, 0);
			int candidateLen = ImmGetCandidateListW(hImc, 0, NULL, 0);
			if (candidateLen > 0)
			{
				stbds_arrsetlen(_carp_win_ime_data.candidate, candidateLen);
				ImmGetCandidateListW(hImc, 0, (CANDIDATELIST*)_carp_win_ime_data.candidate, candidateLen);
			}
			ImmReleaseContext(hWnd, hImc);
		}
		return true;
	case IMN_CLOSECANDIDATE:
		stbds_arrsetlen(_carp_win_ime_data.candidate, 0);
		return true;

	case IMN_SETCONVERSIONMODE:
	case IMN_SETOPENSTATUS:
		_carp_win_ime_check_toggle_state(hWnd);
		return false;

	case IMN_PRIVATE:
		_carp_win_ime_get_private_reading_string(hWnd);

		// Trap some messages to hide reading window
		switch (_carp_win_ime_data.dwId[0])
		{
		case IMEID_CHT_VER42:
		case IMEID_CHT_VER43:
		case IMEID_CHT_VER44:
		case IMEID_CHS_VER41:
		case IMEID_CHS_VER42:
			if ((lParam == 1) || (lParam == 2)) return true;
			break;

		case IMEID_CHT_VER50:
		case IMEID_CHT_VER51:
		case IMEID_CHT_VER52:
		case IMEID_CHT_VER60:
		case IMEID_CHS_VER53:
			if ((lParam == 16) || (lParam == 17) || (lParam == 26) || (lParam == 27) || (lParam == 28)) return true;
			break;
		}
		break;
	}

	return false;
}

/*----------------------------------------------------------------------------*/
static bool _carp_win_ime_on_char(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case '\r':
	case '\n':
		stbds_arrsetlen(_carp_win_ime_data.input, 0);
		break;
	case '\b':
		stbds_arrpop(_carp_win_ime_data.input);
		break;
	case '\t':
	case 27:
		break;
	default:
		if (wParam > 31)
		{
			wchar_t temp;
			MultiByteToWideChar(_carp_win_ime_data.codePage, 0, (char*)&wParam, 1, &temp, 1);
			stbds_arrpush(_carp_win_ime_data.input, temp);
		}
		break;
	}

	return true;
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_input(char* text, int len)
{
	return _carp_win_ime_wchar_to_char(_carp_win_ime_data.codePage, _carp_win_ime_data.input, (int)stbds_arrlen(_carp_win_ime_data.input), text, len);
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_comp(char* text, int len)
{
	return _carp_win_ime_wchar_to_char(_carp_win_ime_data.codePage, _carp_win_ime_data.comp, (int)stbds_arrlen(_carp_win_ime_data.comp), text, len);
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_reading(char* text, int len)
{
	return _carp_win_ime_wchar_to_char(_carp_win_ime_data.codePage, _carp_win_ime_data.reading, (int)stbds_arrlen(_carp_win_ime_data.reading), text, len);
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_candidate(DWORD index, char* text, int len)
{
	if (stbds_arrlen(_carp_win_ime_data.candidate) == 0) return 0;
	CANDIDATELIST* candidateList = (CANDIDATELIST*)_carp_win_ime_data.candidate;

	if (index >= candidateList->dwCount) return 0;

	if (_carp_win_ime_data.bUnicodeIME)
	{
		wchar_t* wText = (wchar_t*)(_carp_win_ime_data.candidate + candidateList->dwOffset[index]);
		return _carp_win_ime_wchar_to_char(_carp_win_ime_data.codePage, wText, (int)wcslen(wText), text, len);
	}

	char* temp = (char*)(_carp_win_ime_data.candidate + candidateList->dwOffset[index]);
	int tempLen = (int)strlen(temp);
	if (len < tempLen) return 0;
	memcpy(text, temp, tempLen);
	
	return tempLen;
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_candidate_count()
{
	if (stbds_arrlen(_carp_win_ime_data.candidate) == 0) return 0;
	return ((CANDIDATELIST*)_carp_win_ime_data.candidate)->dwCount;
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_candidate_selection()
{
	if (stbds_arrlen(_carp_win_ime_data.candidate) == 0) return 0;

	if (PRIMARYLANGID(_carp_win_ime_data.langId) == LANG_KOREAN)
		return ((CANDIDATELIST*)_carp_win_ime_data.candidate)->dwCount;
		
	return ((CANDIDATELIST*)_carp_win_ime_data.candidate)->dwSelection;
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_candidate_pagesize()
{
	if (stbds_arrlen(_carp_win_ime_data.candidate) == 0) return 0;
	return ((CANDIDATELIST*)_carp_win_ime_data.candidate)->dwPageSize;
}

/*----------------------------------------------------------------------------*/
static int carp_win_ime_get_candidate_pagestart()
{
	if (stbds_arrlen(_carp_win_ime_data.candidate) == 0) return 0;
	return ((CANDIDATELIST*)_carp_win_ime_data.candidate)->dwPageStart;
}

/*----------------------------------------------------------------------------*/
static void	carp_win_ime_get_underline(int* start, int* end)
{
	*start = WideCharToMultiByte(_carp_win_ime_data.codePage, 0, _carp_win_ime_data.comp, _carp_win_ime_data.ulStart, NULL, 0, NULL, NULL);
	*end = WideCharToMultiByte(_carp_win_ime_data.codePage, 0, _carp_win_ime_data.comp, _carp_win_ime_data.ulEnd, NULL, 0, NULL, NULL);
}

static void _carp_win_ime_send_editing_event()
{
	if (_carp_win_ime_data.editing_callback)
		_carp_win_ime_data.editing_callback();
}

static bool carp_win_ime_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		_carp_win_ime_on_input_language_change(hWnd, wParam, lParam);
		return true;
		/* IME Messages */
	case WM_INPUTLANGCHANGE:
		_carp_win_ime_on_input_language_change(hWnd, wParam, lParam);
		_carp_win_ime_send_editing_event();
		break;
	case WM_IME_SETCONTEXT:
		lParam = 0;
		break;
	case WM_IME_STARTCOMPOSITION:
		return true;
	case WM_IME_COMPOSITION:
		if (_carp_win_ime_on_composition(hWnd, wParam, lParam))
		{
			_carp_win_ime_send_editing_event();
			return true;
		}
		break;
	case WM_IME_ENDCOMPOSITION:
		if (_carp_win_ime_on_end_composition(hWnd, wParam, lParam))
		{
			_carp_win_ime_send_editing_event();
			return true;
		}
		break;

	case WM_IME_NOTIFY:
		if (_carp_win_ime_on_notify(hWnd, wParam, lParam))
		{
			_carp_win_ime_send_editing_event();
			return true;
		}
		_carp_win_ime_send_editing_event();
		break;
	}

	return false;
}

#endif
#endif