


#ifndef _PRIMG_H_
#define _PRIMG_H_

#include <stdint.h>
#include <stdlib.h>
#include "xram.h"

#include <stdio.h>



uint16_t get_unit16(uint8_t * in);


void setPixel(uint16_t x, uint16_t y,uint8_t color);
void vga_drawline(uint16_t struct_address);
void plot_circle (uint16_t struct_address);


#endif