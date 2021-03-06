/*
* License: Public Domain
*/


#include "partitions.h"
#include <stdint.h>
#include <stdbool.h>
#include "hdd.h"
#include "ext2.h"
#include "pmm.h"

struct PARTITION partitions[MAX_PARTITIONS];

extern struct HDD_INFO hdd_devices[HDD_MAX_DEVICES];

bool is_mbr(unsigned char * data);
void get_partitions(unsigned char * data, unsigned disk);

// Returns 0 for no, 1 for FAT12, 2 for FAT16, 2 for FAT32
bool is_fat_boot_sector(unsigned char * data);
bool is_ext_file_system();

void partition_manager_init()
{
	for(unsigned i = 0; i < (sizeof(struct PARTITION) * MAX_PARTITIONS); i++){
		((char *)&partitions)[i] = 0;
	}
	
	for(int i = 0; i < HDD_MAX_DEVICES; i++){
		if(hdd_devices[i].exists){
			unsigned char bootSector[512];
			int err = hdd_read_sectors((uint16_t *) &bootSector[0], 1, i, 0);
			if(err != 0) continue;// Abandon Drive!
			if(!is_mbr(&bootSector[0])){
				struct PARTITION p;
				p.disk = i;
				p.startSector = 0;
				p.sectors = hdd_devices[i].sectorCount;
				p.fileSystem = 0;
			
				add_partition(&p);
			} else {
				get_partitions(&bootSector[0], i);
			}
			
		}
	}
	
	for(int i = 0; i < MAX_PARTITIONS; i++){
		if (partitions[i].present){
			unsigned char bootSector[512];
			int err = hdd_read_sectors((uint16_t *) &bootSector[0], 1, partitions[i].disk, partitions[i].startSector);
			if (err == 0){
				if(is_fat_boot_sector(&bootSector[0])){
					// FAT
				} else if(is_ext_file_system(&partitions[i])){
					// EXT
				}
			}
		}
	}
	
}

bool is_ext_file_system(struct PARTITION * p){
	struct EXT2_SUPERBLOCK * superBlock = (struct EXT2_SUPERBLOCK *) pmm_allocate_block();
	hdd_read_sectors((uint16_t *) superBlock, 2, p->disk, p->startSector + 2);
	
	if(superBlock->ext2Signature != 0xEF53){
		p->fileSystem = 0;
		pmm_free_block(superBlock);
		return 0;
	}
	
	// YES. IT IS.
	
	p->fileSystem = FILE_SYSTEM_EXT2;
	p->ext2Data = (struct EXT2_DATA *)superBlock;
	
	return FILE_SYSTEM_EXT2;
}

bool is_fat_boot_sector(unsigned char * data)
{
	(void)data;
	return 0;
}

void get_partition(unsigned char * data, unsigned disk, unsigned MBROffset)
{
	if(data[MBROffset] != 0){
		uint32_t startSector = *((uint32_t *)&data[MBROffset + 8]);
		uint32_t sectors = *((uint32_t *)&data[MBROffset + 12]);
		if((startSector + sectors) <= hdd_devices[disk].sectorCount){
			struct PARTITION p;
			p.disk = disk;
			p.startSector = startSector;
			p.sectors = sectors;
			p.fileSystem = 0;
			
			add_partition(&p);
		}
	}
}

void get_partitions(unsigned char * data, unsigned disk)
{
	get_partition(data, disk, 446);
	get_partition(data, disk, 462);
	get_partition(data, disk, 478);
	get_partition(data, disk, 494);
}

bool is_mbr(unsigned char * data)
{
	if(data[446] != 0 && data[446] < 0x7F) return false;
	if(data[462] != 0 && data[462] < 0x7F) return false;
	if(data[478] != 0 && data[478] < 0x7F) return false;
	if(data[494] != 0 && data[494] < 0x7F) return false;
	if(data[446]==0 && data[462]==0 && data[478]==0 && data[494]==0) return false;
	return true;
}

void add_partition(struct PARTITION * partition)
{
	for(unsigned i = 0; i < MAX_PARTITIONS; i++){
		if(!partitions[i].present){
			partitions[i].present = 1;
			partitions[i].disk = partition->disk;
			partitions[i].sectors = partition->sectors;
			partitions[i].startSector = partition->startSector;
			partitions[i].fileSystem = partition->fileSystem;
			return;
		}
	}
}
