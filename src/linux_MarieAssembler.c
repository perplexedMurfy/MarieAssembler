/* Author: Zack Amoroso <zamoroso@student.fitchburgstate.edu>
 * Date: 2021-12-31
 * File: This file contains all platform specific code for Linux.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "Platform_MarieAssembler.h"

void Platform_Breakpoint() {
	raise(SIGINT);
}

size_t GetFileSize(char *FileName, int *Success) {
	struct stat fInfo;

	if(stat(FileName, &fInfo) == -1) {
		fprintf(stderr, "Error getting file info for file: %s\n%s\n", FileName, strerror(errno));
		*Success = FALSE;
	}
	FILE *FileHandle = fopen(FileName, "r");
	if (FileHandle == NULL) {
		fprintf(stderr, "Error opening file: %s\n%s\n", FileName, strerror(errno));
		*Success = FALSE;
	}
	return (size_t)fInfo.st_size;
}

int IndexOfFromEnd(char *String, char Target) {
	int Result = strlen(String);

	for(; Result > 0; Result--) {
		if (String[Result] == Target) {return Result;}
	}
	return -1;
}

int StartsWith(char *String, char *Target) {
	int Result = FALSE;
	int Length = strlen(Target);
	if (Length <= strlen(String)) {
		for (int Index = 0; Index <= Length; Index++) {
			if (Index == Length) { Result = TRUE; break; }
			if (String[Index] != Target[Index]) { break; }
		}
	}
	return Result;
}

char* GenerateOutputPath(char *InFileName, char *PostFix) {
	int DotIndex = IndexOfFromEnd(InFileName, '.');
	int PathSeperatorIndex = IndexOfFromEnd(InFileName, '/');
	if (DotIndex < PathSeperatorIndex || DotIndex == -1) {
		// The dot we found was part of the file path.
		// Or we didn't find a dot.
		DotIndex = strlen(InFileName);
	}

	int AutoFileNameLength = DotIndex + strlen(PostFix) + 1;
	char *AutoFileName = calloc(AutoFileNameLength, sizeof(char));

	snprintf(AutoFileName, AutoFileNameLength, "%.*s%s", DotIndex, InFileName, PostFix);

	return AutoFileName;
}

translation_scope inline void PrintHelp(char *ApplicationName) {
	const char* HelpMessage =
		"Usage: %s <InFileName> [Output Options]\n"
		"Where [Output Options] can be any combination of:\n"
		"  --logisim [FileName] ==> Outputs Logisim rom image at [FileName], or if blank <InFileName>.LogisimImage\n"
		"  --rawhex [FileName] ==> Outputs a file containing the raw hex for the program at [FileName], or if blank <InFileName>.hex\n"
		"  --symboltable [FileName] ==> Outputs a file containing a symbol table for the program at [FileName], or if blank <InFileName>.sym\n"
		"  --listing [FileName] ==> Outputs a file containing a listing for the program at [FileName], or if blank <InFileName>.lst\n";

	printf(HelpMessage, ApplicationName);
}

int main(int argc, char *argv[], char *envp[]) {
	FILE *InFile = 0, *OutLogisim = 0, *OutHex = 0, *OutSymbolTable = 0, *OutListing = 0;
	char *InFileName = 0, *OutLogisimPath = 0, *OutHexPath = 0, *OutSymbolTablePath = 0, *OutListingPath = 0;
	int GenLogisim = FALSE, GenHex = FALSE, GenSymbolTable = FALSE, GenListing = FALSE;
	uint64_t InFileSize = 0;
	int Success = TRUE;

	for (int Index = 1; Index < argc;) {
		char * Arg = argv[Index];

		if (StartsWith(Arg, "--logisim")) {
			if (Index + 1 >= argc) { Arg = ""; }
			else { Arg = argv[Index + 1]; }

			if (OutLogisim == 0 && GenLogisim == FALSE) {
				if (!((StartsWith(Arg, "--")) || (Arg[0] == 0))) {
					Index++;
					OutLogisim = fopen(Arg, "w");
					if (OutLogisim == 0) {
						fprintf(stderr, "I could not open the Logisim output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
					OutLogisimPath = Arg;
				}
				else {
					GenLogisim = TRUE;
				}
			}
			else {
				fprintf(stderr, "Option --logisim was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, "--rawhex")) {
			if (Index + 1 >= argc) { Arg = ""; }
			else { Arg = argv[Index + 1]; }

			if (OutHex == 0 && GenHex == FALSE) {
				if (!((StartsWith(Arg, "--")) || (Arg[0] == 0))) {
					Index++;
					OutHex = fopen(Arg, "wb");
					if (OutHex == 0) {
						fprintf(stderr, "I could not open the raw hex output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
					OutHexPath = Arg;
				}
				else {
					GenHex = TRUE;
				}
			}
			else {
				fprintf(stderr, "Option --rawhex was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, "--symboltable")) {
			if (Index + 1 >= argc) { Arg = ""; }
			else { Arg = argv[Index + 1]; }

			if (OutSymbolTable == 0 && GenSymbolTable == FALSE) {
				if (!((StartsWith(Arg, "--")) || (Arg[0] == 0))) {
					Index++;
					OutSymbolTable = fopen(Arg, "w");
					if (OutSymbolTable == 0) {
						fprintf(stderr, "I could not open the symbol table output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
					OutSymbolTablePath = Arg;
				}
				else {
					GenSymbolTable = TRUE;
				}
			}
			else {
				fprintf(stderr, "Option --symboltable was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, "--listing")) {
			if (Index + 1 >= argc) { Arg = ""; }
			else { Arg = argv[Index + 1]; }

			if (OutListing == 0 && GenListing == FALSE) {
				if (!((StartsWith(Arg, "--")) || (Arg[0] == 0))) {
					Index++;
					OutListing = fopen(Arg, "w");
					if (OutListing == 0) {
						fprintf(stderr, "I could not open the listing output file \"%s\" for writing!\n", Arg);
						Success = FALSE;
						break;
					}
					OutListingPath = Arg;
				}
				else {
					GenListing = TRUE;
				}
			}
			else {
				fprintf(stderr, "Option --listing was provided twice!\n");
				Success = FALSE;
				break;
			}
		}
		else if (StartsWith(Arg, "--")) {
			fprintf(stderr, "Unknown commandline operation encountered: \"%s\"\n", Arg);
			Success = FALSE;
			break;
		}
		else if (InFile == 0) { // This must be our one input file.
			InFile = fopen(Arg, "rb");
			if (InFile == 0) {
				fprintf(stderr, "I could not open the input file \"%s\" for reading!\n", Arg);
				Success = FALSE;
				break;
			}
			if (Success) {
				InFileSize = GetFileSize(Arg, &Success);
				InFileName = Arg;
			}
		}
		else {
			fprintf(stderr, "There can only be one input file, but more than one was provided!\nSecond input file path: \"%s\"\n", Arg);
			Success = FALSE;
			break;
		}

		Index++;
	}

	if (InFile == 0) {
		fprintf(stderr, "No input file was provided!\n");
		Success = FALSE;
	}

	if (Success) {
		if (GenLogisim) {
			char *AutoFileName = GenerateOutputPath(InFileName, ".LogisimImage");
			OutLogisim = fopen(AutoFileName, "w");
			if (OutLogisim == 0) {
				fprintf(stderr, "I could not open the Logisim output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
				Success = FALSE;
				free(AutoFileName);
			}
			else {
				OutLogisimPath = AutoFileName;
			}
		}
		if (GenHex) {
			char *AutoFileName = GenerateOutputPath(InFileName, ".hex");
			OutHex = fopen(AutoFileName, "wb");
			if (OutHex == 0) {
				fprintf(stderr, "I could not open the raw hex output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
				Success = FALSE;
				free(AutoFileName);
			}
			else {
				OutHexPath = AutoFileName;
			}
		}
		if (GenListing) {
			char *AutoFileName = GenerateOutputPath(InFileName, ".lst");
			OutListing = fopen(AutoFileName, "w");
			if (OutListing == 0) {
				fprintf(stderr, "I could not open the listing output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
				Success = FALSE;
				free(AutoFileName);
			}
			else {
				OutListingPath = AutoFileName;
			}
		}
		if (GenSymbolTable) {
			char *AutoFileName = GenerateOutputPath(InFileName, ".sym");
			OutSymbolTable = fopen(AutoFileName, "w");
			if (OutSymbolTable == 0) {
				fprintf(stderr, "I could not open the symbol table output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
				Success = FALSE;
				free(AutoFileName);
			}
			else {
				OutSymbolTablePath = AutoFileName;
			}
		}
	}

	if (Success) {
		Success = ApplicationMain(InFile, InFileSize, OutLogisim, OutHex, OutSymbolTable, OutListing);
	}
	else {
		printf("Exiting without invoking the assembler.\n");
		printf("---------------------------------------\n");
		PrintHelp(argv[0]);

		if (OutLogisim) {
			fclose(OutLogisim);
			Assert(OutLogisimPath != 0);
			remove(OutLogisimPath);
		}
		if (OutSymbolTable) {
			fclose(OutSymbolTable);
			Assert(OutSymbolTablePath != 0);
			remove(OutSymbolTablePath);
		}
		if (OutHex) {
			fclose(OutHex);
			Assert(OutHexPath != 0);
			remove(OutHexPath);
		}
		if (OutListing) {
			fclose(OutListing);
			Assert(OutListingPath != 0);
			remove(OutListingPath);
		}

	}
	// Return 0 on success because 1 is generally interpreted as an error, so if you were
	// to use it in a script and have a line to exit if the assembler fails like so
	//
	// MarieAssembler --rawhex prog.hex prog.MarieAsm || echo "Assembler failed!" && exit 1
	//
	// It would exit
	return Success ? 0 : 1;
}
