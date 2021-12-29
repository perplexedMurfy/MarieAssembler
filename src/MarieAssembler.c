/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * File: Contains the core of the assembler
 */

/*TODO LIST
 * - identifier_source.Length and identifier_dest.Length member repersents the the length in bytes of the identifier. Normally we're able to use the length in bytes as the length of characters and it's fine, but when we process a identifier with a multibyte unicode character using the length in bytes will result in incorrect printouts.
 * - Replace the fixed size arena with a growable arena.
 * - Remove all @ErrorReporting tags. Students will use this, we want to be as nice to them as possible!
 * - Make keyword match case insensitve.
 */

#include "MarieAssembler.h"
#include "Platform_MarieAssembler.h"

#include "Memory_MarieAssembler.c"

#include <stdio.h>
#include <stdarg.h>

// Contains the assembled Marie program
global_var uint16_t Program[Kilobyte(4)] = {0};
// Contains metadata regarding each Word of the program
global_var uint8_t ProgramMetaData[Kilobyte(4)] = {0};

/* Increases File->At pointer by Count characters.
 * This function also keeps File->Column and File->Line.
 */
void IncrementFilePosition(file_state *File, int Count) {
	Assert(Count >= 0);
	
	for (; Count != 0; Count--) {
		if ((File->At[0] & 0xD800) == 0xD800) {
			File->Column  -= 1;
		}
		File->At += 1;
		File->Column += 1;
		if (File->At[0] == '\n') {
			File->Column = 0;
			File->Line += 1;
		}
	}
}

/* Advances File->At past all whitespace and comments.
 */
void AdvancePastWhitespaceAndComments(file_state *File) {
	while (File->At[0] == L' '  ||
	       File->At[0] == L'\n' ||
	       File->At[0] == L'\r' ||
	       File->At[0] == L'\t' ||
	       File->At[0] == L'/') {
	       
		if (File->At[0] == L'/') {
			while (File->At[0] != L'\n' &&
			       File->At[0] != L'\0') {
				IncrementFilePosition(File, 1);
			}
			if (File->At[0] == L'\n') {
				IncrementFilePosition(File, 1);
			}
		}
		else {
			IncrementFilePosition(File, 1);
		}
	}
}

/* Advances File->At past whitespace on the same line.
 */
void AdvancePastWhitespaceOnSameLine(file_state *File) {
	while (File->At[0] == L' '  ||
	       File->At[0] == L'\r' ||
	       File->At[0] == L'\t') {
		IncrementFilePosition(File, 1);
	}
}

/* if File->At does not point to the beginning of a number of the form `0d0000`, then this function returns false and Result is set to 0.
 * if File->At does point to the beginning of a number, then this function returns true and Result will have the value of that number.
 */
int ExtractNumberDecimal(file_state *File, int *Result) {
	int Success = FALSE;
	int sign = 1;
	*Result = 0;

	if (File->At[0] == L'0' && File->At[1] == L'd') {
		IncrementFilePosition(File, 2);
		// @TODO this loop could be writen better...
		while ((*(File->At) >= L'0' && *(File->At) <= L'9') ||
		       (*(File->At) == '-') ||
		       (*(File->At) == '+')) {
			Success = TRUE; // we started to eat something that initally looks like a number...

			if (*(File->At) == '-') {
				sign = -1;
			}

			else if (*(File->At) == '+') {
				sign = +1;
			}

			else if (*(File->At) >= L'0' && *(File->At) <= L'9') {
				if (Result != 0) {
					*Result *= 10;
				}
		
				*Result += *(File->At) - L'0';
			}
		
			IncrementFilePosition(File, 1);
		}
	}

	*Result *= sign;
	return Success;
}


/* if File->At does not point to the beginning of a number of the form `0x0000`, then this function returns false and Result is set to 0.
 * if File->At does point to the beginning of a number, then this function returns true and Result will have the value of that number.
 */
int ExtractNumberHexadecimal(file_state *File, int *Result) {
	int Success = FALSE;
	int sign = 1;
	*Result = 0;

	if (File->At[0] == L'0' && File->At[1] == L'x') {
		IncrementFilePosition(File, 2);
		// @TODO this loop could be writen better...
		while ((File->At[0] >= L'0' && File->At[0] <= L'9') ||
		       (File->At[0] >= L'A' && File->At[0] <= L'F') ||
		       (File->At[0] >= L'a' && File->At[0] <= L'f')
		       ) {
			Success = TRUE; // we started to eat something that initally looks like a number...

			if (Result != 0) {
				*Result *= 0x10;
			}

			int Value = -1;
		
			if (File->At[0] >= L'0' && File->At[0] <= L'9') {
				Value = File->At[0] - L'0';
			}
			else if (File->At[0] >= L'A' && File->At[0] <= L'F') {
				Value = File->At[0] - L'A' + 0xA;
			}
			else if (File->At[0] >= L'a' && File->At[0] <= L'f') {
				Value = File->At[0] - L'a' + 0xA;
			}

			*Result += Value;
		
			IncrementFilePosition(File, 1);
		}
	}
	
	return Success;
}

/* If File->At points to a charcter that can be the beginning of a identifier, then this function returns True, File->At is advnaced past the identifier, and the length of the identifier is returned though Length. 
 * If File->At does not point to a character that can begin an identifier, then 
 */
int ExtractIdentifier(file_state *File, int *Length) {
	int Success = FALSE;
	if (!(File->At[0] >= L'0' && File->At[0] <= L'9')) {
		Success = TRUE;
		(*Length)++;
		IncrementFilePosition(File, 1);
		while ((File->At[0] != L' ') &&
		       (File->At[0] != L'\n') &&
		       (File->At[0] != L'\r') &&
		       (File->At[0] != L'\t') &&
		       (File->At[0] != L'\0')) {
			(*Length)++;
			IncrementFilePosition(File, 1);
		}
	}
	return Success;
}

