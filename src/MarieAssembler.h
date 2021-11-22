/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
 */

#ifndef MARIEASSEMBLER_H
#define MARIEASSEMBLER_H

#include <stdint.h>

#ifdef TRUE
#undef TRUE
#endif

#define TRUE (1)

#ifdef FALSE
#undef FALSE
#endif

#define FALSE (0)

#ifdef Assert
#undef Assert
#endif

#ifdef DEBUG
#define Assert(Cnd) if ((Cnd) != TRUE) { __debugbreak(); }
#else
#define Assert(Cnd)
#endif

#ifdef Min
#undef Min
#endif

#define Min(A, B) ((A) < (B) ? (A) : (B))

#ifdef Max
#undef Max
#endif

#define Max(A, B) ((A) > (B) ? (A) : (B))

#define Kilobyte(A) ((A) * 1024)
#define Megabyte(A) ((Kilobyte(A) * 1024)

#define global_var static
#define local_persist static

#define NO_OPCODE (0xFFFF0000)

typedef struct {
	wchar_t *String;
	int Length;
	int Opcode;
} keyword_entry;

global_var const keyword_entry Keywords[] = {
	{
		.String = L"jns",
		.Length = 3,
		.Opcode = 0x0000,
	},
	{
		.String = L"load",
		.Length = 4,
		.Opcode = 0x1000,
	},
	{
		.String = L"store",
		.Length = 5,
		.Opcode = 0x2000,
	},
	{
		.String = L"add",
		.Length = 3,
		.Opcode = 0x3000,
	},
	{
		.String = L"subt",
		.Length = 4,
		.Opcode = 0x4000,
	},
	{
		.String = L"input",
		.Length = 5,
		.Opcode = 0x5000,
	},
	{
		.String = L"output",
		.Length = 6,
		.Opcode = 0x6000,
	},
	{
		.String = L"halt",
		.Length = 4,
		.Opcode = 0x7000,
	},
	{
		.String = L"skipcond",
		.Length = 8,
		.Opcode = 0x8000,
	},
	{
		.String = L"jump",
		.Length = 4,
		.Opcode = 0x9000,
	},
	{
		.String = L"clear",
		.Length = 5,
		.Opcode = 0xA000,
	},
	{
		.String = L"addi",
		.Length = 4,
		.Opcode = 0xB000,
	},
	{
		.String = L"jumpi",
		.Length = 5,
		.Opcode = 0xC000,
	},
	{
		.String = L"loadi",
		.Length = 5,
		.Opcode = 0xD000,
	},
	{
		.String = L"storei",
		.Length = 6,
		.Opcode = 0xE000,
	},
	{
		.String = L".SetAddr",
		.Length = 8,
		.Opcode = NO_OPCODE,
	},
	{
		.String = L".Ident",
		.Length = 6,
		.Opcode = NO_OPCODE,
	},
	{
		.String = L"data",
		.Length = 4,
		.Opcode = NO_OPCODE,
	},
};

// keyword_index should be able to index correctly into Keywords table.
enum keyword_index {
	KW_Jumpstore = 0,
	KW_Load,
	KW_Store,
	KW_Add,
	KW_Sub,
	KW_Input,
	KW_Output,
	KW_Halt,
	KW_Skipcond,
	KW_Jump,
	KW_Clear,
	KW_Addi,
	KW_Jumpi,
	KW_Loadi,
	KW_Storei,
	KW_M_SetAddr,
	KW_M_Ident,
	KW_Data,
	// Keep this at the end, used for iterating though all keywords.
	KW_COUNT,
};

enum emit_code {
	EMIT_No,
	EMIT_Jump,
	EMIT_Jumpi,
	EMIT_Jumpstore,
	EMIT_Skipcond,
	EMIT_Store,
	EMIT_Storei,
	EMIT_Clear,
	EMIT_Output,
	EMIT_Halt,
};

typedef struct {
	int Line, Column;
	wchar_t *At;
} file_state;

typedef struct {
	wchar_t *Start;
	int Length;
	int Address;
	int Line;
	int Column;
} identifier_dest;

typedef struct {
	wchar_t *Start;
	int Length;
	int Value;
	int Line;
	int Column;
} identifier_source;

#define PMD_IsOccupied (0x1)
#define PMD_UsedIdentifier (0x2)
#define PMD_DefinedIdentifier (0x4)
#define PMD_IsData (0x8)

#define ArraySize(Array) (sizeof(Array)/sizeof(*Array))

#endif
