; License: Public Domain

bits 32

section .text

; void _cdecl outb(unsigned short port, unsigned char data)

global outb
outb:
	mov al, BYTE[esp+8]
	mov dx, WORD[esp+4]
	out dx, al
	ret
	
; unsigned char _cdecl inb(unsigned short port);

global inb
inb:
	xor eax, eax
	mov dx, WORD[esp+4]
	in al, dx
	ret
	
	
;void outw(unsigned short port, unsigned char data);
global outw
outw:
	mov ax, WORD[esp+8]
	mov dx, WORD[esp+4]
	out dx, ax
	ret


;unsigned short inw(unsigned short port);

global inw
inw:
	xor eax, eax
	mov dx, WORD[esp+4]
	in ax, dx
	ret

