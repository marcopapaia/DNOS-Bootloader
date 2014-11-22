/*
* License: Public Domain
*/


#include <stdint.h>
#include <stdbool.h>
#include "display.h"
#include "io.h"
#include "mmap.h"
#include "pmm.h"
#include "hdd.h"
#include "paging.h"
#include "partitions.h"
#include "idt.h"
#include "pic.h"
#include "pit.h"
#include "ext2.h"
#include "elf.h"

void hide_cursor();
void set_progress_bar(uint8_t progress);

uint32_t totalRAM = 0;// RAM in KB
uint16_t totalMemMapEntries = 0;

volatile uint32_t timerTicks = 0;

void timer_handler();

extern struct PARTITION partitions[MAX_PARTITIONS];

void * memcpy(void * dest, void * src, unsigned count);
void * memset(void * ptr, int value, unsigned num);

uint32_t * pdt;

struct BOOT_INFO {
	uint32_t bootDevice;// 0 = HDD
	uint32_t bootDevice2;// HDD number (if HDD)
	uint32_t ramKB;
	uint32_t usedBlocks;
} __attribute__((packed));

extern uint32_t used_blocks;

struct BOOT_INFO bootInfo;

void enablePaging();
void disablePaging();

void increment_progress_bar();

void enterKernel(unsigned address);

void _start()
{
	pmode_exec_start:
	
	totalRAM = *((uint32_t *)0x500);
	totalMemMapEntries = *((uint16_t *)0x504);
	
	hide_cursor();
	
	display_clear(0x00);

	display_place_string(0, 0, "DNOS Bootloader alpha", 0x0F);
	display_place_string(0, 24, "DNOS Bootloader is Setting Up...", 0x03);
		
	pmm_init();
	
	struct mmap_entry* mmap = (struct mmap_entry *)0x28400;

	for (unsigned i = 0; i < totalMemMapEntries; i++){
		if (mmap->type == 1){
			pmm_free_region(mmap->startLo, mmap->sizeLo);
		}
		mmap++;
	}
	
	pmm_allocate_region(0, 1024*1024);
	
	idt_init();
	pic_init();
	pit_init();
	
	
	/* Ready paging */
	
	pdt = (uint32_t *) pmm_allocate_block();
	memset(pdt, 0, 4096);
	
	uint32_t * page_table_0 = (uint32_t *) pmm_allocate_block();
	memset(page_table_0, 0, 4096);
	page_table_0[1023] = (uint32_t)page_table_0 | 3;// Last page maps page table
	
	for(uint32_t address = 0, i = 0; i < 1023; i++){
		page_table_0[i] = address | 3;
		address += 4096;
	}
	
	pdt[0] = ((uint32_t) page_table_0) | 3;
	page_table_0[6] = ((uint32_t)pdt) | 3;
		
	
	__asm__ ("sti");
	hdd_detect_devices();
	
	partition_manager_init();
	
	for(int i = 0; i < MAX_PARTITIONS; i++){
		struct PARTITION * p = &partitions[i];
		if(p->fileSystem == FILE_SYSTEM_EXT2){
			if(ext2_DNOSIC_file_exists(p)){
				// DNOS Partition
				
				bootInfo.bootDevice = 0;// HDD;
				bootInfo.bootDevice2 = p->disk;
				bootInfo.ramKB = totalRAM;
				
				struct EXT2_INODE kernelInode;
				int err = getKernelInode(p, &kernelInode);
				
				if(err)
					continue;
					
					
				
				display_place_string(0, 24, "Loading Kernel...               ", 0x03);
								
				void* buffer = pmm_allocate_block();
				err = ext2_load_block(p, &kernelInode, 0, buffer);
				if(err){
					goto pmode_exec_start;
				}
				
				if(!elf_check_header((struct Elf32_Ehdr *) buffer))
					continue;
				
				if(elf_get_type((struct Elf32_Ehdr *) buffer) == 0)
					continue;
					
				// Loop through all section headers in table
				
				struct Elf32_Ehdr* hdr = (struct Elf32_Ehdr *) buffer;
				
				void * loadedSectionHeaders = pmm_allocate_block();
				err = ext2_load_block(p, &kernelInode, hdr->e_shoff / p->ext2Data->blockSize, loadedSectionHeaders);
				if(err){
					goto pmode_exec_start;
				}
				
				unsigned offset = (hdr->e_shoff % p->ext2Data->blockSize) + hdr->e_shentsize;
				
				// Pass 1: Ensure all pages exist
				for(int s = 1; s < hdr->e_shnum; s++){
					struct Elf32_Shdr* sectionHeader = (struct Elf32_Shdr *)((unsigned)loadedSectionHeaders + offset);
					if(sectionHeader->sh_addr >= 0x100000/* && (sectionHeader->sh_type == SHT_PROGBITS || sectionHeader->sh_type == SHT_NOBITS)*/){
						unsigned startBlock = sectionHeader->sh_addr / 4096;
						unsigned endBlock = ((sectionHeader->sh_addr + sectionHeader->sh_size + 4095) / 4096);
						allocateVirtual(pdt, startBlock, endBlock - startBlock);
					}	
					offset += hdr->e_shentsize;				
				}
				
				unsigned vSectionHdrs = 0x7000;
				
				void* data = pmm_allocate_block();
				page_table_0[0] = ((unsigned) data) | 3;
				page_table_0[vSectionHdrs / 4096] = ((unsigned) loadedSectionHeaders) | 3;
				
				
				unsigned blockSize = p->ext2Data->blockSize;
				
				offset = (hdr->e_shoff % blockSize) + hdr->e_shentsize;
				unsigned e_shnum = hdr->e_shnum;
				unsigned e_shentsize = hdr->e_shentsize;
				
	
				enablePaging();
				
				// Pass 2: Load all text and data sections from file, zero all bss sections
				for(unsigned s = 1; s < e_shnum; s++){					
					//display_place_string(0, s, "starting", 0x02);
					//char buf[10];
					//itoa(s, 10, buf);
					//display_place_string(9, s, buf, 0x02);
					
					struct Elf32_Shdr* sectionHeader = (struct Elf32_Shdr *)(vSectionHdrs + offset);
					
					if(sectionHeader->sh_addr >= 0x100000 /*&& (sectionHeader->sh_flags & SHF_ALLOC) != 0*/){// Under 1MB?
					if(sectionHeader->sh_type == SHT_PROGBITS){// Loaded from file
						unsigned vAddr = sectionHeader->sh_addr;
						unsigned fileOffset = sectionHeader->sh_offset;
						unsigned bytesLeft = sectionHeader->sh_size;
						for(int ppp;;ppp++){
							err = ext2_load_block_vmode(p, blockSize, &kernelInode, fileOffset / blockSize, 0);
							// TODO Optimisation: What if we have already loaded that block?
							if(err) goto pmode_exec_start;
							//increment_progress_bar();
							if (((fileOffset % blockSize) + bytesLeft) > blockSize){
								unsigned total = blockSize - (fileOffset % blockSize);
								memcpy((void *)vAddr, (void *)(fileOffset % blockSize), total);
								bytesLeft -= total;
								fileOffset += total;
								vAddr += total;
							} else {
								memcpy((void *)vAddr, (void *)(fileOffset % blockSize), bytesLeft);
								break;
							}
						}
						//display_place_string(0, 15+s, "done", 0x02);
						//itoa(s, 10, buf);
						//display_place_string(5, 15+s, buf, 0x02);
					} else if(sectionHeader->sh_type == SHT_NOBITS){// Zeroed memory
						//display_place_string(0, 14, "nobits", 0x02);
						memset((void *)sectionHeader->sh_addr, 0, sectionHeader->sh_size);
					}
					}
					
					offset += e_shentsize;
				}
				
				disablePaging();
				
				page_table_0[0] = 3;
				page_table_0[7] = 0x7000 | 3;
				
				pmm_free_block(loadedSectionHeaders);
				pmm_free_block(data);
				
				int z = 0;
				for(; z < 80; z++){
					display_put_character(z, 22, ' ', 0x22);
				}
				display_place_string(0, 24, "Executing Kernel", 0x02);
				
				for(int j = 0; j < MAX_PARTITIONS; j++){
					if(partitions[j].ext2Data != 0)
						pmm_free_block(partitions[j].ext2Data);
				}
				
				pmm_free_block(buffer);
												
				__asm__ __volatile__ ("cli");
				
				bootInfo.usedBlocks = used_blocks;
				
				display_place_string(0, 1, "hdr->e_entry=", 0x02);
				char buf[10];
				itoa(hdr->e_entry, 16, buf);
				display_place_string(13, 1, buf, 0x02);
				
				unsigned e_entry = hdr->e_entry;
				
				enablePaging();
				
				enterKernel(e_entry);
				
				__asm__ __volatile__ ("hlt");
			}
		}
	}
	
	display_place_string(0, 24, "No valid DNOS installation found...halting", 0x04);
	int z = 0;
	for(; z < 80; z++){
		display_put_character(z, 22, ' ', 0x44);
	}
	
	__asm__ __volatile__ ("cli\nhlt");
	
  
  
  
	for(;;);
}



