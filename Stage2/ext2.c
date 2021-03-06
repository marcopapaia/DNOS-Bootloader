/*
* License: Public Domain
*/


#include "ext2.h"
#include "pmm.h"
#include <stdbool.h>
#include "hdd.h"
#include "partitions.h"
#include <stdint.h>

int readBlock(struct PARTITION * p, void * readTo, unsigned block);
int readBlockFromInode(struct PARTITION * p, void * dest, const struct EXT2_INODE * inode, unsigned block);

void * memcpy(void * dest, void * src, unsigned count);
void * memset(void * ptr, int value, unsigned num);

bool ext2_DNOSIC_file_exists(struct PARTITION * p){	
	struct EXT2_SUPERBLOCK* superBlock = (struct EXT2_SUPERBLOCK *) p->ext2Data;
	
	unsigned blockSize = 1024 << superBlock->log2BlockSize;
	if(blockSize > 4096) return false;
	//unsigned totGroups = (superBlock->totalBlocks + superBlock->blocksPerGroup - 1) / superBlock->blocksPerGroup;
	
	struct EXT2_DATA * fsData = (struct EXT2_DATA*) p->ext2Data;
	
	fsData->blockSize = blockSize;
	
	char blockGroup_data[512];
	int err = hdd_read_sectors((uint16_t *)&blockGroup_data[0], 1, p->disk, p->startSector + 2 + blockSize/512);
	if(err) return false;
	
	struct EXT2_BLOCK_GROUP_DESCRIPTOR * blockGroup = (struct EXT2_BLOCK_GROUP_DESCRIPTOR *) &blockGroup_data[0];
	
	unsigned inode_table_address = blockGroup->blockAddress_inodeTable;
	
	unsigned inodeSize = 128;
	if(superBlock->versionMajor >= 1) inodeSize = superBlock->sizeOfInode;
	
	
	// Entry 2 = Root directory table
	
	unsigned index = (2-1) % superBlock->inodesPerGroup;
	unsigned containingBlock = (index * inodeSize) / blockSize;
	
	void * tmpBlock = pmm_allocate_block();
	err = hdd_read_sectors(tmpBlock, 1, p->disk, p->startSector + (containingBlock + inode_table_address) * (blockSize/512));
	if(err) {pmm_free_block(tmpBlock);return false;}
	
	memcpy(&fsData->rootDirectoryInode, (void *)((unsigned)tmpBlock + inodeSize), 128);
	
	pmm_free_block(tmpBlock);
	
	struct EXT2_INODE* inode = &fsData->rootDirectoryInode;
	
	if(inode->directBlock[0] == 0){
		return false;	
	}
	// We now have the inode of the root directory.
	
	
	void* loaded_block = pmm_allocate_block();
	err = hdd_read_sectors(loaded_block, blockSize / 512, p->disk, p->startSector + ((inode->directBlock[0] * blockSize)/512));
	if(err) {pmm_free_block(loaded_block);return false;}
	
	struct EXT2_DIR_ENTRY* dirEntry = (struct EXT2_DIR_ENTRY *) loaded_block;
	
	if(dirEntry->inode == 0){// Empty directory
		pmm_free_block(loaded_block);
		return false;
	}
	
	unsigned cur_entry = 0;
	unsigned cur_offset = 0;
	unsigned cur_loaded_block = 0;
	bool found_entry = false;
	for(;;){
		unsigned cur_block = (cur_offset / blockSize) * blockSize;
		if(cur_loaded_block != cur_block){
			err =  readBlockFromInode(p, loaded_block, inode, cur_block);
		}
		
		dirEntry = (struct EXT2_DIR_ENTRY *) (((unsigned)loaded_block) + (cur_offset % p->ext2Data->blockSize));
		
		if(dirEntry->inode == 0) break;
			
		if(dirEntry->nameLength != 10){
			cur_entry++;
			cur_offset += dirEntry->entrySize;
			continue;
		}
		
		//int bytesLeftInLoadedBlock = blockSize - (cur_offset % blockSize);
		
		// We assume we don't need a new block
		if(dirEntry->name[0] == 'D' &&
		dirEntry->name[1] == 'N' &&
		dirEntry->name[2] == 'O' &&
		dirEntry->name[3] == 'S' &&
		dirEntry->name[4] == 'I' &&
		dirEntry->name[5] == 'C' &&
		dirEntry->name[6] == '.' &&
		dirEntry->name[7] == 'C' &&
		dirEntry->name[8] == 'F' &&
		dirEntry->name[9] == 'G'){
			found_entry = true;
			break;
		}
		
		cur_entry++;
		cur_offset += dirEntry->entrySize;
	}
	
	pmm_free_block(loaded_block);
	
	return found_entry;
}

