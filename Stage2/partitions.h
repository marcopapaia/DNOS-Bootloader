/*
* License: Public Domain
*/


/*
Logical Partition Manager
*/

#ifndef _PARTITIONS_H
#define _PARTITIONS_H

#include <stdint.h>
#include <stdbool.h>

struct PARTITION;

#include "ext2.h"

#define FILE_SYSTEM_UNKNOWN 0
#define FILE_SYSTEM_FAT12	1
#define FILE_SYSTEM_FAT16	2
#define FILE_SYSTEM_FAT32	3
#define FILE_SYSTEM_EXT2	10
#define FILE_SYSTEM_EXT3	11
#define FILE_SYSTEM_EXT4	12
#define FILE_SYSTEM_NTFS10	20
#define FILE_SYSTEM_NTFS11	21
#define FILE_SYSTEM_NTFS12	22
#define FILE_SYSTEM_NTFS30	23
#define FILE_SYSTEM_NTFS31	24


#define MAX_PARTITIONS HDD_MAX_DEVICES * 4

struct PARTITION
{
	char present; // 1 is structure is valid
	uint8_t disk;// 0 = bus 0, master. 1 = bus 0, slave etc.
	uint64_t sectors;// Sector. 64-bit to allow for 48-bit devices
	uint64_t startSector;// 64-bit to allow for 48-bit devices
	uint8_t fileSystem;// FILE_SYSTEM_FAT16 etc.
	union{
		struct EXT2_DATA * ext2Data;
	};
	bool dnosParition;
} __attribute__((packed));

void partition_manager_init();


/** 
 * @param partition pointer to structure to be /copied/
 */
void add_partition(struct PARTITION * partition);


#endif
