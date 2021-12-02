/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
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
void IncFilePos(file_state *File, int Count) {
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
void ConsumeWhitespaceAndComments(file_state *File) {
	while (File->At[0] == L' '  ||
	       File->At[0] == L'\n' ||
	       File->At[0] == L'\r' ||
	       File->At[0] == L'\t' ||
	       File->At[0] == L'/') {
	       
		if (File->At[0] == L'/') {
			while (File->At[0] != L'\n' &&
			       File->At[0] != L'\0') {
				IncFilePos(File, 1);
			}
			if (File->At[0] == L'\n') {
				IncFilePos(File, 1);
			}
		}
		else {
			IncFilePos(File, 1);
		}
	}
}

/* Advances File->At past whitespace on the same line.
 */
void ConsumeWhitespaceNoNewLine(file_state *File) {
	while (File->At[0] == L' '  ||
	       File->At[0] == L'\r' ||
	       File->At[0] == L'\t') {
		IncFilePos(File, 1);
	}
}

/* if File->At does not point to the beginning of a number of the form `0d0000`, then this function returns false and Result is set to 0.
 * if File->At does point to the beginning of a number, then this function returns true and Result will have the value of that number.
 */
int EatNumberDecimal(file_state *File, int *Result) {
	int Success = FALSE;
	int sign = 1;
	*Result = 0;

	if (File->At[0] == L'0' && File->At[1] == L'd') {
		IncFilePos(File, 2);
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
		
			IncFilePos(File, 1);
		}
	}

	*Result *= sign;
	return Success;
}


/* if File->At does not point to the beginning of a number of the form `0x0000`, then this function returns false and Result is set to 0.
 * if File->At does point to the beginning of a number, then this function returns true and Result will have the value of that number.
 */
int EatNumberHexadecimal(file_state *File, int *Result) {
	int Success = FALSE;
	int sign = 1;
	*Result = 0;

	if (File->At[0] == L'0' && File->At[1] == L'x') {
		IncFilePos(File, 2);
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
		
			IncFilePos(File, 1);
		}
	}
	
	return Success;
}

/* If File->At points to a charcter that can be the beginning of a identifier, then this function returns True, File->At is advnaced past the identifier, and the length of the identifier is returned though Length. 
 * If File->At does not point to a character that can begin an identifier, then 
 */
