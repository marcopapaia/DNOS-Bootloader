/*
* License: Public Domain
*/


#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <stdint.h>

void display_put_character(uint8_t x, uint8_t y, char c, uint8_t a);

/**
* Place a string at the given coordinates
* @param x,y Coordinates of string (text mode character cell)
* @param string Pointer to null (0) terminated string
* @param attribute The text mode attribute to give all characters in the string
* 		 This parameter sets the colour and enables/disables blinking
*/
void display_place_string(uint8_t x, uint8_t y, const char * string, uint8_t attribute);

void display_clear(uint8_t attribute);

void itoa(unsigned i, unsigned base, char * buf);

#endif
