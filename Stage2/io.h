/*
* License: Public Domain
*/


#ifndef _IO_H
#define _IO_H

void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

unsigned short inw(unsigned short port);
void outw(unsigned short port, unsigned char data);


#endif
