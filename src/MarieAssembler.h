/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 */

#ifndef MARIEASSEMBLER_H
#define MARIEASSEMBLER_H

#include <stdint.h>
#include "Platform_MarieAssembler.h"

#define NO_OPCODE (0xFFFF0000)

typedef struct {
	char *String;
	int Length;
	int Opcode;
} keyword_entry;

global_var const keyword_entry Keywords[] = {
	{
		.String = "jns",
		.Length = 3,
		.Opcode = 0x0000,
	},
	{
		.String = "load",
		.Length = 4,
		.Opcode = 0x1000,
	},
	{
		.String = "store",
		.Length = 5,
		.Opcode = 0x2000,
	},
	{
		.String = "add",
		.Length = 3,
		.Opcode = 0x3000,
	},
	{
		.String = "subt",
		.Length = 4,
		.Opcode = 0x4000,
	},
	{
		.String = "input",
		.Length = 5,
		.Opcode = 0x5000,
	},
	{
		.String = "output",
		.Length = 6,
		.Opcode = 0x6000,
	},
	{
		.String = "halt",
		.Length = 4,
		.Opcode = 0x7000,
	},
	{
		.String = "skipcond",
		.Length = 8,
		.Opcode = 0x8000,
	},
	{
		.String = "jump",
		.Length = 4,
		.Opcode = 0x9000,
	},
	{
		.String = "clear",
		.Length = 5,
		.Opcode = 0xA000,
	},
	{
		.String = "addi",
		.Length = 4,
		.Opcode = 0xB000,
	},
	{
		.String = "jumpi",
		.Length = 5,
		.Opcode = 0xC000,
	},
	{
		.String = "loadi",
		.Length = 5,
		.Opcode = 0xD000,
	},
	{
		.String = "storei",
		.Length = 6,
		.Opcode = 0xE000,
	},
	{
		.String = ".SetAddr",
		.Length = 8,
		.Opcode = NO_OPCODE,
	},
	{
		.String = ".Ident",
		.Length = 6,
		.Opcode = NO_OPCODE,
	},
	{
		.String = "data",
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
	char *At;
} file_state;

typedef struct {
	char *Start;
	int CharCount;
	int ByteCount;
	int Address;
	int Line;
	int Column;
} identifier_dest;

typedef struct {
	char *Start;
	int CharCount;
	int ByteCount;
	int Value;
	int Line;
	int Column;
} identifier_source;

#define PMD_IsOccupied (0x1)
#define PMD_UsedIdentifier (0x2)
#define PMD_DefinedIdentifier (0x4)
#define PMD_IsData (0x8)

#define ArraySize(Array) (sizeof(Array)/sizeof(*Array))

global_var const char* const ReservedNames[] = {
	"ram",
	"if",
	"goto",
};

#endif
