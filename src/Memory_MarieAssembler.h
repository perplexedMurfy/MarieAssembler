/* Author: Michael Roskuski <mroskusk@student.fitchburgstate.edu>
 * Date: 2021-11-10
 * Program: Assembler for the fictional Marie computer. Supports the extended instruction set.
 */

#ifndef MEMORY_MARIEASEMBLER_H
#define MEMORY_MARIEASEMBLER_H

#include <stdint.h>

typedef struct {
	// Declaring these as uint8_t* because I want to be able to do byte wise pointer arithmetic on them.
	uint8_t *Start;
	uint8_t *End;
	uint8_t *Current;
	uint8_t *Mark;
} linear_arena;

typedef struct paged_list paged_list;

struct paged_list {
	void *Memory;
	uint32_t SizeOfElement;
	uint32_t Length;
	uint32_t NextFreeIndex;
	struct paged_list *NextPage;
};

void* PushMemory(linear_arena *Arena, size_t Size);
#define PushStruct(Arena, Type) PushMemory((Arena), sizeof(Type));
#define PushArray(Arena, Type, Count) PushMemory((Arena), sizeof(Type) * (Count));

void ClearArena(linear_arena *Arena);

paged_list* PushPagedList(linear_arena *Arena, uint32_t SizeOfElement, uint32_t Length);
void AddToPagedList(paged_list *List, void *Data, linear_arena *Arena);
#endif
