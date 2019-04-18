// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <Windows.h>

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		MessageBox(0, L"DLL injected sucessfully!", L"DLL Injection Demo", MB_OK);
	}
	return TRUE;
}