/* If File->At points to the beginning of a keyword, then this function returns true, the length of the keyword is returned though Length.
 * else, this function returns false, and Length will be set to 0.
 */
int PeekKeyword(file_state *File, int *Length) {
	int Success = FALSE;
	*Length = 0;
	if ((File->At[*Length] >= L'a' && File->At[*Length] <= L'z') ||
	    (File->At[*Length] >= L'A' && File->At[*Length] <= L'Z') ||
	    (File->At[*Length] == L'.')) {
		Success = TRUE;
		(*Length)++;
		while ((File->At[*Length] >= L'a' && File->At[*Length] <= L'z') ||
		       (File->At[*Length] >= L'A' && File->At[*Length] <= L'Z') ||
		       (File->At[*Length] == L'.')){
			(*Length)++;
		}
	}
	return Success;
}

// Assumess that null charcters terminate a string.
int CompareStr(wchar_t *A, wchar_t *B, uint64_t Len) {
	int Result = FALSE;
	uint64_t Index = 0;
	while (Index < Len && (Result = (A[Index] == B[Index])) != 0) {
		Index++;
		if (A[Index] == L'\0' || B[Index] == L'\0') { break; }
	}
	return Result;
}

inline int CompareStrToKeyword(wchar_t *Str, int FileKeywordLength, const keyword_entry Keyword) {
	if (FileKeywordLength != Keyword.Length) {
		return FALSE;
	}
	else {
		return CompareStr(Str, Keyword.String, Keyword.Length);
	}
}

/* If ConditionOfFailure is true, then DidErrorOccur is set to true, and vwprintf_s is called with the format string and the VarArg list passed to this function.
 */
void ReportErrorConditionally(int ConditionOfFailure, int *DidErrorOccur, const wchar_t *FormatString, ...) {
	if (ConditionOfFailure) {
		if (DidErrorOccur) {
			*DidErrorOccur = TRUE;
		}
		va_list VarArgsList;
		va_start(VarArgsList,FormatString);
		vwprintf(FormatString, VarArgsList);
		va_end(VarArgsList);
	}
}

inline int WriteProgramData(file_state *File, uint16_t Data, int CurrentAddress, uint8_t ProgramMetaDataFlags) {
	int Success = FALSE;
	ReportErrorConditionally(ProgramMetaData[CurrentAddress] & PMD_IsOccupied, &Success, L"[Error L:%d C:%d] An instruction overlapped another instruction! Pay mind to your usage of .SetAddr\n", File->Line, File->Column);
	Program[CurrentAddress] = Data;
	ProgramMetaData[CurrentAddress] |= ProgramMetaDataFlags;
	
	return Success;
}

