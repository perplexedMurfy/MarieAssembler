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

int IndexOfFromEnd(wchar_t *String, wchar_t Target) {
	int Result = 0;

	while(String[Result] != L'\0') { Result++; }
	Result--;
	
	for(; String[Result] != L'\0'; Result--) {
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

const char* HelpMessage =
	"Usage: MarieAssembler <InFileName> [Output Options]\n"
	"Where [Output Options] can be any combination of:\n"
	"  --logisim [FileName] ==> Outputs Logisim rom image at [FileName], or if blank <InFileName>.LogisimImage\n"
	"  --rawhex [FileName] ==> Outputs a file containing the raw hex for the program at [FileName], or if blank <InFileName>.hex\n"
	"  --symboltable [FileName] ==> Outputs a file containing a symbol table for the program at [FileName], or if blank <InFileName>.sym\n"
	"  --listing [FileName] ==> Outputs a file containing a listing for the program at [FileName], or if blank <InFileName>.lst\n";

int wmain(int ArgCount, wchar_t **Args, wchar_t **Env) {
	FILE *InFile = 0, *OutLogisim = 0, *OutHex = 0, *OutSymbolTable = 0, *OutListing = 0;
	int GenLogisim = FALSE, GenHex = FALSE, GenSymbolTable = FALSE, GenListing = FALSE;
	wchar_t *InFileName = 0;
	uint64_t InFileSize = 0;
	int Success = TRUE;

	if (ArgCount == 1) {
		printf(HelpMessage);
		Success = FALSE;
	}
	
	for (int Index = 1; Index < ArgCount;) {
		wchar_t * Arg = Args[Index];
		
		if (StartsWith(Arg, L"--logisim")) {
			if (Index + 1 >= ArgCount) { Arg = L""; }
			else { Arg = Args[Index + 1]; }

			if (OutLogisim == 0 && GenLogisim == FALSE) {
				if (!((StartsWith(Arg, L"--")) || (Arg[0] == 0))) {
					Index++;
					OutLogisim = _wfopen(Arg, L"w");
					if (OutLogisim == 0) {
						wprintf(L"I could not open the Logisim output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
				}
				else {
					GenLogisim = TRUE;
				}
			}
			else {
				wprintf(L"Option --logisim was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, L"--rawhex")) {
			if (Index + 1 >= ArgCount) { Arg = L""; }
			else { Arg = Args[Index + 1]; }

			if (OutHex == 0 && GenHex == FALSE) {
				if (!((StartsWith(Arg, L"--")) || (Arg[0] == 0))) {
					Index++;
					OutHex = _wfopen(Arg, L"wb");
					if (OutHex == 0) {
						wprintf(L"I could not open the raw hex output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
				}
				else {
					GenHex = TRUE;
				}
			}
			else {
				wprintf(L"Option --rawhex was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, L"--symboltable")) {
			if (Index + 1 >= ArgCount) { Arg = L""; }
			else { Arg = Args[Index + 1]; }

			if (OutSymbolTable == 0 && GenSymbolTable == FALSE) {
				if (!((StartsWith(Arg, L"--")) || (Arg[0] == 0))) {
					Index++;
					OutSymbolTable = _wfopen(Arg, L"w,ccs=UNICODE");
					if (OutSymbolTable == 0) {
						wprintf(L"I could not open the symbol table output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
				}
				else {
					GenSymbolTable = TRUE;
				}
			}
			else {
				wprintf(L"Option --symboltable was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, L"--listing")) {
			if (Index + 1 >= ArgCount) { Arg = L""; }
			else { Arg = Args[Index + 1]; }

			if (OutListing == 0 && GenListing == FALSE) {
				if (!((StartsWith(Arg, L"--")) || (Arg[0] == 0))) {
					Index++;
					OutListing = _wfopen(Arg, L"w,ccs=UNICODE");
					if (OutListing == 0) {
						wprintf(L"I could not open the listing output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
				}
				else {
					GenListing = TRUE;
				}
			}
			else {
				wprintf(L"Option --listing was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, L"--")) {
			wprintf(L"Unknown commandline operation encountered: \"%s\"\n", Arg);
			printf(HelpMessage);
			Success = FALSE;
			break;
		}
		else if (InFile == 0) { // This must be our one input file.
			InFile = _wfopen(Arg, L"rb");
			if (InFile == 0) {
				wprintf(L"I could not open the input file \"%s\" for reading!\n", Arg);
				Success = FALSE;
				break;
			}
			if (Success) {
				InFileSize = Platform_GetFileSize(Arg, &Success);
				InFileName = Arg;
			}
		}
		else {
			wprintf(L"Unknown commandline operation encountered: \"%s\"\n", Arg);
			printf(HelpMessage);
			Success = FALSE;
			break;
		}

		Index++;
	}

	if (GenLogisim) {
		// @TODO this block could be pulled out into a function.
		int DotIndex = IndexOfFromEnd(InFileName, L'.');
		int AutoFileNameLength = DotIndex + wcslen(L".LogisimImage") + 1;
		wchar_t *AutoFileName = calloc(AutoFileNameLength, sizeof(wchar_t));
		_snwprintf(AutoFileName, AutoFileNameLength, L"%.*s.LogisimImage", DotIndex, InFileName);

		OutLogisim = _wfopen(AutoFileName, L"w");
		if (OutLogisim == 0) {
			wprintf(L"I could not open the Logisim output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
			Success = FALSE;
		}
		free(AutoFileName);
	}
	if (GenHex) {
		int DotIndex = IndexOfFromEnd(InFileName, L'.');
		int AutoFileNameLength = DotIndex + wcslen(L".hex") + 1;
		wchar_t *AutoFileName = calloc(AutoFileNameLength, sizeof(wchar_t));
		_snwprintf(AutoFileName, AutoFileNameLength, L"%.*s.hex", DotIndex, InFileName);

		OutHex = _wfopen(AutoFileName, L"wb");
		if (OutLogisim == 0) {
			wprintf(L"I could not open the raw hex output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
			Success = FALSE;
		}
		free(AutoFileName);
	}
	if (GenListing) {
		int DotIndex = IndexOfFromEnd(InFileName, L'.');
		int AutoFileNameLength = DotIndex + wcslen(L".lst") + 1;
		wchar_t *AutoFileName = calloc(AutoFileNameLength, sizeof(wchar_t));
		_snwprintf(AutoFileName, AutoFileNameLength, L"%.*s.lst", DotIndex, InFileName);

		OutListing = _wfopen(AutoFileName, L"w,ccs=UNICODE");
		if (OutLogisim == 0) {
			wprintf(L"I could not open the listing output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
			Success = FALSE;
		}
		free(AutoFileName);
	}
	if (GenSymbolTable) {
		int DotIndex = IndexOfFromEnd(InFileName, L'.');
		int AutoFileNameLength = DotIndex + wcslen(L".sym") + 1;
		wchar_t *AutoFileName = calloc(AutoFileNameLength, sizeof(wchar_t));
		_snwprintf(AutoFileName, AutoFileNameLength, L"%.*s.sym", DotIndex, InFileName);

		OutSymbolTable = _wfopen(AutoFileName, L"w,ccs=UNICODE");
		if (OutLogisim == 0) {
			wprintf(L"I could not open the Logisim output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
			Success = FALSE;
		}
		free(AutoFileName);
	}

	if (Success) {
		Success = ApplicationMain(InFile, InFileSize, OutLogisim, OutHex, OutSymbolTable, OutListing);
	}
	else {
		printf("Exiting without invoking the assembler.\n");
	}

	return Success;
}
