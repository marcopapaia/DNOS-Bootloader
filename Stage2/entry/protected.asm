; License: Public Domain

bits 16

GoProtected:
	cli
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax
	
	jmp 8:.bit32
	
	bits 32
	.bit32:
	
	mov	ax, 10h
	mov	ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov	ss, ax
	mov	esp, 9B000h

	; File runs straight into main.c
