/*
* License: Public Domain
*/


#include "pmm.h"

#define unlikely(x)     __builtin_expect((x),0)
#define likely(x)       __builtin_expect((x),1)

uint32_t* pmm_memory_bitmap = (uint32_t *)0x38400;
uint32_t pmm_total_blocks = 0;
uint32_t used_blocks = 0;
extern uint32_t totalRAM;

void * memset(void * ptr, int value, unsigned num);

uint32_t pmm_used_blocks()
{
	return used_blocks;
}

void pmm_init(){
	pmm_total_blocks = totalRAM / 4;
	used_blocks = pmm_total_blocks;
	display_put_character(0, 0, 'Z', 0x0F);

	//unsigned t = (pmm_total_blocks+0) / 32;
	unsigned t = (pmm_total_blocks+8) / 8;
	char buf[10];
	itoa(t, 10, buf);
	display_place_string(0, 2, buf, 3);
	//for(unsigned i = 0; i < t; i++)
	//	pmm_memory_bitmap[i] = 0xFFFFFFFF;
	memset(pmm_memory_bitmap, 0xFF, 131072);
	display_put_character(0, 0, 'Y', 0x0F);
}

unsigned pmm_free_blocks(){
	return pmm_total_blocks - used_blocks;
}

int pmm_first_free(){
	for (uint32_t i = 0; i < (pmm_total_blocks/32); i++)// Search for memory map one uint32_t at a time
		for (int j = 0; j < 32; j++) {// For each bit in the uint32_t...
			int bit = 1 << j;
			if (!(pmm_memory_bitmap[i] & bit))
				return (i * 4 * 8) + j;
		}

	return -1;
}

void pmm_set_bit_in_bitmap(int bit) {
	pmm_memory_bitmap[bit / 32] |= (1 << (bit % 32));
}

void pmm_unset_bit_in_bitmap(int bit) {
	pmm_memory_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

int pmm_is_bit_set(int bit) {
	return pmm_memory_bitmap[bit / 32] & (1 << (bit % 32));
}

void* pmm_allocate_block() {
	if (unlikely(used_blocks >= pmm_total_blocks)){
		return (void *)0;
	}

	int block = pmm_first_free();

	if (unlikely(block == -1))
		return 0;	// Out of memory, return NULL pointer

	pmm_set_bit_in_bitmap(block);

	used_blocks++;

	return (void*)(block * 4096);
}

int pmm_first_free_s(unsigned size) 
{
	if (size == 0)
		return -1;

	if (size == 1)
		return pmm_first_free();

	for (uint32_t i = 64; i < (pmm_total_blocks / 32); i++)// skip first 8mb
		if (pmm_memory_bitmap[i] != 0xFFFFFFFF)
			for (int j = 0; j<32; j++) {	// test each bit in the dword
				int bit = 1 << j;
				if (!(pmm_memory_bitmap[i] & bit)) {

					int startingBit = i * 32;
					startingBit += bit;		// get the free bit in the dword at index i

					uint32_t free = 0; // loop through each bit to see if its enough space
					for (uint32_t count = 0; count <= size; count++) {

						if (!pmm_is_bit_set(startingBit + count))
							free++;	// this bit is clear (free frame)

						if (free == size)
							return i * 4 * 8 + j; //free count==size needed; return index
					}
				}
			}

	return -1;
}

void * pmm_allocate_blocks(unsigned blockCount)
{
	if (unlikely(used_blocks >= pmm_total_blocks)){
		return (void *)0;
	}
	
	unsigned freeBlocks = pmm_total_blocks - used_blocks;
	if(unlikely(freeBlocks < blockCount)){
		return 0;
	}

	int frame = pmm_first_free_s(blockCount);

	if (frame == -1)
		return 0;

	for (uint32_t i = 0; i < blockCount; i++)
		pmm_set_bit_in_bitmap(frame + i);

	uint32_t addr = frame * 4096;
	used_blocks += blockCount;

	return (void*)addr;
	
}

void pmm_free_block(void * address)
{
	int block = (unsigned)address / 4096;
	
	pmm_unset_bit_in_bitmap(block);
	
	if(likely(used_blocks != 0))
		used_blocks--;
}

void pmm_free_region(uint32_t base, unsigned size)
{
	int cur_block = base / 4096;
	int blocks = (size+4095) / 4096;

	for (; blocks > 0; blocks--, cur_block++) {
		int bitWasSet = pmm_is_bit_set(cur_block);
		if (bitWasSet){
			pmm_unset_bit_in_bitmap(cur_block);
			used_blocks--;
		}
	}
}

void pmm_allocate_region(uint32_t base, unsigned size)
{
	int cur_block = base / 4096;
	int blocks = (size+4095) / 4096;

	for (; blocks > 0; blocks--, cur_block++) {
		int bitWasSet = pmm_is_bit_set(cur_block);
		if (!bitWasSet){
			pmm_set_bit_in_bitmap(cur_block);
			used_blocks++;
		}
	}
}