int readBlock(struct PARTITION * p, void * readTo, unsigned block)
{
	return hdd_read_sectors(readTo, p->ext2Data->blockSize/512, p->disk, p->startSector + ((block * p->ext2Data->blockSize) / 512));
}

int readBlock_vmode(struct PARTITION * p, unsigned blockSize, void * readTo, unsigned block)
{
	return hdd_read_sectors(readTo, blockSize/512, p->disk, p->startSector + ((block * blockSize) / 512));
}


int readBlockFromInode(struct PARTITION * p, void * dest, const struct EXT2_INODE * inode, unsigned i)
{
	// Direct
	
	if( i < 12 ){
		if( inode->directBlock[i] == 0 ) return 1;
		return readBlock(p, dest, inode->directBlock[i]);
	}
	
	// Singly Direct
	
	
	// i=12
	
	unsigned blocksInSingly = p->ext2Data->blockSize / 4;
	
	// 256
	
	if( i < (12 + blocksInSingly) ) {
		uint32_t* loadedSingly = pmm_allocate_block();
		int err = readBlock(p, loadedSingly, inode->singly);
		if(!err){
			unsigned index = i - 12;
			err = readBlock(p, dest, loadedSingly[index]);
		}
		pmm_free_block(loadedSingly);
		return err;
	}
	
	// TODO Doubly Direct
	
	// TODO Triply Direct
	
	
	return 1;
}

int readBlockFromInode_vmode(struct PARTITION * p, unsigned blockSize, void * dest, const struct EXT2_INODE * inode, unsigned i)
{
	// Direct
	
	if( i < 12 ){
		if( inode->directBlock[i] == 0 ) return 1;
		return readBlock_vmode(p, blockSize, dest, inode->directBlock[i]);
	}
	
	// Singly Direct
	
	unsigned blocksInSingly = blockSize / 4;
	
	if( i < (12 + blocksInSingly) ) {
		uint32_t* loadedSingly = (uint32_t *) dest;
		int err = readBlock_vmode(p, blockSize, loadedSingly, inode->singly);
		if(!err){
			unsigned index = i - 12;
			err = readBlock_vmode(p, blockSize, dest, loadedSingly[index]);
		}
		return err;
	}
	
	// TODO Doubly Direct
	
	// TODO Triply Direct
	
	
	return 1;
}

int getDNOSInode(struct PARTITION * p, struct EXT2_INODE * inode, struct EXT2_INODE * rootDirInode);



