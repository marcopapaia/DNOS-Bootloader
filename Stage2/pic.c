/*
* License: Public Domain
*/


#include "pic.h"
#include "io.h"
#include <stdint.h>
#include <stdbool.h>
#include "idt.h"

#include "display.h"

#define PIC1_PORT			0x20		// IO base address for master PIC
#define PIC2_PORT			0xA0		// IO base address for slave PIC
#define PIC1_COMMAND_PORT	PIC1_PORT
#define PIC1_DATA_PORT		(PIC1_PORT+1)
#define PIC2_COMMAND_PORT	PIC2_PORT
#define PIC2_DATA_PORT		(PIC2_PORT+1)
#define PIC_EOI				0x20

#define PIC_WAIT_TIME 10


void PIC_LOWER_INTERRUPT_IGNORE_INTERRUPT();
void PIC_HIGHER_INTERRUPT_IGNORE_INTERRUPT();

void outportb(unsigned short port, unsigned char data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}

void pic_init(){
	__asm__ ("cli");
	
	for (int i = 0; i < 8; i++){
		idt_install_isr(32 + i, (void *)PIC_LOWER_INTERRUPT_IGNORE_INTERRUPT);
	}
	for (int i = 8; i < 16; i++){
		idt_install_isr(32 + i, (void *)PIC_HIGHER_INTERRUPT_IGNORE_INTERRUPT);
	}	

	// ICW 1
	
	outb(PIC1_COMMAND_PORT, 0x11);
	for (int i = 0; i < PIC_WAIT_TIME; i++);
	
	outb(PIC2_COMMAND_PORT, 0x11);
	for (int i = 0; i < PIC_WAIT_TIME; i++);

	// ICW 2: PIC IDT index
	
	outb(PIC1_DATA_PORT, 32);
	for (int i = 0; i < PIC_WAIT_TIME; i++);
	outb(PIC2_DATA_PORT, 40);
	for (int i = 0; i < PIC_WAIT_TIME; i++);

	// ICW 3: Inform PICs of other PICs existance
	
	outb(PIC1_DATA_PORT, 4);
	for (int i = 0; i < PIC_WAIT_TIME; i++);
	outb(PIC2_DATA_PORT, 2);
	for (int i = 0; i < PIC_WAIT_TIME; i++);

	// ICW 4: Tell the PICs how to operate
	
	outb(PIC1_DATA_PORT, 1);
	for (int i = 0; i < PIC_WAIT_TIME; i++);
	outb(PIC2_DATA_PORT, 1);
	for (int i = 0; i < PIC_WAIT_TIME; i++);

	outb(PIC1_DATA_PORT, 0xFF);   // No interrupts
	outb(PIC2_DATA_PORT, 0xFF);	
}

void pic_mask_interrupt(uint8_t IRQ, bool mask){
	uint16_t port;
	uint8_t value;

	if (IRQ < 8) {
		port = PIC1_DATA_PORT;
	} else {
		port = PIC2_DATA_PORT;
		IRQ -= 8;
	}
	if (!mask){
		value = inb(port) & ~(1 << IRQ);
	} else {
		value = inb(port) | (1 << IRQ);
	}
	outb(port, value);
}
