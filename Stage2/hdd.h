/*
* License: Public Domain
*/


#ifndef _HDD_H
#define _HDD_H

#include <stdint.h>
#include <stdbool.h>

#define HDD_MAX_DEVICES 4

struct HDD_INFO
{
	bool exists;
	bool lba48;// boolean
	uint32_t sectorCount;
} __attribute__((packed));

void hdd_detect_devices();

int hdd_read_sectors(uint16_t * readTo, uint16_t sectors, unsigned drive, uint32_t startSector);

#endif
