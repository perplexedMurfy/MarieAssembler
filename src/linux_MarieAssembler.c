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
#include "Memory_MarieAssembler.h"

#include "Platform_MarieAssembler.h"

void Platform_Breakpoint() {
	raise(SIGINT);
}

size_t Platform_GetFileSize(char *FileName, int *Success) {
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

	for(; String[Result] != '\0'; Result--) {
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
	int PathSeperatorIndex = Max(IndexOfFromEnd(InFileName, '/'), IndexOfFromEnd(InFileName, '/'));
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

void PrintHelp(char *ApplicationName);

inline void PrintHelp(char *ApplicationName) {
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
	int option_index = 0;
	int c;

	static struct option long_options[] = {
		{"logisim",		optional_argument, 0, 1},
		{"rawhex",		optional_argument, 0, 2},
		{"symboltable", optional_argument, 0, 3},
		{"listing",		optional_argument, 0, 4},
		{0, 0, 0, 0}
	};

	char *Arg;

	// Input file is before arguments
	char firstTwo[2];
	strncpy(firstTwo, argv[optind], 2);
	if (strcmp("--", firstTwo) != 0) {
		Arg = argv[optind];
		InFileName = Arg;
		InFile = fopen(InFileName, "r");
		if (InFile == 0) {
			int err = errno;
			fprintf(stderr, "I could not open the input file \"%s\" for reading!\n%s\n", Arg, strerror(err));
			Success = FALSE;
		}
		optind += 1;
	}

	while (1) {
		option_index = 0;
		c = getopt_long(argc, argv, "", long_options, &option_index);

		// Detect end of options
		if (c == -1) {
			break;
		}

		switch (c) {
			case 1:
				GenLogisim = TRUE;
				OutLogisimPath = optarg;
				if (OutListingPath != NULL) {
					OutLogisim = fopen(OutLogisimPath, "w");
					if (OutLogisim == NULL) {
						int err = errno;
						fprintf(stderr, "Could not open Logisim output for writing\n%s\n", strerror(err));
						Success = FALSE;
					}
				}
				break;
			case 2:
				GenHex = TRUE;
				OutHexPath = optarg;
				if (OutHexPath != NULL) {
					OutHex = fopen(OutHexPath, "w");
					if (OutHex == NULL) {
						int err = errno;
						fprintf(stderr, "Could not open hex output for writing\n%s\n", strerror(err));
						Success = FALSE;
					}
				}
				break;
			case 3:
				GenSymbolTable = TRUE;
				OutSymbolTablePath = optarg;
				if (OutSymbolTablePath != NULL) {
					OutSymbolTable = fopen(OutSymbolTablePath, "w");
					if (OutSymbolTable == NULL) {
						int err = errno;
						fprintf(stderr, "Could not open symbol table for writing\n%s\n", strerror(err));
						Success = FALSE;
					}
				}
				break;
			case 4:
				GenListing = TRUE;
				OutListingPath = optarg;
				if (optarg != NULL) {
					OutListing = fopen(OutListingPath, "w");
					if (OutListing == NULL) {
						int err = errno;
						fprintf(stderr, "Could not open listing for writing\n%s\n", strerror(err));
						Success = FALSE;
					}
				}
				break;
			case '?':
				// getout_long already printed an error message
				break;
			default:
				exit(1);
		}
	}

	if (optind < argc) {
		strncpy(firstTwo, argv[optind], 2);
		if (strcmp("--", firstTwo) != 0) {
			Arg = argv[optind];
			InFileName = Arg;
			InFile = fopen(InFileName, "r");
			if (InFile == 0) {
				int err = errno;
				fprintf(stderr, "I could not open the input file \"%s\" for reading!\n%s\n", Arg, strerror(err));
				Success = FALSE;
			}
			if (optind+1 < argc) {
				strncpy(firstTwo, argv[optind+1], 2);
				if (strcmp("--", firstTwo) != 0) {
					fprintf(stderr, "Please supply only one input file!\n");
					Success = FALSE;
				}
			}
		}
	}

	if (optind == argc && InFileName == NULL) {
		fprintf(stderr, "Please supply an input file\n");
		Success = FALSE;
	}

	if (Success) {
		InFileSize = Platform_GetFileSize(Arg, &Success);
		if (GenLogisim) {
			char *AutoFileName = GenerateOutputPath(InFileName, ".LogisimImage");
			OutLogisim = fopen(AutoFileName, "w");
			if (OutLogisim == 0) {
				fprintf(stderr,"I could not open the Logisim output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
				Success = FALSE;
				free(AutoFileName);
			}
			else {
				OutLogisimPath = AutoFileName;
			}
		}
		if (GenHex) {
			char *AutoFileName = GenerateOutputPath(InFileName, ".hex");
			OutHex = fopen(AutoFileName, "w");
			if (OutHex == 0) {
				fprintf(stderr,"I could not open the raw hex output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
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
				fprintf(stderr,"I could not open the listing output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
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
				fprintf(stderr,"I could not open the symbol table output file's auto-generated path \"%s\" for writing!\n", AutoFileName);
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
	} else {
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

	// If you return 1, the shell will consider the program failed, which
	// we don't want if it didn't actually fail.
	return Success ? 0 : 1;
}
