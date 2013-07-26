/*
 * vga.h
 *
 *  Created on: 1 de Out de 2012
 *      Author: fabio32883
 */

#ifndef VGA_H_
#define VGA_H_

#include <linux/fb.h>


#define COLOR_WHITE 		0xffff
#define COLOR_BLACK 		0x0
#define COLOR_RED 			0xf800
#define COLOR_BLUE 			0x1f
#define COLOR_GREEN 		0x7e0
#define COLOR_LIGHT_BLUE 	0x731f
#define COLOR_DARK_BLUE 	0x1014
#define COLOR_LIGHT_GRAY 	0xad95
#define COLOR_DARK_GRAY  	0x4a49
#define COLOR_PINK 			0xfbf7
#define COLOR_ORANGE 		0xfbe0
#define WIN_COLOR			VGA_get16bpp_color(80, 80, 90)

extern struct fb_var_screeninfo VGA_vinfo;

#ifndef _BYTE_
#define _BYTE_
typedef unsigned char byte;
#endif /* _BYTE_ */

#ifndef _WORD_
#define _WORD_
typedef unsigned short word;
#endif /* _WORD_ */

typedef struct tagBITMAP              /* the structure for a bitmap. */
{
  word width;
  word height;
  word*data;
} BITMAP;


void VGA_init();
void VGA_exit();

#define DEFAULT_FONT_SIZE 18
#define DEFAULT_FONT_NAME ("fonts/lutRS00.bdf")

//Font* VGA_loadFont(char* font_name); // só 8, 10, 12, 14, 18 , 19 e 24
int VGA_setFontSize(int size); // se nao existir o tamanho fica 24 por default

void VGA_paint(int x, int y,int w, int h, int r, int g, int b);
void VGA_set_pixel( int x, int y, int color);
#define VGA_get16bpp_color(r, g, b) (((r*31)/100)<<11) | (((g*63)/100)<<5) | ((b*31)/100)
unsigned short VGA_8bpp_to_16bpp(byte b8);

int VGA_drawButton_end(char* text,int xpos, int ypos); // nao desenha apenas retorna x final
int VGA_drawButton( char* text,int xpos, int ypos);
void VGA_drawButtonClick( char * text, int xpos, int ypos);
void VGA_drawText(char*text, int xpos, int ypos, int color);

void VGA_line(int x1,int y1, int x2, int y2, int color);
void VGA_circle(int x, int y, int r, int color);
void VGA_fill_circle(int x, int y, int r, int color);
void VGA_rect(int x1,int y1, int x2, int y2, int color);
void VGA_fill_rect(int x1,int y1, int x2, int y2, int color);
void VGA_fill(int color);

BITMAP * VGA_loadBitmap(char * image_name);
void VGA_destroyBitmap(BITMAP* bmp);
void VGA_drawBitmap(BITMAP *bmp,int x,int y);
void VGA_drawBitmap_Transparent(BITMAP *bmp,int x,int y, int transp_color);
void VGA_drawImage(char * image_name, int x, int y);
void VGA_drawImage_Transparent(char * image_name, int x, int , int transp_color); //-1 para o primeiro pixel




// Funcao generica de set_pixel, tem de se alterar pUser_ref e color_ref antes da chamada
extern int color_ref;
void set_pixel_ref(int x, int y);

#endif /* VGA_H_ */
