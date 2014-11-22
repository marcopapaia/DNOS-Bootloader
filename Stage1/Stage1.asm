; License: Public Domain

bits 16
org 0x7C00

ENTRY: jmp START
bpb_DriveNum db 0x80

INT13INFO:
INT13INFO_size		db	0x10
INT13INFO_0			db	0
INT13INFO_blkcnt:	dw	20
INT13INFO_addr_off:	dw	0x1000
INT13INFO_addr_seg:	dw	0
INT13INFO_sect:		dd 	0
INT13INFO_sect_hi:	dd	0
				
					
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		;;;;;;;;;;;;;;;;;;;;;;;		;;;;;;;;;;;
START:

cli
xor ax, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
 
mov ss, ax
mov sp, 0xFFFC

cld

mov BYTE[bpb_DriveNum], dl

xor ah, ah
mov al, 3
int 10h; Just to be sure we're in text mode.

sti	

mov ah, 0x41
mov bx, 0x55AA
mov dl, 0x80
int 13h
jc FAILURE


xor ah, ah
mov al, BYTE[DATA_STAGE2_SIZE]
mov WORD[INT13INFO_blkcnt], ax

mov edx, DWORD [DATA_STAGE2_START]
mov DWORD [INT13INFO_sect], edx

mov edx, DWORD [DATA_STAGE2_START_HI]
mov DWORD [INT13INFO_sect_hi], edx


mov dl, BYTE[bpb_DriveNum]

mov si, INT13INFO
mov ah, 0x42

int 13h
jc FAILURE



mov dl, BYTE[bpb_DriveNum]

jmp 0x0:0x1000

FAILURE:
  mov al, 'F'
  mov ah, 0Eh
  int 10h
  cli
  hlt

TIMES 501-($-$$) DB 0x20

DATA_STAGE2_START: dd 5122; Sector (LBA)
DATA_STAGE2_START_HI dd 0; Upper 32 bits
DATA_STAGE2_SIZE db 40; SECTORS

DW 0xAA55
