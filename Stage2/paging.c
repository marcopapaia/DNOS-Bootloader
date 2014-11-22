/*
* License: Public Domain
*/


#include "paging.h"
#include "pmm.h"

#include "display.h"

void * memcpy(void * dest, void * src, unsigned count);
void * memset(void * ptr, int value, unsigned num);

void allocateVirtual(uint32_t * pdt, unsigned block, unsigned blocks)
{
	if(!blocks) return;
	unsigned b = block;
	for(unsigned i = 0; i < blocks; i++){
		unsigned pageTableIndex = b / 1024;
		if(pdt[pageTableIndex] == 0){
			unsigned pt_addr = (unsigned)pmm_allocate_block();
			pdt[pageTableIndex] = pt_addr | 3;
			uint32_t* pt = (uint32_t *)pt_addr;
			for(unsigned i = 0; i < 1024; i++)
				pt[i] = 0;
		}
		
		uint32_t pt_addr = pdt[pageTableIndex] & 0xFFFFF000;
		uint32_t* pageTable = (uint32_t *) pt_addr;
		unsigned pageIndex = b % 1024;
		if(pageTable[pageIndex] == 0){
			uint32_t address = (uint32_t)pmm_allocate_block();
			pageTable[pageIndex] = address | 3;
		}
		
		b++;
	}
	
}