int Assemble(file_state *File, paged_list *IdentifierDestinationList, paged_list *IdentifierSourceList) {
	int DidErrorOccur = FALSE;
	int ToIncrementAddress = FALSE;
	int LastLineOperationWasProcessed = -1;

	int CurrentAddress = 0;
	
	while(!DidErrorOccur) {
		AdvancePastWhitespaceAndComments(File);
		if (ToIncrementAddress == TRUE) {
			CurrentAddress++;
			ToIncrementAddress = FALSE;
		}
		if (File->At[0] == L'\0') { break; } // we reached the end of the file, no more parsing to be done.
		ReportErrorConditionally(CurrentAddress < 0 || CurrentAddress > 0xfff, &DidErrorOccur, L"[Error] The CurrentAddress (%X) is less than 0 or greater than 0xfff. This was likely caused by a .SetAddress that was too high, or if there are more than 4095 instructions in this program. This program was at Line %d, Column %d when this error was caught.\nTerminateing Assembly...", CurrentAddress, File->Line, File->Column);
		if (DidErrorOccur) { break; }

		int KeywordIndex = 0;
		int KeywordLength = 0;
		ReportErrorConditionally(PeekKeyword(File, &KeywordLength) == FALSE, &DidErrorOccur, L"[Error L:%d C:%d] Failed to find a keyword\n");
		for (; KeywordIndex < KW_COUNT; KeywordIndex++) {
			if (CompareStrToKeyword(File->At, KeywordLength, Keywords[KeywordIndex])) {
				break;
			}
			else if (KeywordIndex + 1 == KW_COUNT) { KeywordIndex++; } // If we fail on the last keyword, take on a bogus value.
		}

		switch(KeywordIndex) {

		case(KW_Jumpstore):
		case(KW_Jump):
		case(KW_Jumpi):
		case(KW_Load):
		case(KW_Loadi):
		case(KW_Store):
		case(KW_Storei):
		case(KW_Add):
		case(KW_Addi):
		case(KW_Sub): {
			// KEYWORD [Addr|Identifier]
			
			IncrementFilePosition(File, Keywords[KeywordIndex].Length);
			AdvancePastWhitespaceOnSameLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;

			// @TODO Allow 0d numbers as addresses
			int Address = 0;
			identifier_dest IdentifierDest = {.Start = File->At, .Address = CurrentAddress, .Line = File->Line, .Column = File->Column};
			if (ExtractNumberHexadecimal(File, &Address)) {
				ReportErrorConditionally(Address > 0xFFF || Address < 0, &DidErrorOccur, L"[Error L:%d C:%d] The Address provided (0x%X) was not between 0x0 and 0xFFF.\n", File->Line, File->Column, Address);
				WriteProgramData(File, Keywords[KeywordIndex].Opcode | Address, CurrentAddress, PMD_IsOccupied);
			}
			else if (ExtractIdentifier(File, &IdentifierDest.Length)) {
				AddToPagedList(IdentifierDestinationList, &IdentifierDest);
				WriteProgramData(File, Keywords[KeywordIndex].Opcode, CurrentAddress, PMD_IsOccupied | PMD_UsedIdentifier);
			}
			else {
				ReportErrorConditionally(TRUE, &DidErrorOccur, L"[Error L:%d C:%d] Failed to read an argument for %s operation. Please provide a Hex Address or a Identifier.\n", File->Line, File->Column, Keywords[KeywordIndex].String);
			}

			if (KeywordIndex == KW_Jumpstore) {
				ReportErrorConditionally(Address == 0xFFF, 0, L"[Warning L:%d C:%d] A jns instruction was provided 0xfff as a destination address. Make sure you know what you Marie Processor does when the Program Counter is > 0xFFF!\n", File->Line, File->Column);
			}
		} break;
			
		case(KW_Input):
		case(KW_Output):
		case(KW_Halt):
		case(KW_Clear): {
			// KEYWORD

			IncrementFilePosition(File, Keywords[KeywordIndex].Length);
			AdvancePastWhitespaceOnSameLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;

			WriteProgramData(File, Keywords[KeywordIndex].Opcode, CurrentAddress, PMD_IsOccupied);
		} break;

		case(KW_Skipcond): {
			// Skipcond [lesser|greater|equal|NUMBER]

			IncrementFilePosition(File, Keywords[KW_Skipcond].Length);
			AdvancePastWhitespaceOnSameLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;
			
			int RawOperation = 0;
			if (CompareStr(File->At, L"lesser", 6)) {
				IncrementFilePosition(File, 6);
				RawOperation = 0x000;
			}
			else if (CompareStr(File->At, L"equal", 5)) {
				IncrementFilePosition(File, 5);
				RawOperation = 0x400;
			}
			else if (CompareStr(File->At, L"greater", 7)) {
				IncrementFilePosition(File, 7);
				RawOperation = 0xC00;
			}
			else if (ExtractNumberHexadecimal(File, &RawOperation)) {
			}
			else {
				ReportErrorConditionally(TRUE, &DidErrorOccur, L"[Error L:%d C:%d] Failed to read an argument for Skipcond operation. Please provide either a named operation (\"lesser\", \"equal\", or \"greater\") or the raw operation value (0x000, 0x400, 0xC000 respectively).\n", File->Line, File->Column);
			}
			const int DidFail = RawOperation != 0x000 && RawOperation != 0x400 && RawOperation != 0xC00;
			ReportErrorConditionally(DidFail, 0, L"[Warning L:%d C:%d] The Operation provided (0x%0.3X) was not a known operation. We will continue to assemble this program but know that this skipcond instruction may have unintended behaivor!\nKnown operation constants are lesser (0x000), equal (0x400), or greater (0xC00)\n", File->Line, File->Column, RawOperation);
			WriteProgramData(File, Keywords[KeywordIndex].Opcode | RawOperation, CurrentAddress, PMD_IsOccupied);
		} break;

		case(KW_M_SetAddr): {
			// .SetAddr [Addr]

			IncrementFilePosition(File, Keywords[KW_M_SetAddr].Length);
			AdvancePastWhitespaceOnSameLine(File);
			
			ReportErrorConditionally(ExtractNumberHexadecimal(File, &CurrentAddress) == FALSE, &DidErrorOccur, L"[Error L:%d C:%d] Unable to Extract a Hexadecimal Number for .SetAddr", File->Line, File->Column);

			ReportErrorConditionally(CurrentAddress > 0xFFF || CurrentAddress < 0, &DidErrorOccur, L"[Error L:%d C:%d] The Address provided (%x) was not between 0x0 and 0xfff.\n", File->Line, File->Column, CurrentAddress);
		} break;

		case(KW_M_Ident): {
			// .Ident [Identifier]

			IncrementFilePosition(File, Keywords[KW_M_Ident].Length);
			AdvancePastWhitespaceOnSameLine(File);
			
			identifier_source Data = {.Start = File->At, .Value = CurrentAddress - 1, .Line = File->Line, .Column = File->Column};

			ReportErrorConditionally(LastLineOperationWasProcessed != File->Line, &DidErrorOccur, L"[Error L:%d C:%d] Identifiers must follow right after a operation on the same line.\nEx: data 0d0 .Ident Foo\nIf you don't like it, change it!\n", File->Line, File->Column); 
			ReportErrorConditionally(ExtractIdentifier(File, &Data.Length) == FALSE, &DidErrorOccur, L"[Error L:%d C:%d] Failed to find an Identifier Name after .Ident!\n", File->Line, File->Column);

			for (int Index = 0; ; Index++) {
				const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, Index);
				if (IdentifierSource == 0) { break; }
		
				if (Data.Length == IdentifierSource->Length &&
				    CompareStr(Data.Start, IdentifierSource->Start, Data.Length)) {
					ReportErrorConditionally(TRUE, &DidErrorOccur, L"[Error L:%d C:%d] Identifier \"%.*s\" was redefined!", Data.Length, Data.Start);
				}
			}
			
			ProgramMetaData[CurrentAddress - 1] |= PMD_DefinedIdentifier;
			// .Value is CurrentAddress - 1 because that was the address of the last instruction that was processed. Thanks to the following checks, we can be sure that we're refering to the instruction that was immeatly preceeded this .Ident.
			
			AddToPagedList(IdentifierSourceList, &Data);
		} break;

		case(KW_Data): {
			// Data [NUMBER]

			IncrementFilePosition(File, Keywords[KW_Data].Length);
			AdvancePastWhitespaceOnSameLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;
			int Value = 0;
			
			if (ExtractNumberDecimal(File, &Value)) {
			}
			else if (ExtractNumberHexadecimal(File, &Value)) {
			}
			else {
				ReportErrorConditionally(TRUE, &DidErrorOccur, L"[Error L:%d C:%d] Failed to read an argument for the Data directive. Please provide a number constant within 0 - 65535 (0x0 - 0xffff).\n", File->Line, File->Column);
			}
			ReportErrorConditionally(Value < 0 || Value > 0xffff, &DidErrorOccur, L"[Error L:%d C:%d] Invalid argument for the Data directive. Please provide a number constant within 0 - 65535 (0x0 - 0xffff).\n", File->Line, File->Column);
			DidErrorOccur = WriteProgramData(File, Value, CurrentAddress, PMD_IsOccupied | PMD_IsData);
		} break;

		default: {
			ReportErrorConditionally(TRUE, &DidErrorOccur, L"[Error L:%d C:%d] \"%.*s\" is not a valid keyword.\n", File->Line, File->Column, KeywordLength, File->At);
		}
			
		}		         
	}

	// resolve identifiers

	if (!DidErrorOccur) {
		for (int DestIndex = 0; ; DestIndex++) {
			const identifier_dest *IdentifierDest = GetFromPagedList(IdentifierDestinationList, DestIndex);
			if (IdentifierDest == 0) { break; }

			DidErrorOccur = TRUE;
			for (int SourceIndex = 0; ; SourceIndex++) {
				const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, SourceIndex);
				if (IdentifierSource == 0) { break; }
				
				if (IdentifierDest->Length == IdentifierSource->Length) {
					if (CompareStr(IdentifierDest->Start, IdentifierSource->Start, IdentifierDest->Length)) {
						Assert(IdentifierSource->Value <= 0xfff); // I'm pretty sure this should never be possible.
						Program[IdentifierDest->Address] |= IdentifierSource->Value;
						DidErrorOccur = FALSE;
						break; // we can't match more than once.
					}
				}
			}
			if (DidErrorOccur) {
				// @TODO we this should output a line and a column
				wprintf(L"[Error L:%d C:%d] Identifier \"%.*s\" was never defined!\n", IdentifierDest->Line, IdentifierDest->Column, IdentifierDest->Length, IdentifierDest->Start);
				break;
			}
		}
	}

	return !DidErrorOccur;
}