int EatIdentifier(file_state *File, int *Length) {
	int Success = FALSE;
	if (!(File->At[0] >= L'0' && File->At[0] <= L'9')) {
		Success = TRUE;
		(*Length)++;
		IncFilePos(File, 1);
		while ((File->At[0] != L' ') &&
		       (File->At[0] != L'\n') &&
		       (File->At[0] != L'\r') &&
		       (File->At[0] != L'\t') &&
		       (File->At[0] != L'\0')) {
			(*Length)++;
			IncFilePos(File, 1);
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
		ConsumeWhitespaceAndComments(File);
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
			
			IncFilePos(File, Keywords[KeywordIndex].Length);
			ConsumeWhitespaceNoNewLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;

			// @TODO Allow 0d numbers as addresses
			int Address = 0;
			identifier_dest IdentifierDest = {.Start = File->At, .Address = CurrentAddress, .Line = File->Line, .Column = File->Column};
			if (EatNumberHexadecimal(File, &Address)) {
				ReportErrorConditionally(Address > 0xFFF || Address < 0, &DidErrorOccur, L"[Error L:%d C:%d] The Address provided (0x%X) was not between 0x0 and 0xFFF.\n", File->Line, File->Column, Address);
				WriteProgramData(File, Keywords[KeywordIndex].Opcode | Address, CurrentAddress, PMD_IsOccupied);
			}
			else if (EatIdentifier(File, &IdentifierDest.Length)) {
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

			IncFilePos(File, Keywords[KeywordIndex].Length);
			ConsumeWhitespaceNoNewLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;

			WriteProgramData(File, Keywords[KeywordIndex].Opcode, CurrentAddress, PMD_IsOccupied);
		} break;

		case(KW_Skipcond): {
			// Skipcond [lesser|greater|equal|NUMBER]

			IncFilePos(File, Keywords[KW_Skipcond].Length);
			ConsumeWhitespaceNoNewLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;
			
			int RawOperation = 0;
			if (CompareStr(File->At, L"lesser", 6)) {
				IncFilePos(File, 6);
				RawOperation = 0x000;
			}
			else if (CompareStr(File->At, L"equal", 5)) {
				IncFilePos(File, 5);
				RawOperation = 0x400;
			}
			else if (CompareStr(File->At, L"greater", 7)) {
				IncFilePos(File, 7);
				RawOperation = 0xC00;
			}
			else if (EatNumberHexadecimal(File, &RawOperation)) {
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

			IncFilePos(File, Keywords[KW_M_SetAddr].Length);
			ConsumeWhitespaceNoNewLine(File);
			
			ReportErrorConditionally(EatNumberHexadecimal(File, &CurrentAddress) == FALSE, &DidErrorOccur, L"[Error L:%d C:%d] Unable to Eat a Hexadecimal Number for .SetAddr", File->Line, File->Column);

			ReportErrorConditionally(CurrentAddress > 0xFFF || CurrentAddress < 0, &DidErrorOccur, L"[Error L:%d C:%d] The Address provided (%x) was not between 0x0 and 0xfff.\n", File->Line, File->Column, CurrentAddress);
		} break;

		case(KW_M_Ident): {
			// .Ident [Identifier]

			IncFilePos(File, Keywords[KW_M_Ident].Length);
			ConsumeWhitespaceNoNewLine(File);
			
			identifier_source Data = {.Start = File->At, .Value = CurrentAddress - 1, .Line = File->Line, .Column = File->Column};

			ReportErrorConditionally(LastLineOperationWasProcessed != File->Line, &DidErrorOccur, L"[Error L:%d C:%d] Identifiers must follow right after a operation on the same line.\nEx: data 0d0 .Ident Foo\nIf you don't like it, change it!\n", File->Line, File->Column); 
			ReportErrorConditionally(EatIdentifier(File, &Data.Length) == FALSE, &DidErrorOccur, L"[Error L:%d C:%d] Failed to find an Identifier Name after .Ident!\n", File->Line, File->Column);

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

			IncFilePos(File, Keywords[KW_Data].Length);
			ConsumeWhitespaceNoNewLine(File);
			ToIncrementAddress = TRUE;
			LastLineOperationWasProcessed = File->Line;
			int Value = 0;
			
			if (EatNumberDecimal(File, &Value)) {
			}
			else if (EatNumberHexadecimal(File, &Value)) {
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

// String should be a null terminated 
int IndexOfFromEnd(wchar_t *String, wchar_t Target) {
	int Result = 0;

	while(String[Result] != L'\0') { Result++; }
	Result--;
	
	for(; String[Result] != L'\0'; Result--) {
		if (String[Result] == Target) { return Result; }
	}
	return -1;
}

void OutputLogisimImage(wchar_t *FileName) {
	FILE *FileStream = Platform_WideFOpen(FileName, L"w");
	if (!FileStream) {
		wprintf(L"[Error Output] Could not open file \"%s\" for output.\n", FileName);
	}
	
	fprintf(FileStream, "v2.0 raw\r\n");

	int Consectuive = 0;
	int Prev = Program[0];
	for (int Index = 0; Index <= Kilobyte(4); Index++) {
		if (Prev == Program[Index] && Index != Kilobyte(4)) {
			Consectuive++;
		}
		else {
			char Buffer[16] = {0};
			if (Consectuive == 1) {
				fprintf(FileStream, "%X\n", Prev);
			}
			else {
				fprintf(FileStream, "%d*%X\n", Consectuive, Prev);
			}
			Consectuive = 1;
		}
		Prev = Program[Index];
	}

	fclose(FileStream);
}

void OutputRawHex(wchar_t *FileName) {
	FILE *FileStream = Platform_WideFOpen(FileName, L"w");

	fwrite(Program, sizeof(Program), 1, FileStream);
	
	fclose(FileStream);
}

void OutputSymbolTable(wchar_t *FileName, paged_list *IdentifierDestinationList, paged_list *IdentifierSourceList) {
	FILE *FileStream = Platform_WideFOpen(FileName, L"w,ccs=UNICODE");

	int IdentifierMaxLength = 0;
	for (int Index = 0; ; Index++) {
		const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, Index);
		if (IdentifierSource == 0) { break; }
		
		if (IdentifierSource->Length > IdentifierMaxLength) {
			IdentifierMaxLength = IdentifierSource->Length;
		}
	}

	IdentifierMaxLength = Max(IdentifierMaxLength, 10);

	fwprintf(FileStream, L"| %- *s | Identifier's Value | Addresses that use Identifier\n", IdentifierMaxLength, L"Identifier");
	for (int SourceIndex = 0; ; SourceIndex++) {
		const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, SourceIndex);
		if (IdentifierSource == 0) { break; }
		
		fwprintf(FileStream, L"| %- *.*s | 0x%-0*.3X | ", IdentifierMaxLength, IdentifierSource->Length, IdentifierSource->Start, 18 - 2, IdentifierSource->Value);
		for (int DestIndex = 0; ; DestIndex++) {
			const identifier_dest *IdentifierDest = GetFromPagedList(IdentifierDestinationList, DestIndex);
			if (IdentifierDest == 0) { break; }
			
			if ((IdentifierSource->Length == IdentifierDest->Length) &&
			    CompareStr(IdentifierSource->Start, IdentifierDest->Start, IdentifierSource->Length)) {
				fwprintf(FileStream, L" 0x%-0.3X", IdentifierDest->Address);
			}
		}
		fwprintf(FileStream, L"\n");
	}
	fclose(FileStream);
}

void OutputListing(wchar_t *FileName, paged_list *IdentifierDestinationList, paged_list *IdentifierSourceList) {
	FILE *FileStream = Platform_WideFOpen(FileName, L"w,ccs=UNICODE");
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
	
	fwprintf(FileStream, L"| Address | Opcode | %- *s | High Level Code\n", ListingMaxLength, L"Listing");
	
	for (int Index = 0; Index < Kilobyte(4); Index++) {
		int ListingCharacterCount = 0;
		if (ProgramMetaData[Index] & PMD_IsOccupied) {
			if (InMemoryGap) {
				InMemoryGap = FALSE;
				Assert(ListingMaxLength >= 14);
				fwprintf(FileStream, L"|         |        | .SetAddr 0x%0.3X% *s |\n", Index, ListingMaxLength - 14, L"");
			}

			fwprintf(FileStream, L"| 0x%0.3X   | 0x%0.4X | ", Index, Program[Index]);

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
				ListingCharacterCount += fwprintf(FileStream, L"data 0x%0.4X", Program[Index]);
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
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"0x%0.3X", Program[Index] & 0x0FFF);
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
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"0x%0.3X", Program[Index] & 0x0FFF);
						}
					}
					else {
						if (IdentifierDestination) {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"%s + %.*s", ContentsOfAC, IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"%s + 0x%0.3X", ContentsOfAC, Program[Index] & 0x0FFF);
						}
					}
				}
				else if (Opcode == Keywords[KW_Sub].Opcode) {
					OpcodeMemonic = Keywords[KW_Sub].String;
					EmitCode = EMIT_No;
					if (ContentsOfAC[0] == L'0' && ContentsOfAC[1] == L'\0') {
						if (IdentifierDestination) {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"-%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"-0x%0.3X", Program[Index] & 0x0FFF);
						}
					}
					else {
						if (IdentifierDestination) {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"%s - %.*s", ContentsOfAC, IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"%s - 0x%0.3X", ContentsOfAC, Program[Index] & 0x0FFF);
						}
					}
				}
				else if (Opcode == Keywords[KW_Input].Opcode) {
					OpcodeMemonic = Keywords[KW_Input].String;
					EmitCode = EMIT_No;
					_snwprintf(ContentsOfAC, ContentsOfACSize, L"Input");
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
					_snwprintf(ContentsOfAC, ContentsOfACSize, L"0");
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
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"*%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"*0x%0.3X", Program[Index] & 0x0FFF);
						}
					}
					else {
						if (IdentifierDestination) {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"%s + *%.*s", ContentsOfAC, IdentifierDestination->Length, IdentifierDestination->Start);
						}
						else {
							_snwprintf(ContentsOfAC, ContentsOfACSize, L"%s + *0x%0.3X", ContentsOfAC, Program[Index] & 0x0FFF);
						}
					}
				}
				else if (Opcode == Keywords[KW_Loadi].Opcode) {
					OpcodeMemonic = Keywords[KW_Loadi].String;
					EmitCode = EMIT_No;
					if (IdentifierDestination) {
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"*%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"*0x%0.3X", Program[Index] & 0x0FFF);
					}
				}
				else if (Opcode == Keywords[KW_Storei].Opcode) {
					OpcodeMemonic = Keywords[KW_Storei].String;
					EmitCode = EMIT_Storei;
				}
				ListingCharacterCount += fwprintf(FileStream, L"%s", OpcodeMemonic);

				if (OpcodeMemonic != Keywords[KW_Halt].String &&
				    OpcodeMemonic != Keywords[KW_Input].String &&
				    OpcodeMemonic != Keywords[KW_Output].String &&
				    OpcodeMemonic != Keywords[KW_Clear].String) {
					if (ProgramMetaData[Index] & PMD_UsedIdentifier) {
						Assert(IdentifierDestination != FALSE);
						ListingCharacterCount += fwprintf(FileStream, L" %.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						if (OpcodeMemonic == Keywords[KW_Skipcond].String) {
							switch(Program[Index] & 0x0FFF) {
							case(0x000): ListingCharacterCount += fwprintf(FileStream, L" lesser"); break;
							case(0x400): ListingCharacterCount += fwprintf(FileStream, L" equal"); break;
							case(0xC00): ListingCharacterCount += fwprintf(FileStream, L" greater"); break;
							default: ListingCharacterCount += fwprintf(FileStream, L" 0x%0.3X", Program[Index] & 0x0FFF); break;
							}
						}
						else {
							ListingCharacterCount += fwprintf(FileStream, L" 0x%0.3X", Program[Index] & 0x0FFF);
						}
					}
				}
			}
		
			if (ProgramMetaData[Index] & PMD_DefinedIdentifier) {
				for (int IdentifierIndex = 0; ; IdentifierIndex++) {
					const identifier_source *IdentifierSource = GetFromPagedList(IdentifierSourceList, IdentifierIndex);
					if (IdentifierSource == 0) {
						fwprintf(FileStream, L" .Ident COULD NOT RESOLVE IDENTIFER DEFINITION");
						wprintf(L"[Error Lising] Failed to resolve an Identifier defined at address 0x%0.3X\n", Index);
						break;
					}
					if (IdentifierSource->Value == Index) {
						ListingCharacterCount += fwprintf(FileStream, L" .Ident %.*s", IdentifierSource->Length, IdentifierSource->Start);
						break;
					}
				}
			}
			fwprintf(FileStream, L"% *s | ", ListingMaxLength - ListingCharacterCount, L"");
			if (EmitIndentNextLine) {
				EmitIndentNextLine = FALSE;
				fwprintf(FileStream, L"    ");
			}
			
			{
				switch (EmitCode) {

				case(EMIT_No): {
				} break;
					
				case(EMIT_Jump): {
					if (IdentifierDestination) {
						fwprintf(FileStream, L"Goto %.*s // 0x%0.3X", IdentifierDestination->Length, IdentifierDestination->Start, Program[Index] & 0xFFF);
					}
					else {
						fwprintf(FileStream, L"Goto 0x%0.3X", Program[Index] & 0xFFF);
					}
				} break;

				case(EMIT_Jumpi): {
					if (IdentifierDestination) {
						fwprintf(FileStream, L"Goto *%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						fwprintf(FileStream, L"Goto *0x%0.3X", Program[Index] & 0xFFF);
					}
				} break;

				case(EMIT_Jumpstore): {
					if (IdentifierDestination) {
						fwprintf(FileStream, L"%.*s = PC\n", IdentifierDestination->Length, IdentifierDestination->Start);
						fwprintf(FileStream, L"|         |        | %- *s | % *sGoto (%.*s + 0x1) // (0x%0.3X + 0x1)", ListingMaxLength, L"", EmitIndentNextLine ? 0 : 4, L"", IdentifierDestination->Length, IdentifierDestination->Start, Program[Index] & 0xFFF);
					}
					else {
						fwprintf(FileStream, L"0x%0.3X = PC\n", Program [Index] & 0x0FFF);
						fwprintf(FileStream, L"|         |        | %- *s | Goto (0x%0.3X + 0x1)", ListingMaxLength, L"", Program[Index] & 0x0FFF);
					}
				} break;

				case(EMIT_Skipcond): {
					if ((Program[Index] & 0x0FFF) == 0xC00) { // Greater
						fwprintf(FileStream, L"if ((%s) <= 0)", ContentsOfAC);
					}
					else if ((Program[Index] & 0x0FFF) == 0x400) { // Equal
						fwprintf(FileStream, L"if ((%s) != 0)", ContentsOfAC);
					}
					else if ((Program[Index] & 0x0FFF) == 0x000) { // Lesser
						fwprintf(FileStream, L"if ((%s) >= 0)", ContentsOfAC);
					}
					else { // Unknown
						fwprintf(FileStream, L"Skip next if (unknown operation)");
					}
					EmitIndentNextLine = TRUE;
				} break;

				case(EMIT_Store): {
					if (IdentifierDestination) {
						fwprintf(FileStream, L"%.*s = %s", IdentifierDestination->Length, IdentifierDestination->Start, ContentsOfAC);
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"%.*s", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						fwprintf(FileStream, L"RAM[0x%0.3x] = %s", Program[Index] & 0xFFF, ContentsOfAC);
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"RAM[0x%0.3x]", Program[Index] & 0xFFF);
					}
				} break;

				case(EMIT_Storei): {
					if (IdentifierDestination) {
						fwprintf(FileStream, L"RAM[%.*s] = %s", IdentifierDestination->Length, IdentifierDestination->Start, ContentsOfAC);
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"RAM[%.*s]", IdentifierDestination->Length, IdentifierDestination->Start);
					}
					else {
						fwprintf(FileStream, L"RAM[RAM[0x%0.3X]] = %s", Program[Index] & 0xFFF, ContentsOfAC);
						_snwprintf(ContentsOfAC, ContentsOfACSize, L"RAM[RAM[0x%0.3X]]", Program[Index] & 0xFFF);
					}
					
				} break;

				case(EMIT_Clear): {
					fwprintf(FileStream, L"AC = 0");
				} break;

				case(EMIT_Output): {
					fwprintf(FileStream, L"Output = %s", ContentsOfAC);
				} break;

				case(EMIT_Halt): {
					fwprintf(FileStream, L"End execution");
				} break;

				default: {
					wprintf(L"[Error Listing] We Tried to emit a invalid emit code! Something is wrong with the compiler\n");
				} break;
				}
				EmitCode = EMIT_No;	
			}
			
			fwprintf(FileStream, L"\n");
		}
		else {
			InMemoryGap = TRUE;
		}
	}
	
	fclose(FileStream);
}

