/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
 * File: This file contains all platform spefic code for the win32 platform.
 * Use this as a base if you wish to port this program to another platform.
 * 
 * When building, build the platform layer in a different translation unit than the application. then link the two together.
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

void *Platform_AllocateMemory(size_t Size) {
	return VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
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

// @TODO: This function only loads utf-16 little endian files.
wchar_t* Platform_LoadFileIntoMemory(wchar_t *FileName, int *Success) {	
	wchar_t *Result = 0;

	HANDLE FileHandle = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	
	LARGE_INTEGER FileSize;
	if (GetFileSizeEx(FileHandle, &FileSize) == 0) {
		DWORD Error = GetLastError();
		wprintf(L"[Error File Handling] This program could not get the size of the provided file (%s).\nThe value of win32's GetLastError is '%d'", FileName, Error);
		*Success = FALSE;
	}
	else { SetLastError(0); }

	Result = malloc(FileSize.QuadPart + sizeof(wchar_t));
	Result[FileSize.QuadPart/sizeof(wchar_t)] = L'\0';

	DWORD BytesRead = 0;

	uint8_t ByteOrderMark[4];
	ReadFile(FileHandle, &ByteOrderMark, 4, &BytesRead, 0);
	if ((ByteOrderMark[0] == 0xFF) &&
	    (ByteOrderMark[1] == 0xFE) &&
	    (ByteOrderMark[2] == 0x00) &&
	    (ByteOrderMark[3] == 0x00)) { // UTF-32-LE
	}
	else if ((ByteOrderMark[0] == 0x00) &&
	         (ByteOrderMark[1] == 0x00) &&
	         (ByteOrderMark[2] == 0xFE) &&
	         (ByteOrderMark[3] == 0xFF)) { // UTF-32-BE
	}
	else if ((ByteOrderMark[0] == 0xFF) &&
	         (ByteOrderMark[1] == 0xFE)) { // UTF-16-LE
	}
	else if ((ByteOrderMark[0] == 0xFE) &&
	         (ByteOrderMark[1] == 0xFF)) { // UTF-16-BE
	}
	else if ((ByteOrderMark[0] == 0xEF) &&
	         (ByteOrderMark[1] == 0xBB) &&
	         (ByteOrderMark[2] == 0xBF)) { // UTF-8
	}
	else { // Assuming Ascii
	}
	
	if (ReadFile(FileHandle, Result, FileSize.QuadPart, &BytesRead, 0) == FALSE) {
		DWORD Error = GetLastError();
		wprintf(L"[Error File Handling] This program could not read the contents of the provided file (%s).\nThe value of win32's GetLastError is '%d'", FileName, Error);
		*Success = FALSE;
	};

	CloseHandle(FileHandle);

	if (Result[0] != 0xfeff) {
		wprintf(L"[Error File Handling] This program only supports UTF-16 in Little Endian. Please change you source file's encoding to UTF-16 Little Endian, or program in support for other character formats.");
		*Success = FALSE;
	}
	Result += 1;

	return Result;
}

wchar_t *Platform_CreateOutputFileName(wchar_t *Path, linear_arena *Arena, int *Success) {
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
		Result = PushArray(Arena, wchar_t, PathLength + OutputNameLength + wcslen(L"OUT_") + 1);
		_snwprintf(Result, PathLength + OutputNameLength + wcslen(L"OUT_") + 1, L"%.*sOUT_%.*s", PathLength, Path, OutputNameLength, StartOfFileName);
	}

	if (!Result) { *Success = FALSE; }
	return Result;
}

int wmain(int ArgCount, wchar_t **Args, wchar_t **Env) {
	return ApplicationMain(ArgCount, Args);
}
