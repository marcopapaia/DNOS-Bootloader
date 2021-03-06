/*
* License: Public Domain
*/


#ifndef _PIC_H
#define _PIC_H

#include <stdbool.h>
#include <stdint.h>

void pic_init();
void pic_mask_interrupt(uint8_t IRQ, bool mask);

#endif
