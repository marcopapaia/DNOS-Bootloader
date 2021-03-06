/*
* License: Public Domain
*/


#include "idt.h"
#include <stdint.h>
#include <stdbool.h>

void INTERRUPT_HANDLER_DEFAULT();

#define IDT_DESC_GATE_INTERRUPT	0xE // ...1110
#define IDT_DESC_GATE_TRAP		0xF
#define IDT_DESC_GATE_TASK		0x5

#define IDT_DESC_RING_0		0
#define IDT_DESC_RING_1		0x40 //01000000
#define IDT_DESC_RING_2		0x20 //00100000
#define IDT_DESC_RING_3		0x60 //01100000

#define IDT_DESC_PRESENT	0x80 //10000000

struct IDT_DESCRIPTOR
{
	uint16_t offsetLow;
	uint16_t selector;
	uint8_t reserved;
	uint8_t typeAndAttributes;
	uint16_t offsetHigh;
} __attribute__((packed));

struct IDTR
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct IDTR idtr;

struct IDT_DESCRIPTOR idt[256];

void idt_install_isr(int i, void * irq){
	uint32_t pointer = (uint32_t)irq;
	idt[i].offsetLow = (uint16_t)(pointer & 0xFFFF);
	idt[i].offsetHigh = (uint16_t)((pointer >> 16) & 0xffff);
	idt[i].reserved = 0;
	idt[i].typeAndAttributes = IDT_DESC_GATE_INTERRUPT | IDT_DESC_PRESENT;
	idt[i].selector = 8;
}

void install_idt();


void idt_init(){
	for (unsigned i = 0; i < 256; i++){
		idt_install_isr(i, (void *)INTERRUPT_HANDLER_DEFAULT);
	}

	idtr.limit = 2048;
	idtr.base = (uint32_t)&idt[0];
	
	install_idt();
}
