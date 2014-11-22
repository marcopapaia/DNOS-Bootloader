/*
* License: Public Domain
*/


#ifndef _elf_h
#define _elf_h

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t Elf32_Half;	// Unsigned half int
typedef uint32_t Elf32_Off;		// Unsigned offset
typedef uint32_t Elf32_Addr;	// Unsigned address
typedef uint32_t Elf32_Word;	// Unsigned int
typedef int32_t  Elf32_Sword;	// Signed int

#define ELF_NIDENT 16

struct Elf32_Ehdr {
	uint8_t		e_ident[ELF_NIDENT];
	Elf32_Half	e_type;
	Elf32_Half	e_machine;
	Elf32_Word	e_version;
	Elf32_Addr	e_entry;
	Elf32_Off	e_phoff;
	Elf32_Off	e_shoff;
	Elf32_Word	e_flags;
	Elf32_Half	e_ehsize;
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;
	Elf32_Half	e_shstrndx;
} __attribute__((packed));

enum Elf_Ident {
	EI_MAG0		= 0, // 0x7F
	EI_MAG1		= 1, // 'E'
	EI_MAG2		= 2, // 'L'
	EI_MAG3		= 3, // 'F'
	EI_CLASS	= 4, // Architecture (32/64)
	EI_DATA		= 5, // Byte Order
	EI_VERSION	= 6, // ELF Version
	EI_OSABI	= 7, // OS Specific
	EI_ABIVERSION	= 8, // OS Specific
	EI_PAD		= 9  // Padding
};
 
# define ELFMAG0	0x7F // e_ident[0]
# define ELFMAG1	'E'  // e_ident[1]
# define ELFMAG2	'L'  // e_ident[2]
# define ELFMAG3	'F'  // e_ident[3]
 
# define ELFDATA2LSB 1  // Little Endian
# define ELFCLASS32	 1  // 32-bit Architecture

enum Elf_Type {
	ET_NONE		= 0, // Unkown Type
	ET_EXEC		= 2  // Executable File
};
 
# define EM_386	3  // x86 Machine Type

struct Elf32_Shdr {
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;// Virtual
	Elf32_Off	sh_offset;// File
	Elf32_Word	sh_size;// Bytes
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} __attribute__((packed));

enum ShT_Types {
	SHT_NULL	= 0,   // Null section
	SHT_PROGBITS= 1,   // Program information
	SHT_NOBITS	= 8,   // Not present in file
};
 
enum ShT_Attributes {
	SHF_WRITE	= 0x01, // Writable section
	SHF_ALLOC	= 0x02  // Exists in memory
};

bool elf_check_header(struct Elf32_Ehdr * header);

// 0 = unsupported, 1 = x86, 2 will be amd64
int elf_get_type(struct Elf32_Ehdr * header);

#endif
