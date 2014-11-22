/*
* License: Public Domain
*/


#ifndef _IDT_H
#define _IDT_H

void idt_init();
void idt_install_isr(int i, void * irq);

#endif
