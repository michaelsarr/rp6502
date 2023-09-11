


#ifndef _PRIMG_H_
#define _PRIMG_H_

#include <stdint.h>
#include <stdlib.h>
#include "xram.h"
uint16_t primg_x0;
uint16_t primg_y0;
uint8_t primg_color;

uint16_t get_unit16(uint8_t * in);


void setPixel();
void vga_drawline(uint16_t struct_address);
void plot_circle (uint16_t struct_address);


#endif