int fprintfCheck(int *Success, FILE *File, char *FormatStr, ...) {
	va_list ArgList;
	va_start(ArgList, FormatStr);

	int Result = vfprintf(File, FormatStr, ArgList);
	if (Result < 0) { *Success = FALSE; }
	
	va_end(ArgList);
	
	if (!Success) {
		return Result = 0;
	}
	return Result;
}

int fwprintfCheck(int *Success, FILE *File, wchar_t *FormatStr, ...) {
	va_list ArgList;
	va_start(ArgList, FormatStr);

	int Result = vfwprintf(File, FormatStr, ArgList);
	if (Result < 0) { *Success = FALSE; }
	
	va_end(ArgList);

	if (!Success) {
		return Result = 0;
	}
	return Result;
}

int swprintfCheck(int *Success, wchar_t *Buffer, size_t Size, wchar_t *FormatStr, ...) {
	va_list ArgList;
	va_start(ArgList, FormatStr);

	int Result = vswprintf(Buffer, Size, FormatStr, ArgList);
	if (Result < 0) { *Success = FALSE; }
	
	va_end(ArgList);

	if (!Success) {
		return Result = 0;
	}
	return Result;
}

int OutputLogisimImage(FILE *FileStream) {
	int Success = TRUE;
	fprintfCheck(&Success, FileStream, "v2.0 raw\r\n");

	int Consectuive = 0;
	int Prev = Program[0];
	for (int Index = 0; Index <= Kilobyte(4) && Success; Index++) {
		if (Prev == Program[Index] && Index != Kilobyte(4)) {
			Consectuive++;
		}
		else {
			char Buffer[16] = {0};
			if (Consectuive == 1) {
				fprintfCheck(&Success, FileStream, "%X\n", Prev);
			}
			else {
				fprintfCheck(&Success, FileStream, "%d*%X\n", Consectuive, Prev);
			}
			Consectuive = 1;
		}
		Prev = Program[Index];
	}

	fclose(FileStream);

	if (Success == FALSE) {
		printf("[Error Logisim] There was a error encountered while writing to the Logisim output file!\n");
	}
	
	return Success;
}

int OutputRawHex(FILE *FileStream) {
	int Result = fwrite(Program, sizeof(Program), 1, FileStream);
	int Success = Result == 1;
	
	fclose(FileStream);

	if (Success == FALSE) {
		printf("[Error Raw Hex] There was a error encountered while writing to the hex output file!\n");
	}

	return Success;
}

int OutputSymbolTable(FILE *FileStream, paged_list *IdentifierDestinationList, paged_list *IdentifierSourceList) {
	int IdentifierMaxLength = 0;
	int Success = TRUE;

	for (int Index = 0; ; Index++) {
		const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, Index);
		if (IdentifierSource == 0) { break; }
		
		if (IdentifierSource->Length > IdentifierMaxLength) {
			IdentifierMaxLength = IdentifierSource->Length;
		}
	}

	IdentifierMaxLength = Max(IdentifierMaxLength, 10);

	fwprintfCheck(&Success, FileStream, L"| %- *s | Identifier's Value | Addresses that use Identifier\n", IdentifierMaxLength, L"Identifier");
	for (int SourceIndex = 0; Success; SourceIndex++) {
		const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, SourceIndex);
		if (IdentifierSource == 0) { break; }
		
		fwprintfCheck(&Success, FileStream, L"| %- *.*s | 0x%-0*.3X | ", IdentifierMaxLength, IdentifierSource->Length, IdentifierSource->Start, 18 - 2, IdentifierSource->Value);
		
		for (int DestIndex = 0; Success; DestIndex++) {
			const identifier_dest *IdentifierDest = GetFromPagedList(IdentifierDestinationList, DestIndex);
			if (IdentifierDest == 0) { break; }
			
			if ((IdentifierSource->Length == IdentifierDest->Length) &&
			    CompareStr(IdentifierSource->Start, IdentifierDest->Start, IdentifierSource->Length)) {
				fwprintfCheck(&Success, FileStream, L" 0x%-0.3X", IdentifierDest->Address);
			}
		}
		fwprintfCheck(&Success, FileStream, L"\n");
	}
	fclose(FileStream);

	if (Success == FALSE) {
		printf("[Error Symbol Table] There was a error encountered while writing to the symbol table output file!\n");
	}

	return Success;
}

