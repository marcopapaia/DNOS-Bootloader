; License: Public Domain


bits 16

A20_enable:
	cli
 
	call A20_wait_out
	mov al, 0xAD
	out 0x64, al
 
	call A20_wait_out
	mov al, 0xD0
	out 0x64, al
 
	call A20_wait_out_in
	in al, 0x60
	push eax
 
	call A20_wait_out
	mov al, 0xD1
	out 0x64, al
 
	call A20_wait_out
	pop eax
	or al, 2
	out 0x60, al
 
	call A20_wait_out
	mov al, 0xAE
	out 0x64, al
 
	call A20_wait_out
		
	sti
	ret
 
A20_wait_out:; Wait for input buffer to clear to output
	in al, 0x64
	test al, 2
	jnz A20_wait_out
	ret
 
 
A20_wait_out_in:; Wait for output buffer to clear to input
	in al,0x64
	test al,1
	jz A20_wait_out_in
	ret
