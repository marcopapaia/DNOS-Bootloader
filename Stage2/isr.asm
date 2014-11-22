; License: Public Domain

bits 32

;extern display_place_string

section .text

global INTERRUPT_HANDLER_DEFAULT
INTERRUPT_HANDLER_DEFAULT:
	iretd
	
global PIC_LOWER_INTERRUPT_IGNORE_INTERRUPT
PIC_LOWER_INTERRUPT_IGNORE_INTERRUPT:
	push eax
	mov al, 0x20
	out 0x20, al
	pop eax
	iretd


global PIC_HIGHER_INTERRUPT_IGNORE_INTERRUPT
PIC_HIGHER_INTERRUPT_IGNORE_INTERRUPT:
	push eax
	mov al, 0x20
	out 0x20, al
	out 0xA0, al
	pop eax
	iretd

global timer_handler
timer_handler:
	push eax
	
	extern timerTicks
	add DWORD[timerTicks], 20

	mov al, 20h
	out 20h, al

	pop eax
	iretd

global install_idt
install_idt:
	extern idtr
	lidt [idtr]
	ret



	
;section .rodata
;str_unhandled_int: db "Unhandled interrupt!", 0

section .bss
global idt
resb 2048


