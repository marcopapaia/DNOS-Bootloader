/*
* License: Public Domain
*/


ENTRY(_start)

SECTIONS
{
	
	/* Start at 0x1000 plus the size of the entry code */
	. = 0x1000 + 429;
	
	.text BLOCK(1) : ALIGN(1)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(1) : ALIGN(1)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(1) : ALIGN(1)
	{
		*(.data)
	}

	.bss BLOCK(1) : ALIGN(1)
	{
		*(.bss)
	}
	
}
