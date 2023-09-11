
#include "primg.h"
uint16_t primg_x0;
uint16_t primg_y0;
uint8_t primg_color;

uint16_t get_unit16(uint8_t * in)
{
    uint16_t x = (*(in+1) *  256) | *in;
    return x;
}


void vga_drawline(uint16_t struct_address)
{ 
primg_x0 = *((uint16_t*)(xram+struct_address));
primg_y0 = *((uint16_t*)(xram+struct_address+2));
uint16_t x1 = *((uint16_t*)(xram+struct_address+4));
uint16_t y1 = *((uint16_t*)(xram+struct_address+6));

primg_color =  xram[struct_address+8];
//printf("server side drawline(%d,%d,%d,%d,%d)\n",primg_x0,primg_y0,x1,y1,primg_color);
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
 
void setPixel(uint16_t x, uint16_t y,uint8_t color) 
{
 // printf("%d, %d ,%d \n",primg_x0,primg_y0,primg_color);
    // insert primg_color @ primg_x0, primg_y0
    uint16_t offset = (x / 2) + (y * (320 / 2));
    if (x % 2)
     xram[offset] = (xram[offset] & 15) | (color << 4);
    else
      xram[offset] = (xram[offset] & 240) | color;
}


void plot_circle (uint16_t struct_address)
{
 uint16_t x0= *((uint16_t*)(xram+struct_address));
 uint16_t y0 = *((uint16_t*)(xram+struct_address+2));
 uint16_t radius = *((uint16_t*)(xram+struct_address+4));
 uint16_t color =  xram[struct_address+6];
 
 int16_t f = 1 - radius;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * radius;
  int16_t x = 0;
  int16_t y = radius;
 
  setPixel (x0, y0 + radius,color);
  setPixel (x0, y0 - radius,color);
  setPixel (x0 + radius, y0,color);
  setPixel (x0 - radius, y0,color);
  while (x < y)
  {
    // ddF_x == 2 * x + 1;
    // ddF_y == -2 * y;
    // f == x*x + y*y - radius*radius + 2*x - y + 1;
    if (f >= 0) 
    {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;    
    setPixel (x0 + x, y0 + y,color);
    setPixel (x0 - x, y0 + y,color);
    setPixel (x0 + x, y0 - y,color);
    setPixel (x0 - x, y0 - y,color);
    setPixel (x0 + y, y0 + x,color);
    setPixel (x0 - y, y0 + x,color);
    setPixel (x0 + y, y0 - x,color);
    setPixel (x0 - y, y0 - x,color);
  }
}