/**
 * N.B. This will comepletely update the row of characters. May add a very slight performance overhead.
 * @param progress 0 - 80
 */
void set_progress_bar(uint8_t progress)
{
	int i = 0;
	for(; i < progress && i < 80; i++){
		display_put_character(i, 22, ' ', 0x11);
	}
	for(; i < 80; i++){
		display_put_character(i, 22, ' ', 0x00);
	}
}

uint8_t progressBarPosition = 0;
void increment_progress_bar(){
	progressBarPosition++;
	if(progressBarPosition >= 80)
		progressBarPosition = 0;
	if(progressBarPosition == 0)
		set_progress_bar(0);
	display_put_character(progressBarPosition, 22, ' ', 0x11);
}

void hide_cursor()
{
	outb(*(uint16_t *)0x0463, 0x0F);
	outb((*(uint16_t *)0x0463) + 1, 209);// 2001 low byte
	outb(*(uint16_t *)0x0463, 0x0E);
	outb((*(uint16_t *)0x0463) + 1, 7);// 2001 high byte
}

void * memcpy(void * dest, void * src, unsigned count)
{
	const char *sp = (const char *)src;
	char *dp = (char *)dest;
	//for (; count != 0; count--) *dp++ = *sp++;
	for(unsigned i = 0; i < count; i++){
		*dp = *sp;
		dp++;
		sp++;
	}
	return dest;
}

void * memset(void * ptr, int value, unsigned num)
{
	void * o = ptr;
	while (num > 0){
		((char *) ptr)[num] = (char) value;
		num--;
	}
	return o;
}
