/*
* License: Public Domain
*/


#ifndef _EXT2_H
#define _EXT2_H

#include <stdint.h>
#include <stdbool.h>
#include "partitions.h"

struct EXT2_SUPERBLOCK
{
	uint32_t totalInodes;
	uint32_t totalBlocks;
	uint32_t superUserReservedBlocks;
	uint32_t totalUnallocatedBlocks;
	uint32_t totalUnallocatedInodes;
	uint32_t superBlock_BlockNumber;
	uint32_t log2BlockSize;
	uint32_t log2FragmentSize;
	uint32_t blocksPerGroup;
	uint32_t fragmentsPerGroup;
	uint32_t inodesPerGroup;
	uint32_t lastMountTime;// POSIX
	uint32_t lastWrittenTime;// POSIX
	uint16_t mountTimesSinceConsistancyCheck;
	uint16_t mountsAllowedBeforeConsistanyCheck;
	uint16_t ext2Signature;
	uint16_t state;
	uint16_t errorAction;
	uint16_t versionMinor;
	uint32_t timeOfLastConsitancyCheck;// POSIX time
	uint32_t intervalBetweenForcedConsistancyChecks;// POSIX time
	uint32_t osID;
	uint32_t versionMajor;
	uint16_t reservedBlocksUserID;
	uint16_t reservedBlocksGroupID;
	
	// 1.0+
	uint32_t firstNonReservedInode;
	uint16_t sizeOfInode;
	uint16_t thisBlockGroup;// For backup copy
	uint32_t optionalFeatures;
	uint32_t requiredFeatures;
	uint32_t readOnlyFlags;
	unsigned char fsID[16];
	char volumeName[16];
	char lastMountPoint[64];
	uint32_t compressionAlgorithm;
	uint8_t blocksPreallocatedForFiles;
	uint8_t blocksPreallocatedForDirectories;
	uint16_t reserved;
	unsigned char journalID[16];
	uint32_t journalInode;
	uint32_t journalDevice;
	uint32_t orphanInodeList;
} __attribute__((packed));
/*
off end size 			description
0	3	4	Total number of inodes in file system
4	7	4	Total number of blocks in file system
8	11	4	Number of blocks reserved for superuser (see offset 80)
12	15	4	Total number of unallocated blocks
16	19	4	Total number of unallocated inodes
20	23	4	Block number of the block containing the superblock
24	27	4	log2 (block size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the block size)
28	31	4	log2 (fragment size) - 10. (In other words, the number to shift 1,024 to the left by to obtain the fragment size)
32	35	4	Number of blocks in each block group
36	39	4	Number of fragments in each block group
40	43	4	Number of inodes in each block group
44	47	4	Last mount time (in POSIX time)
48	51	4	Last written time (in POSIX time)
52	53	2	Number of times the volume has been mounted since its last consistency check (fsck)
54	55	2	Number of mounts allowed before a consistency check (fsck) must be done
56	57	2	Ext2 signature (0xef53), used to help confirm the presence of Ext2 on a volume
58	59	2	File system state (see below)
60	61	2	What to do when an error is detected (see below)
62	63	2	Minor portion of version (combine with Major portion below to construct full version field)
64	67	4	POSIX time of last consistency check (fsck)
68	71	4	Interval (in POSIX time) between forced consistency checks (fsck)
72	75	4	Operating system ID from which the filesystem on this volume was created (see below)
76	79	4	Major portion of version (combine with Minor portion above to construct full version field)
80	81	2	User ID that can use reserved blocks
82	83	2	Group ID that can use reserved blocks

Extended Superblock Fields
These fields are only present if Major version (specified in the base superblock fields), is greater than or equal to 1.

84	87	4	First non-reserved inode in file system. (In versions < 1.0, this is fixed as 11)
88	89	2	Size of each inode structure in bytes. (In versions < 1.0, this is fixed as 128)
90	91	2	Block group that this superblock is part of (if backup copy)
92	95	4	Optional features present (features that are not required to read or write, but usually result in a performance increase. see below)
96	99	4	Required features present (features that are required to be supported to read or write. see below)
100	103	4	Features that if not supported, the volume must be mounted read-only see below)
104	119	16	File system ID (what is output by blkid)
120	135	16	Volume name (C-style string: characters terminated by a 0 byte)
136	199	64	Path volume was last mounted to (C-style string: characters terminated by a 0 byte)
200	203	4	Compression algorithms used (see Required features above)
204	204	1	Number of blocks to preallocate for files
205	205	1	Number of blocks to preallocate for directories
206	207	2	(Unused)
208	223	16	Journal ID (same style as the File system ID above)
224	227	4	Journal inode
228	231	4	Journal device
232	235	4	Head of orphan inode list
236	1023	X	(Unused)
*/