wchar_t *LoadFileIntoMemory(wchar_t *FileName, int *Success) {
	wchar_t *Result = 0;
	size_t FileSize = Platform_GetFileSize(FileName, Success);

	if (*Success) {
		FILE *FileStream = Platform_WideFOpen(FileName, L"rb");	
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

int ApplicationMain(int ArgCount, wchar_t **Args) {
	if (!(ArgCount == 2 || ArgCount == 3)) {
		const wchar_t *HelpMessage = 
			L"* Usage: %s <Path To Source> [Output File Name]\n"
			"* This program compiles Marie Assembly code into logisim rom images.\n"
			"* <Path To Source> is the path to the assembly code you'd like to assemble.\n"
			"* [Output File Name] is the name of the files that this program will emit.\n"
			"** If [Output File Name] is not provied, then this program generates [Output File Name] in the form of OUT_<File Name>.\n"
			"* For Every compliation:\n"
			"** A Raw Image file will be emitted as [Output File Name].hex\n"
			"** A listing file will be emitted as [Output File Name].lst\n"
			"** A symbol table will be output as [Output File Name].sym\n"
			"** A logisim image file will bit output as [Output File Name].LogisimImage\n";
		wprintf(HelpMessage, Args[0]);
		return 0;
	}

	int Success = TRUE;

	file_state FileState = {
		.Line = 1,
		.Column = 0,
	};
	FileState.At = LoadFileIntoMemory(Args[1], &Success);
	if (Success == FALSE) { return 0; }

	paged_list *IdentifierDestinationList = AllocatePagedList(sizeof(identifier_dest), 10);
	paged_list *IdentifierSourceList = AllocatePagedList(sizeof(identifier_source), 10);
	
	if (Assemble(&FileState, IdentifierDestinationList, IdentifierSourceList)) {
		wchar_t *OutputFileName;
		int OutputNameLength;

		if (ArgCount == 2) {
			OutputFileName = Platform_CreateOutputFileName(Args[1], &Success);
		}
		else {
			OutputFileName = Args[2];
		}

		OutputNameLength = wcslen(OutputFileName) + 1;
	
		int Size = OutputNameLength;
		
		wchar_t *OutputLogisimImageName = calloc(sizeof(wchar_t), Size + 13);
		wchar_t *OutputRawHexName = calloc(sizeof(wchar_t), Size + 4);
		wchar_t *OutputSymbolTableName = calloc(sizeof(wchar_t), Size + 4);
		wchar_t *OutputListingName = calloc(sizeof(wchar_t), Size + 4);

		_snwprintf(OutputLogisimImageName, Size + 13, L"%.*s.LogisimImage", OutputNameLength + 4, OutputFileName);
		_snwprintf(OutputRawHexName, Size + 4, L"%.*s.hex", OutputNameLength + 4, OutputFileName);	
		_snwprintf(OutputSymbolTableName, Size + 4, L"%.*s.sym", OutputNameLength + 4, OutputFileName);
		_snwprintf(OutputListingName, Size + 4, L"%.*s.lst", OutputNameLength + 4, OutputFileName);

		OutputRawHex(OutputRawHexName);
		OutputLogisimImage(OutputLogisimImageName);
		OutputSymbolTable(OutputSymbolTableName, IdentifierDestinationList, IdentifierSourceList);
		OutputListing(OutputListingName, IdentifierDestinationList, IdentifierSourceList);
	}

	// @TODO output a value that repersents our success.
	return 0;
}
