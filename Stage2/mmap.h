/*
* License: Public Domain
*/


#ifndef _MMAP_H
#define _MMAP_H

struct mmap_entry
{
	uint32_t	startLo;	//base address
	uint32_t	startHi;
	uint32_t	sizeLo;		//length (in bytes)
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
} __attribute__((packed));

#endif