struct EXT2_BLOCK_GROUP_DESCRIPTOR
{
	uint32_t blockAddress_blockUsageBitmap;
	uint32_t blockAddress_inodeUsageBitmap;
	uint32_t blockAddress_inodeTable;
	uint16_t unallocatedBlocks;
	uint16_t unallocatedInodes;
	uint16_t directories;
	unsigned char unused[20];
} __attribute__((packed));

/*
 0	3	4	Block address of block usage bitmap
4	7	4	Block address of inode usage bitmap
8	11	4	Starting block address of inode table
12	13	2	Number of unallocated blocks in group
14	15	2	Number of unallocated inodes in group
16	17	2	Number of directories in group
18	31	X	(Unused)

 */
 
 
struct EXT2_INODE
{
	uint16_t typePermisions;
	uint16_t userID;
	uint32_t sizeLo;
	uint32_t lastAccessTime;
	uint32_t creationTime;
	uint32_t lastModificationTime;
	uint32_t deletionTime;
	uint16_t groupID;
	uint16_t hardLinkCount;
	uint32_t diskSectorCount;
	uint32_t flags;
	uint32_t OSSpecific1;
	uint32_t directBlock[12];
	uint32_t singly;
	uint32_t doubly;
	uint32_t triply;
	uint32_t gen;
	uint32_t extendedAttributeBlock;
	uint32_t sizeHi;
	uint32_t fragmentBlock;
	unsigned char OSSpecific2[12];
} __attribute__((packed));

/*
0	1	2	Type and Permissions (see below)
2	3	2	User ID
4	7	4	Lower 32 bits of size in bytes
8	11	4	Last Access Time (in POSIX time)
12	15	4	Creation Time (in POSIX time)
16	19	4	Last Modification time (in POSIX time)
20	23	4	Deletion time (in POSIX time)
24	25	2	Group ID
26	27	2	Count of hard links (directory entries) to this inode. When this reaches 0, the data blocks are marked as unallocated.
28	31	4	Count of disk sectors (not Ext2 blocks) in use by this inode, not counting the actual inode structure nor directory entries linking to the inode.
32	35	4	Flags (see below)
36	39	4	Operating System Specific value #1
40	43	4	Direct Block Pointer 0
44	47	4	Direct Block Pointer 1
48	51	4	Direct Block Pointer 2
52	55	4	Direct Block Pointer 3
56	59	4	Direct Block Pointer 4
60	63	4	Direct Block Pointer 5
64	67	4	Direct Block Pointer 6
68	71	4	Direct Block Pointer 7
72	75	4	Direct Block Pointer 8
76	79	4	Direct Block Pointer 9
80	83	4	Direct Block Pointer 10
84	87	4	Direct Block Pointer 11
88	91	4	Singly Indirect Block Pointer (Points to a block that is a list of block pointers to data)
92	95	4	Doubly Indirect Block Pointer (Points to a block that is a list of block pointers to Singly Indirect Blocks)
96	99	4	Triply Indirect Block Pointer (Points to a block that is a list of block pointers to Doubly Indirect Blocks)
100	103	4	Generation number (Primarily used for NFS)
104	107	4	In Ext2 version 0, this field is reserved. In version >= 1, Extended attribute block (File ACL).
108	111	4	In Ext2 version 0, this field is reserved. In version >= 1, Upper 32 bits of file size (if feature bit set) if it's a file, Directory ACL if it's a directory
112	115	4	Block address of fragment
116	127	12	Operating System Specific Value #2
*/


struct EXT2_DIR_ENTRY
{
	uint32_t inode;
	uint16_t entrySize;
	uint8_t nameLength;
	union{
		uint8_t type;
		uint8_t nameLengthHi;
	};
	char name[];
} __attribute((packed));

/*
0	3	4	Inode
4	5	2	Total size of this entry (Including all subfields)
6	6	1	Name Length least-significant 8 bits
7	7	1	Type indicator (only if the feature bit for "directory entries have file type byte" is set, else this is the most-significant 8 bits of the Name Length)
8	8+N-1	N	Name characters
*/


struct EXT2_DATA{
	struct EXT2_SUPERBLOCK superBlock;
	struct EXT2_INODE rootDirectoryInode;
	uint16_t blockSize;
} __attribute((packed));

bool ext2_DNOSIC_file_exists(struct PARTITION * p);
int getKernelInode(struct PARTITION * p, struct EXT2_INODE * inode);

int ext2_load_block(struct PARTITION * p, struct EXT2_INODE * inode, int block, void * dest);
int ext2_load_block_vmode(struct PARTITION * p, unsigned blockSize, struct EXT2_INODE * inode, int block, void * dest);


#endif
