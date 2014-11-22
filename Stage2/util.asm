section .text
;char bchars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
global bchars
bchars:
	db "0123456789ABCDEF"
	
	
global enablePaging:
enablePaging:
	extern pdt
	mov eax, DWORD[pdt]
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	ret

global disablePaging:
disablePaging:
	mov eax, cr0
	and eax, 0x7FFFFFFF
	mov cr0, eax
	ret

;void enterKernel(unsigned address);
extern bootInfo
global enterKernel
enterKernel:
	mov eax, 0xABBA
	mov ebx, bootInfo
	mov edx, [esp+4]
	jmp edx