int OutputListing(FILE *FileStream, paged_list *IdentifierDestinationList, paged_list *IdentifierSourceList) {
	int Success = TRUE;
	int InMemoryGap = FALSE;

	// @TODO make this growable!!! Someone someday will be really mad at me for limiting the size of this string.
	wchar_t *ContentsOfAC = calloc(5000, sizeof(wchar_t));
	int ContentsOfACSize = 5000;
	ContentsOfAC[0] = L'0';
	ContentsOfAC[1] = L'\0';
	int EmitCode = EMIT_No;
	int EmitIndentNextLine = FALSE;

	int OperandMaxLength = wcslen(L"greater"); // "greater" is the longest literal operand, as a argument to skipcond.
	for (int Index = 0; ; Index++) {
		const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, Index);
		if (IdentifierSource == 0) { break; }
		
		if (IdentifierSource->Length > OperandMaxLength) {
			OperandMaxLength = IdentifierSource->Length;
		}
	}

	//  + 1 + is repersentive of spaces.
	// Max length is "skipcond LongstIdentifier .Ident LongestIdentifier"
	int ListingMaxLength = Keywords[KW_Skipcond].Length + 1 + OperandMaxLength + 1 + Keywords[KW_M_Ident].Length + 1 + OperandMaxLength + 1;
	
	fwprintfCheck(&Success, FileStream, L"| Address | Opcode | %- *s | High Level Code\n", ListingMaxLength, L"Listing");
	
	for (int Index = 0; Index < Kilobyte(4) && Success; Index++) {
		int ListingCharacterCount = 0;
		if (ProgramMetaData[Index] & PMD_IsOccupied) {
			if (InMemoryGap) {
				InMemoryGap = FALSE;
				Assert(ListingMaxLength >= 14);
				fwprintfCheck(&Success, FileStream, L"|         |        | .SetAddr 0x%0.3X% *s |\n", Index, ListingMaxLength - 14, L"");
			}

			fwprintfCheck(&Success, FileStream, L"| 0x%0.3X   | 0x%0.4X | ", Index, Program[Index]);

			identifier_dest *IdentifierDestination = 0;
			if (ProgramMetaData[Index] & PMD_UsedIdentifier) {
				for (int IdentifierIndex = 0; ; IdentifierIndex++) {
					IdentifierDestination = GetFromPagedList(IdentifierDestinationList, IdentifierIndex);
							
					if (IdentifierDestination == 0) {
						wprintf(L"[Error Lising] Failed to resolve an Identifier used at  0x%0.3X\n", Index);
						break;
					}
					if (IdentifierDestination->Address == Index) {
						break;   
					}
				}
			}


			if (ProgramMetaData[Index] & PMD_IsData) {
				ListingCharacterCount += fwprintfCheck(&Success, FileStream, L"data 0x%0.4X", Program[Index]);
			}
			else {
				wchar_t *OpcodeMemonic = 0;
				const uint16_t Opcode = Program[Index] & 0xF000;
				if (Opcode == Keywords[KW_Jumpstore].Opcode) {
					OpcodeMemonic = Keywords[KW_Jumpstore].String;
					EmitCode = EMIT_Jumpstore;
				}
				else if (Opcode == Keywords[KW_Load].Opcode) {
					OpcodeMemonic = Keywords[KW_Load].String;
					EmitCode = EMIT_No;
					if (IdentifierDestination) {
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[0x%0.3X]/", Program[Index] & 0x0FFF);
					}
				}
				else if (Opcode == Keywords[KW_Store].Opcode) {
					OpcodeMemonic = Keywords[KW_Store].String;
					EmitCode = EMIT_Store;
				}
				else if (Opcode == Keywords[KW_Add].Opcode) {
					OpcodeMemonic = Keywords[KW_Add].String;
					EmitCode = EMIT_No;
					if (ContentsOfAC[0] == L'0' && ContentsOfAC[1] == L'\0') {
						if (IdentifierDestination) {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[0x%0.3X]", Program[Index] & 0x0FFF);
						}
					}
					else {
						if (IdentifierDestination) {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%s + %.*s", ContentsOfAC, IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%s + RAM[0x%0.3X]", ContentsOfAC, Program[Index] & 0x0FFF);
						}
					}
				}
				else if (Opcode == Keywords[KW_Sub].Opcode) {
					OpcodeMemonic = Keywords[KW_Sub].String;
					EmitCode = EMIT_No;
					if (ContentsOfAC[0] == L'0' && ContentsOfAC[1] == L'\0') {
						if (IdentifierDestination) {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"-%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"-RAM[0x%0.3X]", Program[Index] & 0x0FFF);
						}
					}
					else {
						if (IdentifierDestination) {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%s - %.*s", ContentsOfAC, IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%s - RAM[0x%0.3X]", ContentsOfAC, Program[Index] & 0x0FFF);
						}
					}
				}
				else if (Opcode == Keywords[KW_Input].Opcode) {
					OpcodeMemonic = Keywords[KW_Input].String;
					EmitCode = EMIT_No;
					swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"Input");
				}
				else if (Opcode == Keywords[KW_Output].Opcode) {
					OpcodeMemonic = Keywords[KW_Output].String;
					EmitCode = EMIT_Output;
				}
				else if (Opcode == Keywords[KW_Halt].Opcode) {
					OpcodeMemonic = Keywords[KW_Halt].String;
					EmitCode = EMIT_Halt;
				}
				else if (Opcode == Keywords[KW_Skipcond].Opcode) {
					OpcodeMemonic = Keywords[KW_Skipcond].String;
					EmitCode = EMIT_Skipcond;
				}
				else if (Opcode == Keywords[KW_Jump].Opcode) {
					OpcodeMemonic = Keywords[KW_Jump].String;
					EmitCode = EMIT_Jump;
				}
				else if (Opcode == Keywords[KW_Clear].Opcode) {
					OpcodeMemonic = Keywords[KW_Clear].String;
					EmitCode = EMIT_Clear;
					swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"0");
				}
				else if (Opcode == Keywords[KW_Jumpi].Opcode) {
					OpcodeMemonic = Keywords[KW_Jumpi].String;
					EmitCode = EMIT_Jumpi;
				}
				else if (Opcode == Keywords[KW_Addi].Opcode) {
					OpcodeMemonic = Keywords[KW_Addi].String;
					EmitCode = EMIT_No;
					if (ContentsOfAC[0] == L'0' && ContentsOfAC[1] == L'\0') {
						if (IdentifierDestination) {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[%.*s]", IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[RAM[0x%0.3X]]", Program[Index] & 0x0FFF);
						}
					}
					else {
						if (IdentifierDestination) {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%s + RAM[%.*s]", ContentsOfAC, IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%s + RAM[RAM[0x%0.3X]]", ContentsOfAC, Program[Index] & 0x0FFF);
						}
					}
				}
				else if (Opcode == Keywords[KW_Loadi].Opcode) {
					OpcodeMemonic = Keywords[KW_Loadi].String;
					EmitCode = EMIT_No;
					if (IdentifierDestination) {
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[%.*s]", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[RAM[0x%0.3X]]", Program[Index] & 0x0FFF);
					}
				}
				else if (Opcode == Keywords[KW_Storei].Opcode) {
					OpcodeMemonic = Keywords[KW_Storei].String;
					EmitCode = EMIT_Storei;
				}
				ListingCharacterCount += fwprintfCheck(&Success, FileStream, L"%s", OpcodeMemonic);

				if (OpcodeMemonic != Keywords[KW_Halt].String &&
				    OpcodeMemonic != Keywords[KW_Input].String &&
				    OpcodeMemonic != Keywords[KW_Output].String &&
				    OpcodeMemonic != Keywords[KW_Clear].String) {
					if (ProgramMetaData[Index] & PMD_UsedIdentifier) {
						Assert(IdentifierDestination != FALSE);
						ListingCharacterCount += fwprintfCheck(&Success, FileStream, L" %.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						if (OpcodeMemonic == Keywords[KW_Skipcond].String) {
							switch(Program[Index] & 0x0FFF) {
							case(0x000): ListingCharacterCount += fwprintfCheck(&Success, FileStream, L" lesser"); break;
							case(0x400): ListingCharacterCount += fwprintfCheck(&Success, FileStream, L" equal"); break;
							case(0xC00): ListingCharacterCount += fwprintfCheck(&Success, FileStream, L" greater"); break;
							default: ListingCharacterCount += fwprintfCheck(&Success, FileStream, L" 0x%0.3X", Program[Index] & 0x0FFF); break;
							}
						}
						else {
							ListingCharacterCount += fwprintfCheck(&Success, FileStream, L" 0x%0.3X", Program[Index] & 0x0FFF);
						}
					}
				}
			}
		
			if (ProgramMetaData[Index] & PMD_DefinedIdentifier) {
				for (int IdentifierIndex = 0; ; IdentifierIndex++) {
					const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, IdentifierIndex);
					if (IdentifierSource == 0) {
						fwprintfCheck(&Success, FileStream, L" .Ident COULD NOT RESOLVE IDENTIFER DEFINITION");
						wprintf(L"[Error Lising] Failed to resolve an Identifier defined at address 0x%0.3X\n", Index);
						Success = FALSE;
						break;
					}
					if (IdentifierSource->Value == Index) {
						ListingCharacterCount += fwprintfCheck(&Success, FileStream, L" .Ident %.*s", IdentifierSource->Length, IdentifierSource->Start);
						break;
					}
				}
			}
			
			fwprintfCheck(&Success, FileStream, L"% *s | ", ListingMaxLength - ListingCharacterCount, L"");

			if (EmitIndentNextLine) {
				EmitIndentNextLine = FALSE;
				fwprintfCheck(&Success, FileStream, L"    ");
			}
			
			{
				switch (EmitCode) {

				case(EMIT_No): {
				} break;
					
				case(EMIT_Jump): {
					if (IdentifierDestination) {
						fwprintfCheck(&Success, FileStream, L"Goto %.*s // 0x%0.3X", IdentifierDestination->Length, IdentifierDestination->Start, Program[Index] & 0xFFF);
					}
					else {
						fwprintfCheck(&Success, FileStream, L"Goto 0x%0.3X", Program[Index] & 0xFFF);
					}
				} break;

				case(EMIT_Jumpi): {
					if (IdentifierDestination) {
						fwprintfCheck(&Success, FileStream, L"Goto RAM[%.*s]", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						fwprintfCheck(&Success, FileStream, L"Goto RAM[0x%0.3X]", Program[Index] & 0xFFF);
					}
				} break;

				case(EMIT_Jumpstore): {
					if (IdentifierDestination) {
						fwprintfCheck(&Success, FileStream, L"%.*s = PC\n", IdentifierDestination->Length, IdentifierDestination->Start);
						fwprintfCheck(&Success, FileStream, L"|         |        | %- *s | % *sGoto (%.*s + 0x1) // (0x%0.3X + 0x1)", ListingMaxLength, L"", EmitIndentNextLine ? 0 : 4, L"", IdentifierDestination->Length, IdentifierDestination->Start, Program[Index] & 0xFFF);
					}
					else {
						fwprintfCheck(&Success, FileStream, L"0x%0.3X = PC\n", Program [Index] & 0x0FFF);
						fwprintfCheck(&Success, FileStream, L"|         |        | %- *s | Goto (0x%0.3X + 0x1)", ListingMaxLength, L"", Program[Index] & 0x0FFF);
					}
				} break;

				case(EMIT_Skipcond): {
					if ((Program[Index] & 0x0FFF) == 0xC00) { // Greater
						fwprintfCheck(&Success, FileStream, L"if ((%s) <= 0)", ContentsOfAC);
					}
					else if ((Program[Index] & 0x0FFF) == 0x400) { // Equal
						fwprintfCheck(&Success, FileStream, L"if ((%s) != 0)", ContentsOfAC);
					}
					else if ((Program[Index] & 0x0FFF) == 0x000) { // Lesser
						fwprintfCheck(&Success, FileStream, L"if ((%s) >= 0)", ContentsOfAC);
					}
					else { // Unknown
						fwprintfCheck(&Success, FileStream, L"Skip next if (unknown operation)");
					}
					EmitIndentNextLine = TRUE;
				} break;

				case(EMIT_Store): {
					if (IdentifierDestination) {
						fwprintfCheck(&Success, FileStream, L"%.*s = %s", IdentifierDestination->Length, IdentifierDestination->Start, ContentsOfAC);
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						fwprintfCheck(&Success, FileStream, L"RAM[0x%0.3x] = %s", Program[Index] & 0xFFF, ContentsOfAC);
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[0x%0.3x]", Program[Index] & 0xFFF);
					}
				} break;

				case(EMIT_Storei): {
					if (IdentifierDestination) {
						fwprintfCheck(&Success, FileStream, L"RAM[%.*s] = %s", IdentifierDestination->Length, IdentifierDestination->Start, ContentsOfAC);
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[%.*s]", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						fwprintfCheck(&Success, FileStream, L"RAM[RAM[0x%0.3X]] = %s", Program[Index] & 0xFFF, ContentsOfAC);
						swprintfCheck(&Success, ContentsOfAC, ContentsOfACSize, L"RAM[RAM[0x%0.3X]]", Program[Index] & 0xFFF);
					}
					
				} break;

				case(EMIT_Clear): {
					fwprintfCheck(&Success, FileStream, L"AC = 0");
				} break;

				case(EMIT_Output): {
					fwprintfCheck(&Success, FileStream, L"Output = %s", ContentsOfAC);
				} break;

				case(EMIT_Halt): {
					fwprintfCheck(&Success, FileStream, L"End execution");
				} break;

				default: {
					wprintf(L"[Error Listing] We Tried to emit a invalid emit code! Something is wrong with the compiler\n");
					Success = FALSE;
				} break;
				}
				EmitCode = EMIT_No;	
			}
			
			fwprintfCheck(&Success, FileStream, L"\n");
		}
		else {
			InMemoryGap = TRUE;
		}
	}
	
	fclose(FileStream);

	if (Success == FALSE) {
		printf("[Error Listing] There was a error encountered while writing to the listing output file!\n");
	}
	
	return Success;
}

