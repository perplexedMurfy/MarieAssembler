/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
 */

#ifndef MEMORY_MARIEASSEMBLER_H
#define MEMORY_MARIEASSEMBLER_H

#include <stdint.h>

typedef struct paged_list paged_list;

struct paged_list {
	void *Memory;
	uint32_t SizeOfElement;
	uint32_t Length;
	uint32_t NextFreeIndex;
	struct paged_list *NextPage;
};

translation_scope paged_list* AllocatePagedList(uint32_t SizeOfElement, uint32_t Length);
translation_scope void AddToPagedList(paged_list *List, void *Data);
translation_scope void FreePagedList(paged_list *List);
#endif
