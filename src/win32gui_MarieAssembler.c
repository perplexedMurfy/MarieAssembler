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

size_t win32_GetFileSize(wchar_t *FileName, int *Success) {
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

int IndexOfFromEnd(wchar_t *String, wchar_t Target) {
	int Result = 0;

	Result = wcslen(String) - 1;
	
	for(; Result >= 0; Result--) {
		if (String[Result] == Target) { return Result; }
	}
	return -1;
}

int StartsWith(wchar_t *String, wchar_t *Target) {
	int Result = FALSE;
	int Length = wcslen(Target);
	if (Length <= wcslen(String)) {
		for (int Index = 0; Index <= Length; Index++) {
			if (Index == Length) { Result = TRUE; break; }
			if (String[Index] != Target[Index]) { break; }
		}
	}

	return Result;
}

wchar_t* GenerateOutputPath(wchar_t *InFileName, wchar_t *PostFix) {
	int DotIndex = IndexOfFromEnd(InFileName, L'.');
	int PathSeperatorIndex = Max(IndexOfFromEnd(InFileName, L'\\'), IndexOfFromEnd(InFileName, L'/'));
	if (DotIndex < PathSeperatorIndex || DotIndex == -1) {
		// The dot we found was part of the file path.
		// Or we didn't find a dot.
		DotIndex = wcslen(InFileName);
	}
	
	int AutoFileNameLength = DotIndex + wcslen(PostFix) + 1;
	wchar_t *AutoFileName = calloc(AutoFileNameLength, sizeof(wchar_t));
	
	_snwprintf(AutoFileName, AutoFileNameLength, L"%.*s%s", DotIndex, InFileName, PostFix);

	return AutoFileName;
}

int __declspec(dllexport) __stdcall VisualBasicEntryPoint(wchar_t *InputPath, wchar_t *LogisimPath, wchar_t *HexPath, wchar_t *SymbolTablePath, wchar_t *ListingPath) {
	FILE *InFile = 0, *OutLogisim = 0, *OutHex = 0, *OutSymbolTable = 0, *OutListing = 0;
	int InFileSize = -1;
	int Success = TRUE;

	freopen(".\\stdout.txt", "w+", stdout);
	
	if (*InputPath != 0) {
		InFile = _wfopen(InputPath, L"rb");
		if (!InFile) {
			wprintf(L"[Error File Handling] Input file path \"%s\" could not be open for reading!\n", InputPath);
			Success = FALSE;
		}
		else {
			InFileSize = win32_GetFileSize(InputPath, &Success);
		}
	}
	else {
		wprintf(L"[Error File Handling] Input file path was not provided!\n");
		Success = FALSE;
	}

	if (*LogisimPath != 0) {
		OutLogisim = _wfopen(LogisimPath, L"w");
		if (OutLogisim == 0) {
			Success = FALSE;
			wprintf(L"[Error File Handling] Logisim Output path \"%s\" could not be opened for writing!\n", LogisimPath);
		}
	}
	
	if (*HexPath != 0) {
		OutHex = _wfopen(HexPath, L"wb");
		if (OutHex == 0) {
			Success = FALSE;
			wprintf(L"[Error File Handling] Raw Output path \"%s\" could not be opened for writing!\n", HexPath);
		}
	}

	if (*SymbolTablePath != 0) {
		OutSymbolTable = _wfopen(SymbolTablePath, L"w");
		if (OutSymbolTable == 0) {
			Success = FALSE;
			wprintf(L"[Error File Handling] Symbol Table path \"%s\" could not be opened for writing!\n", SymbolTablePath);
		}
	}
	
	if (*ListingPath != 0) {
		OutListing = _wfopen(ListingPath, L"w");
		if (OutListing == 0) {
			Success = FALSE;
			wprintf(L"[Error File Handling] Listing Output path \"%s\" could not be opened for writing!\n", ListingPath);
		}
	}

	if (Success) {
		Success = ApplicationMain(InFile, InFileSize, OutLogisim, OutHex, OutSymbolTable, OutListing);
	}
	else {
		wprintf(L"Exiting without invoking the assembler.\n");
	}

	if (!Success) {
		if (OutLogisim) {
			fclose(OutLogisim);
			Assert(*LogisimPath != 0);
			DeleteFile(LogisimPath);
		}
		if (OutSymbolTable) {
			fclose(OutSymbolTable);
			Assert(*SymbolTablePath != 0);
			DeleteFile(SymbolTablePath);
		}
		if (OutHex) {
			fclose(OutHex);
			Assert(*HexPath != 0);
			DeleteFile(HexPath);
		}
		if (OutListing) {
			fclose(OutListing);
			Assert(*ListingPath != 0);
			DeleteFile(ListingPath);
		}
		
	}

	fclose(stdout);
	return Success;
}
