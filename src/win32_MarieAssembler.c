/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * File: This file contains all platform spefic code for the win32 platform.
 * Use this as a base if you wish to port this program to another platform.
 * 
 * When building, build the platform layer in a different translation unit than the application. Then link the two together.
 */

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#undef WIN32_LEAN_AND_MEAN
#undef UNICODE

#include "Platform_MarieAssembler.h"

void Platform_Breakpoint() {
	__debugbreak();
}

size_t Platform_GetFileSize(wchar_t *FileName, int *Success) {
	LARGE_INTEGER Result = {0};
	
	HANDLE FileHandle = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	
	if (GetFileSizeEx(FileHandle, &Result) == 0) {
		DWORD Error = GetLastError();
		wprintf(L"[Error File Handling] This program could not get the size of the provided file (%s).\nThe value of win32's GetLastError is '%d'", FileName, Error);
		*Success = FALSE;
	}
	else { SetLastError(0); }

	return (size_t)Result.QuadPart;
}

FILE *Platform_WideFOpen(wchar_t *Path, wchar_t *Mode) {
	return _wfopen(Path, Mode);
}

wchar_t *Platform_CreateOutputFileName(wchar_t *Path, int *Success) {
	wchar_t *Result = 0;

	wchar_t *StartOfFileName = PathFindFileName(Path);
	int OutputNameLength = IndexOfFromEnd(StartOfFileName, L'.');

	int PathLength = 0;
	for(int Index = 0; Path[Index] != L'\0'; Index++) {
		if (Path[Index] == L'\\' ||
		    Path[Index] == L'/') { // @TODO I'm not totally sure if this is kosher on windows.
			PathLength = Index + 1;
		}
	}

	if (*Success) {
		Result = calloc(PathLength + OutputNameLength + wcslen(L"OUT_") + 1, sizeof(wchar_t));
		_snwprintf(Result, PathLength + OutputNameLength + wcslen(L"OUT_") + 1, L"%.*sOUT_%.*s", PathLength, Path, OutputNameLength, StartOfFileName);
	}

	if (!Result) { *Success = FALSE; }
	return Result;
}

int wmain(int ArgCount, wchar_t **Args, wchar_t **Env) {
	return ApplicationMain(ArgCount, Args);
}