wchar_t *LoadFileIntoMemory(FILE* FileStream, int FileSize, int *Success) {
	wchar_t *Result = 0;

	if (*Success) {
		uint8_t ByteOrderMark[4];
		fread(&ByteOrderMark, sizeof(uint8_t), Min(4, FileSize), FileStream);
		fseek(FileStream, 0, SEEK_SET);

		if ((FileSize >= 4) &&
		    (ByteOrderMark[0] == 0xFF) &&
		    (ByteOrderMark[1] == 0xFE) &&
		    (ByteOrderMark[2] == 0x00) &&
		    (ByteOrderMark[3] == 0x00)) { // UTF-32-LE
			wprintf(L"[Error File Handling] Little Endian UTF 32 encoding is not supported. Please use UTF 16 or UTF 8.\n");
			*Success = FALSE;       
		}
		
		else if ((FileSize >= 4) &&
		         (ByteOrderMark[0] == 0x00) &&
		         (ByteOrderMark[1] == 0x00) &&
		         (ByteOrderMark[2] == 0xFE) &&
		         (ByteOrderMark[3] == 0xFF)) { // UTF-32-BE
			wprintf(L"[Error File Handling] Big Endian UTF 32 encoding is not supported. Please use UTF 16 or UTF 8.\n");
			*Success = FALSE;
		}
		
		else if ((FileSize >= 2) &&
		         (ByteOrderMark[0] == 0xFF) &&
		         (ByteOrderMark[1] == 0xFE)) { // UTF-16-LE
			FileSize -= 2;
			if (FileSize == 0) {
				wprintf(L"[Error File Handling] This file contains no textual content!\n");
				*Success = 0;
			}
			else {
				fseek(FileStream, 2, SEEK_SET);
				Result = malloc(FileSize + sizeof(wchar_t));
				fread(Result, sizeof(uint8_t), FileSize, FileStream);
				Result[FileSize/2] = L'\0';
			}
		}
		
		else if ((FileSize >= 2) &&
		         (ByteOrderMark[0] == 0xFE) &&
		         (ByteOrderMark[1] == 0xFF)) { // UTF-16-BE
			FileSize -= 2;
			if (FileSize == 0) {
				wprintf(L"[Error File Handling] This file contains no textual content!\n");
				*Success = 0;
			}
			else {
				fseek(FileStream, 2, SEEK_SET);
				Result = malloc(FileSize + sizeof(wchar_t));
				fread(Result, sizeof(uint8_t), FileSize, FileStream);
				Result[FileSize/2] = L'\0';
				for (int Index = 0; Index < FileSize; Index += 2) { // swaps points from BE to LE
					uint8_t *TempPointer = (uint8_t*)&(Result[Index/2]);
					TempPointer[0] = TempPointer[0] ^ TempPointer[1];
					TempPointer[1] = TempPointer[0] ^ TempPointer[1];
					TempPointer[0] = TempPointer[0] ^ TempPointer[1];
				}
			}
		}
		
		else { // Assuming UTF-8/Ascii
			if ((FileSize >= 3) &&
			    (ByteOrderMark[0] == 0xEF) &&
			    (ByteOrderMark[1] == 0xBB) &&
			    (ByteOrderMark[2] == 0xBF)) { // remove UFT-8 Header if present.
				FileSize -= 3;
				if (FileSize == 0) {
					wprintf(L"[Error File Handling] This file contains no textual content!\n");
					*Success = FALSE;
				}
			}
			if (*Success) {
				int LengthOfResult = 100;
				int ResultIndex = 0;
				Result = malloc(sizeof(wchar_t) * LengthOfResult);
				
				for (int Index = 0; Index < FileSize; Index++) {
					if(ResultIndex >= LengthOfResult) {
						LengthOfResult += 100;
						Result = realloc(Result, sizeof(wchar_t) * LengthOfResult);
					}
					
					uint8_t Buffer[4] = {0};
					fread(&Buffer[0], sizeof(uint8_t), 1, FileStream);
					uint8_t MultibyteCode = Buffer[0] & 0xF8;
					if (MultibyteCode == 0xC0) { // 2 Bytes
						fread(&Buffer[1], sizeof(uint8_t), 1, FileStream);
						Result[ResultIndex] =
							(Buffer[0] & 0x1F) |
							((Buffer[1] & 0x3F) << 5);
						ResultIndex++;
					}
					else if (MultibyteCode == 0xE0) { // 3 Bytes
						fread(&Buffer[1], sizeof(uint8_t), 2, FileStream);
						Result[ResultIndex] =
							(Buffer[0] & 0x0F) |
							((Buffer[1] & 0x3F) << 4) |
							((Buffer[2] & 0x3F) << (4+6));
						ResultIndex++;
					}
					else if (MultibyteCode == 0xF0) { // 4 Bytes
						fread(&Buffer[1], sizeof(uint8_t), 3, FileStream);
						uint32_t Codepoint =
							(Buffer[0] & 0x07) |
							((Buffer[1] & 0x3F) << 3) |
							((Buffer[2] & 0x3F) << (3+6)) |
							((Buffer[3] & 0x3F) << (3+6+6));
						Result[ResultIndex] = 0xD800 | (Codepoint >> 10);
						ResultIndex++;
						if(ResultIndex >= LengthOfResult) {
							LengthOfResult += 100;
							Result = realloc(Result, sizeof(wchar_t) * LengthOfResult);
						}
						Result[ResultIndex] = 0xDC00 | Codepoint;
						ResultIndex++;
					}
					else { // 1 Byte
						Result[ResultIndex] = Buffer[0];
						ResultIndex++;
					}
				}
				
				if(ResultIndex >= LengthOfResult) {
						LengthOfResult += 100;
						Result = realloc(Result, sizeof(wchar_t) * LengthOfResult);
				}
				Result[ResultIndex] = L'\0';
			}
		}
		fclose(FileStream);
	}
	
	return Result;
}

