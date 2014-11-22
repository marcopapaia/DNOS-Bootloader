/*
* License: Public Domain
*/


#include "pit.h"
#include <stdint.h>
#include <stdbool.h>
#include "pic.h"
#include "idt.h"
#include "io.h"

#define PIT_REG_COUNTER0		0x40
#define PIT_REG_COUNTER1		0x41
#define PIT_REG_COUNTER2		0x42
#define PIT_REG_COMMAND			0x43

#define	PIT_OCW_MODE_3			0x6	//00000110
#define	PIT_OCW_COUNTER			0xC0 //11000000
#define	PIT_OCW_RL_DATA			0x30 //110000

void pit_send_command(uint8_t cmd) {
	outb(PIT_REG_COMMAND, cmd);
}

void pit_send_data(uint8_t data, uint8_t counter) {
	uint8_t port;
	if (counter == 0){
		port = PIT_REG_COUNTER0;
	}
	else if (counter == 1){
		port = PIT_REG_COUNTER1;
	}
	else {
		port = PIT_REG_COUNTER2;
	}

	outb(port, data);
}

void timer_handler();

void pit_init(){
	idt_install_isr(32, (void *)timer_handler);

	uint16_t divisor = (uint16_t)(1193181 / (uint16_t)20);

	// send ocw
	uint8_t ocw = 0;
	ocw = PIT_OCW_MODE_3;
	ocw |= PIT_OCW_RL_DATA;
	pit_send_command(ocw);

	// set frequency rate
	pit_send_data(divisor & 0xFF, 0);
	pit_send_data((divisor >> 8) & 0xFF, 0);

	pic_mask_interrupt(0, false);
}
