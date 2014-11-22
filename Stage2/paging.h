/*
* License: Public Domain
*/


#ifndef _PAGING_H
#define _PAGING_H

#include <stdint.h>

/* These routines do not work when paging is enabled */

void allocateVirtual(uint32_t * pdt, unsigned block, unsigned blocks);

#endif