int ApplicationMain(FILE *InFile, int InFileSize, FILE *OutLogisim, FILE *OutRawHex, FILE *OutSymbolTable, FILE *OutListing) {
	int Success = TRUE;
	if (InFile == 0) {
		Success = FALSE;
		printf("A input file was not provided!\n");
	}

	if (Success) {
		file_state FileState = {
			.Line = 1,
			.Column = 0,
		};
		FileState.At = LoadFileIntoMemory(InFile, InFileSize, &Success);

		paged_list *IdentifierDestinationList = AllocatePagedList(sizeof(identifier_dest), 10);
		paged_list *IdentifierSourceList = AllocatePagedList(sizeof(identifier_source), 10);
		
		Success = Assemble(&FileState, IdentifierDestinationList, IdentifierSourceList);
		
		if (Success) {
			if ((OutLogisim == 0) && (OutRawHex == 0) && (OutSymbolTable == 0) && (OutListing == 0)) {
				Success = FALSE;
				printf("Warning: No outputs were were requested. No output files are being generated.\n");
			}

			if ((OutRawHex != 0) && (Success)) {
				Success = OutputRawHex(OutRawHex);
			}
			if ((OutLogisim != 0) && (Success)) {
				Success = OutputLogisimImage(OutLogisim);
			}
			if ((OutSymbolTable != 0) && (Success)) {
				Success = OutputSymbolTable(OutSymbolTable, IdentifierDestinationList, IdentifierSourceList);
			}
			if ((OutListing != 0) && (Success)) {
				Success = OutputListing(OutListing, IdentifierDestinationList, IdentifierSourceList);
			}
			
		}
	}

	return Success;
}
