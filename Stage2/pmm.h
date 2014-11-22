/*
* License: Public Domain
*/


#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>

void pmm_init();
void * pmm_allocate_block();
void * pmm_allocate_blocks(unsigned blockCount);
void pmm_free_block(void * address);
void pmm_free_region(uint32_t base, unsigned size);
void pmm_allocate_region(uint32_t base, unsigned size);

uint32_t pmm_used_blocks();

#endif
