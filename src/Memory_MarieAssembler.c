/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * File: Defines growable lists
 */

#include "Memory_MarieAssembler.h"
#include <malloc.h>
#include <string.h>

translation_scope inline paged_list* AllocatePagedList(uint32_t SizeOfElement, uint32_t Length) {
	paged_list *Result = 0;

	Result = calloc(1, sizeof(paged_list));
	Result->NextFreeIndex = 0;
	Result->NextPage = 0;
	Result->SizeOfElement = SizeOfElement;
	Result->Length = Length;
	Result->Memory = calloc(Length, SizeOfElement);
	
	return Result;
}

translation_scope inline void AddToPagedList(paged_list *List, void *Data) {
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

translation_scope inline void* GetFromPagedList(paged_list *List, uint32_t Index) {
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

translation_scope inline void FreePagedList(paged_list *List) {
	paged_list *Last = 0;
	paged_list *Temp = List;
	
	while (List) {
		if (Temp->NextPage) {
			Last = Temp;
			Temp = Temp->NextPage;
		}
		else {
			free(Temp->Memory);
			free(Temp);

			if (Last) { Last->NextPage = 0; }

			if (Temp == List) { List = 0; }
			else { Temp = List; }
		}
	}
}
