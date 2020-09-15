#ifndef CARP_WIN_IME_INCLUDED
#define CARP_WIN_IME_INCLUDED (1)

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <minwindef.h>
#include <imm.h>
#include <Dimm.h>

typedef struct
{
	HINSTANCE hImm32;
	INPUTCONTEXT* (WINAPI* ImmLockIMC)(HIMC);
	BOOL(WINAPI* ImmUnlockIMC)(HIMC);
	LPVOID(WINAPI* ImmLockIMCC)(HIMCC);
	BOOL(WINAPI* ImmUnlockIMCC)(HIMCC);
} _carp_win_ime_data_t;

static _carp_win_ime_data_t _carp_win_ime_data = {0};

static bool _carp_win_ime_data_setup()
{
	if (_carp_win_ime_data.hImm32 != NULL) return true;
	_carp_win_ime_data.hImm32 = LoadLibrary("imm32.dll");
	if (_carp_win_ime_data.hImm32 == NULL) return false;

	_carp_win_ime_data.ImmLockIMC = (INPUTCONTEXT * (WINAPI*)(HIMC))GetProcAddress(_carp_win_ime_data.hImm32, "ImmLockIMC");
	_carp_win_ime_data.ImmUnlockIMC = (BOOL(WINAPI*)(HIMC))GetProcAddress(_carp_win_ime_data.hImm32, "ImmUnlockIMC");
	_carp_win_ime_data.ImmLockIMCC = (LPVOID(WINAPI*)(HIMCC))GetProcAddress(_carp_win_ime_data.hImm32, "ImmLockIMCC");
	_carp_win_ime_data.ImmUnlockIMCC = (BOOL(WINAPI*)(HIMCC)) GetProcAddress(_carp_win_ime_data.hImm32, "ImmUnlockIMCC");
}

static void _carp_win_ime_data_shutdown()
{
	if (_carp_win_ime_data.hImm32 == NULL) return;
	FreeLibrary(_carp_win_ime_data.hImm32);
	_carp_win_ime_data.hImm32 = NULL;
	_carp_win_ime_data.ImmLockIMC = NULL;
	_carp_win_ime_data.ImmUnlockIMC = NULL;
	_carp_win_ime_data.ImmLockIMCC = NULL;
	_carp_win_ime_data.ImmUnlockIMCC = NULL;
}

#endif
#endif