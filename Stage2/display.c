/*
* License: Public Domain
*/


#include "display.h"

#define unlikely(x)     __builtin_expect((x),0)
#define likely(x)       __builtin_expect((x),1)

void display_put_character(uint8_t x, uint8_t y, char c, uint8_t a){
	((char *) (0xB8000))[(y * 80 + x) * 2] = c;
	((uint8_t *) (0xB8000))[(y * 80 + x) * 2 + 1] = a;
}

void display_place_string(uint8_t x, uint8_t y, const char * string, uint8_t attribute){
	while(*string != 0){
		((char *) (0xB8000))[(y * 80 + x) * 2] = *string;
		((uint8_t *) (0xB8000))[(y * 80 + x) * 2 + 1] = attribute;
		x++;
		if(x >= 80){
			y++;
			x = 0;
		}
		if(unlikely(y >= 25)){
			return;
		}
		string++;
	}
}

void display_clear(uint8_t attribute){
	uint16_t v = ' ' | (attribute << 8);
	for(int i = 0; i < 2000; i++)
		((uint16_t *) (0xB8000))[i] = v;
}

char tbuf[32];
//char bchars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
extern char bchars[16];
void itoa(unsigned i, unsigned base, char * buf) {
	int pos = 0;
	int opos = 0;
	int top = 0;

	if (i == 0 || base > 16) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	while (i != 0) {
		tbuf[pos] = bchars[i % base];
		pos++;
		i /= base;
	}
	top = pos--;
	for (opos = 0; opos < top; pos--, opos++) {
		buf[opos] = tbuf[pos];
	}
	buf[opos] = 0;
}

