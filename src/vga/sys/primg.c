
#include "primg.h"


uint16_t get_unit16(uint8_t * in)
{
    uint16_t x = (*(in+1) *  256) | *in;
    return x;
}
void vga_drawline(uint16_t struct_address)
{ 
primg_x0 = get_unit16(xram+struct_address);
uint16_t x1 = get_unit16(xram+struct_address+2); 
primg_y0 = get_unit16(xram+struct_address+4); 
uint16_t y1 = get_unit16(xram+struct_address+6); 
primg_color =  xram[struct_address+8];
/*
i could not seem to figure out how to cast the address coming in as an int to a pointer address to my structure type
everything I seemed to try came in with some other address , not something im familar with any suggestions would be apprciated
xram (pointer) + struct_address (int)  , comes out to the correct address 
 
i just had to pull the values from memory in the order they are defined on the client side 6502 structure

*/

int16_t dx =  abs (x1 - primg_x0), sx = primg_x0 < x1 ? 1 : -1;
int16_t dy = -abs (y1 - primg_y0), sy = primg_y0 < y1 ? 1 : -1; 
int16_t err = dx + dy, e2; /* error value e_xy */
 
  while(1)
  {  
    setPixel(primg_x0,primg_y0,primg_color);
    if (primg_x0 == x1 && primg_y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; primg_x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; primg_y0 += sy; } /* e_xy+e_y < 0 */
  }
}// end drawline
 
void setPixel() 
{
    // insert primg_color @ primg_x0, primg_y0
    uint16_t offset = (primg_x0 / 2) + (primg_y0 * (320 / 2));
    if (primg_x0 % 2)
     xram[offset] = (xram[offset] & 15) | (primg_color << 4);
    else
      xram[offset] = (xram[offset] & 240) | primg_color;
}


void plot_circle (uint16_t struct_address)
{
primg_x0 = get_unit16(xram+struct_address);
primg_y0 = get_unit16(xram+struct_address+2);
uint16_t r   = get_unit16(xram+struct_address+4);
primg_color =  xram[struct_address+6];

   int16_t x = -r, y = 0, err = 2-2*r; /* II. Quadrant */ 
   do {
      setPixel (primg_x0-x, primg_y0+y,primg_color); /*   I. Quadrant */
      setPixel (primg_x0-y, primg_y0-x,primg_color); /*  II. Quadrant */
      setPixel (primg_x0+x, primg_y0-y,primg_color); /* III. Quadrant */
      setPixel (primg_x0+y, primg_y0+x,primg_color); /*  IV. Quadrant */
      r = err;
      if (r >  x) err += ++x*2+1; /* e_xy+e_x > 0 */
      if (r <= y) err += ++y*2+1; /* e_xy+e_y < 0 */
   } while (x < 0);
}