int getKernelInode(struct PARTITION * p, struct EXT2_INODE * inode)
{
	void* loaded_block = pmm_allocate_block();
	int err = hdd_read_sectors(loaded_block, p->ext2Data->blockSize / 512, p->disk, p->startSector + ((p->ext2Data->rootDirectoryInode.directBlock[0] * p->ext2Data->blockSize)/512));
	if(err) {pmm_free_block(loaded_block);return 1;}
	
	struct EXT2_DIR_ENTRY* dirEntry = (struct EXT2_DIR_ENTRY *) loaded_block;
	
	if(dirEntry->inode == 0){// Empty directory
		pmm_free_block(loaded_block);
		return 1;
	}
	
	unsigned cur_entry = 0;
	unsigned cur_offset = 0;
	unsigned cur_loaded_block = 0;
	for(;;){
		unsigned cur_block = (cur_offset / p->ext2Data->blockSize) * p->ext2Data->blockSize;
		if(cur_loaded_block != cur_block){
			err =  readBlockFromInode(p, loaded_block, inode, cur_block);
			if(err) break;
		}
		
		dirEntry = (struct EXT2_DIR_ENTRY *) (((unsigned)loaded_block) + (cur_offset % p->ext2Data->blockSize));
		
		if(dirEntry->inode == 0) {err=1; break;}
			
		if(dirEntry->nameLength != 15){
			cur_entry++;
			cur_offset += dirEntry->entrySize;
			continue;
		}
		
		//int bytesLeftInLoadedBlock = p->ext2Data->blockSize - (cur_offset % p->ext2Data->blockSize);
		
		// We assume we don't need a new block
		if(dirEntry->name[0] == 'd' &&
		dirEntry->name[1] == 'n' &&
		dirEntry->name[2] == 'o' &&
		dirEntry->name[3] == 's'&&
		dirEntry->name[4] == '_'&&
		dirEntry->name[5] == 'k'&&
		dirEntry->name[6] == 'e'&&
		dirEntry->name[7] == 'r'&&
		dirEntry->name[8] == 'n'&&
		dirEntry->name[9] == 'e'&&
		dirEntry->name[10] == 'l'&&
		dirEntry->name[11] == '.'&&
		dirEntry->name[12] == 'e'&&
		dirEntry->name[13] == 'l'&&
		dirEntry->name[14] == 'f'){
			err = 0;
			break;
		}
		
		cur_entry++;
		cur_offset += dirEntry->entrySize;
	}
	
	if(err){ 
		pmm_free_block(loaded_block);
		return 1;
	}
	
	unsigned DNOSInode = dirEntry->inode;
	
	
	unsigned inodeBlockGroup = (DNOSInode - 1) / p->ext2Data->superBlock.inodesPerGroup;// 0
	
	
	char blockGroup_data[512];
	err = hdd_read_sectors((uint16_t *)&blockGroup_data[0], 1, p->disk, p->startSector + 2 + (p->ext2Data->blockSize/512) + (inodeBlockGroup * 32 / 512));
	if(err) return 1;
	
	struct EXT2_BLOCK_GROUP_DESCRIPTOR * blockGroup = (struct EXT2_BLOCK_GROUP_DESCRIPTOR *) &blockGroup_data[(inodeBlockGroup * 32) - (inodeBlockGroup * 32 / 512 * 512)];
	
	unsigned inode_table_address = blockGroup->blockAddress_inodeTable;
	
	unsigned inodeSize = 128;
	if(p->ext2Data->superBlock.versionMajor >= 1) inodeSize = p->ext2Data->superBlock.sizeOfInode;
	
	
	// in inode table now
	
	unsigned index = (DNOSInode-1) % p->ext2Data->superBlock.inodesPerGroup;
	unsigned containingBlock = (index * inodeSize) / p->ext2Data->blockSize;
	
	void * tmpBlock = pmm_allocate_block();
	err = hdd_read_sectors(tmpBlock, p->ext2Data->blockSize / 512, p->disk, p->startSector + ((containingBlock + inode_table_address) * (p->ext2Data->blockSize/512)));
	if (err) { pmm_free_block(tmpBlock); pmm_free_block(loaded_block); return 1; }
	
	struct EXT2_INODE* inodeSrc = (struct EXT2_INODE *)(((unsigned)tmpBlock) + (index*inodeSize % p->ext2Data->blockSize));
	
	memcpy(inode, inodeSrc, 128);
	
	pmm_free_block(loaded_block);
	pmm_free_block(tmpBlock);
	
	return 0;
	
}


int ext2_load_block(struct PARTITION * p, struct EXT2_INODE * inode, int block, void * dest){
	return readBlockFromInode(p, dest, inode, block);
}

int ext2_load_block_vmode(struct PARTITION * p, unsigned blockSize, struct EXT2_INODE * inode, int block, void * dest){
	return readBlockFromInode_vmode(p, blockSize, dest, inode, block);
}





