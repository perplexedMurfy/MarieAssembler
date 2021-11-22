/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
 * File: Conatians dynamically sized lists.
 */

#include "Memory_MarieAssembler.h"
#include <malloc.h>

inline paged_list* AllocatePagedList(uint32_t SizeOfElement, uint32_t Length) {
	paged_list *Result = 0;

	Result = calloc(1, sizeof(paged_list));
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
			List->NextPage = AllocatePagedList(List->SizeOfElement, List->Length);
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
