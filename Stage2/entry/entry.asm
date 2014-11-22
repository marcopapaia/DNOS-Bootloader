; License: Public Domain

bits 16
org 0x1000

entry:
	cli

	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0xFFFC
	
	mov ah, 0x0e
	mov al, '1'
	int 10h
	
	call A20_enable
	
	mov ah, 0x0e
	mov al, '2'
	int 10h
  
	call InstallGDT
	
	mov ah, 0x0e
	mov al, '3'
	int 10h
  
	call GetTotalRAM
	
	mov ah, 0x0e
	mov al, '4'
	int 10h
	
	call LoadMemoryMapEntries
	
	mov ah, 0x0e
	mov al, '5'
	int 10h
	
	jmp GoProtected
	
global totalRAM
totalRAM dd 0

global totalMemMapEntries
totalMemMapEntries dw 0

%include "A20.asm"
%include "GDT.asm"
%include "memory.asm"
%include "protected.asm"
