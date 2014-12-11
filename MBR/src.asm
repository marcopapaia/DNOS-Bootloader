bits 16
org 0x600

ENTRY:
cli
xor ax, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
 
mov ax, 6000h
mov ss, ax
mov sp, 0

push dx

cld
sti

mov di, 0x600; Destination
mov si, 0x7C00; Source
mov cx, 512
rep movsb

jmp 0:AFTER_MOVING_CODE

AFTER_MOVING_CODE:

pop dx
mov BYTE[driveNum], dl

; Check for int 13h extensions support

mov ah, 0x41
mov bx, 0x55AA
mov dl, 80h
int 13h
jc FAILURE

partition_selection:

mov si, selectPartition
call print

; * Wait for keypress * ;
xor ah, ah
int 16h

; ASCII Character is now in AL ;

; Check if valid (1-4) or 'd'

cmp al, '4'
je partition4

cmp al, '3'
je partition3

cmp al, '2'
je partition2

cmp al, '1'
je partition1

cmp al, 'd'
je goDNOSbootloader

jmp partition_selection

; * Check parittion is valid and load starting sector * ;

partition1:
cmp BYTE[PARTITION_1_status], 80h
jg partition_selection
mov eax, DWORD[PARTITION_1_LBA_start]
mov DWORD[INT13INFO_sect], eax
mov bp, PARTITION_1
jmp load_partition

partition2:
cmp BYTE[PARTITION_2_status], 80h
jg partition_selection
mov eax, DWORD[PARTITION_2_LBA_start]
mov DWORD[INT13INFO_sect], eax
mov bp, PARTITION_2
jmp load_partition

partition3:
cmp BYTE[PARTITION_3_status], 80h
jg partition_selection
mov eax, DWORD[PARTITION_3_LBA_start]
mov DWORD[INT13INFO_sect], eax
mov bp, PARTITION_3
jmp load_partition

partition4:
cmp BYTE[PARTITION_4_status], 80h
jg partition_selection
mov eax, DWORD[PARTITION_4_LBA_start]
mov DWORD[INT13INFO_sect], eax
mov bp, PARTITION_4
jmp load_partition

goDNOSbootloader:
mov DWORD[INT13INFO_blkcnt], 62
mov WORD[INT13INFO_addr_off], 0x1000
;jmp load_partition


load_partition:
push bp

; * Read first sector of parition * ;

mov ah, 42h
mov dl, BYTE[driveNum]
mov si, INT13INFO
int 13h
jc FAILURE

or ah, ah
jne FAILURE

;ds:bp = MBR partition struct pointer
pop bp

; * Execute boot sector of partition * ;

cli
jmp WORD[INT13INFO_addr_off]


; -- Data/Functions --

align 4
INT13INFO:
INT13INFO_size		db	0x10
INT13INFO_0			db	0
INT13INFO_blkcnt:	dw	1
INT13INFO_addr_off:	dw	0x7C00	
INT13INFO_addr_seg:	dw	0x0000
INT13INFO_sect:		dd 	1
INT13INFO_sect_hi:	dd	0
					
driveNum db 0x80


;	Prints a string
;	DS:SI = null (0) terminated string
print:
lodsb
or al, al
jz .PrintDone
mov	ah, 0Eh
int	10h
jmp	print
.PrintDone:
	ret
 
 

FAILURE:
mov si, failure_msg
call print
cli
hlt

failure_msg: db "MBR failure", 0
selectPartition: db "Select Partition (1-4 or 'd')", 0x0A, 0x0D, 0

	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


TIMES 446-($-$$) DB 0

PARTITION_1:
PARTITION_1_status 			db 80h
PARTITION_1_chs_start_0 	db 0
PARTITION_1_chs_start_1 	db 0
PARTITION_1_chs_start_2 	db 0
PARTITION_1_partition_type	db 0
PARTITION_1_chs_end_0 		db 0
PARTITION_1_chs_end_1 		db 0
PARTITION_1_chs_end_2 		db 0
PARTITION_1_LBA_start		dd 63
PARTITION_1_LBA_size		dd 16384


PARTITION_2:
PARTITION_2_status 			db 0
PARTITION_2_chs_start_0 	db 0
PARTITION_2_chs_start_1 	db 0
PARTITION_2_chs_start_2 	db 0
PARTITION_2_partition_type	db 0
PARTITION_2_chs_end_0 		db 0
PARTITION_2_chs_end_1 		db 0
PARTITION_2_chs_end_2 		db 0
PARTITION_2_LBA_start		dd 0
PARTITION_2_LBA_size		dd 0


PARTITION_3:
PARTITION_3_status 			db 0
PARTITION_3_chs_start_0 	db 0
PARTITION_3_chs_start_1 	db 0
PARTITION_3_chs_start_2 	db 0
PARTITION_3_partition_type	db 0
PARTITION_3_chs_end_0 		db 0
PARTITION_3_chs_end_1 		db 0
PARTITION_3_chs_end_2 		db 0
PARTITION_3_LBA_start		dd 0
PARTITION_3_LBA_size		dd 0


PARTITION_4:
PARTITION_4_status 			db 0
PARTITION_4_chs_start_0 	db 0
PARTITION_4_chs_start_1 	db 0
PARTITION_4_chs_start_2 	db 0
PARTITION_4_partition_type	db 0
PARTITION_4_chs_end_0 		db 0
PARTITION_4_chs_end_1 		db 0
PARTITION_4_chs_end_2 		db 0
PARTITION_4_LBA_start		dd 0
PARTITION_4_LBA_size		dd 0

DW 0xAA55
