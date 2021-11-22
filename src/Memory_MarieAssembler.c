/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
 * File: Conatians abstratced memory management functions and dynamically sized lists.
 */

#include "Memory_MarieAssembler.h"
#include <malloc.h>

linear_arena InitArena (size_t Size) {
	linear_arena Result;
	Result.Start = malloc(Size);
	Result.End = Result.Start + Size;
	Result.Current = Result.Start;
	Result.Mark = 0;
	return Result;
}

void* PushMemory(linear_arena *Arena, size_t Size) {
	Assert(Arena->Current + Size < Arena->End);
	if (Arena->Current + Size >= Arena->End) {
		wprintf(L"[Error Memory Allocation] The program ran out of memory! Increase the amoun of memory given to this arena if you want a easy fix.\n");
		return 0; 
	}

	void *Result = Arena->Current;
	Arena->Current += Size;

	return Result; 
}
#define PushStruct(Arena, Type) PushMemory((Arena), sizeof(Type));
#define PushArray(Arena, Type, Count) PushMemory((Arena), sizeof(Type) * (Count));

void ClearArena(linear_arena *Arena) {
	Arena->Current = Arena->Start;
	Arena->Mark = 0;
	memset(Arena->Start, 0, Arena->End - Arena->Start);
}

//~

inline paged_list* PushPagedList(linear_arena *Arena, uint32_t SizeOfElement, uint32_t Length) {
	paged_list *Result = 0;

	Result = PushStruct(Arena, paged_list);
	Result->NextFreeIndex = 0;
	Result->NextPage = 0;
	Result->SizeOfElement = SizeOfElement;
	Result->Length = Length;
	Result->Memory = PushMemory(Arena, Length * SizeOfElement);
	
	return Result;
}

inline void AddToPagedList(paged_list *List, void *Data, linear_arena *Arena) {
	while (List->NextFreeIndex == List->Length) {
		if (List->NextPage) {
			List = List->NextPage;
		}
		else {
			List->NextPage = PushPagedList(Arena, List->SizeOfElement, List->Length);
			List = List->NextPage;
		}
	}

	void *Slot = (void*) ((uint8_t*)List->Memory + List->SizeOfElement * List->NextFreeIndex);
	memcpy(Slot, Data, List->SizeOfElement);
	List->NextFreeIndex++;
}

inline void* GetFromPagedList(paged_list *List, uint32_t Index) {
	while (Index >= List->Length) {
		Index -= List->Length;
		if (List->NextPage) {
			List = List->NextPage;
		}
		else {
			return 0;
		}
	}
	
	if (Index < List->NextFreeIndex) {
		return (void*) ((uint8_t*)List->Memory + List->SizeOfElement * Index);
	}
	return 0;
}
