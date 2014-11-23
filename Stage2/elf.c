/*
* License: Public Domain
*/


#include "elf.h"

bool elf_check_header(struct Elf32_Ehdr * header)
{
	if(header->e_ident[EI_MAG0] != ELFMAG0 ||
	header->e_ident[EI_MAG1] != ELFMAG1 ||
	header->e_ident[EI_MAG2] != ELFMAG2 ||
	header->e_ident[EI_MAG3] != ELFMAG3){
			return false;
	}	
	return true;
}

// 0 = unsupported, 1 = x86, 2 will be amd64
int elf_get_type(struct Elf32_Ehdr * header)
{
	if(header->e_machine != EM_386 && header->e_machine != EM_AMD64) {
		return 0;
	}
	if(header->e_ident[EI_CLASS] != ELFCLASS32 && header->e_ident[EI_CLASS] != ELFCLASS64) {
		return 0;
	}	
	if(header->e_ident[EI_DATA] != ELFDATA2LSB) {
		return 0;
	}
	if(header->e_type != ET_EXEC) {
		return 0;
	}
	return header->e_ident[EI_CLASS];
}
