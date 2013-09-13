/*
 * vgatest.c
 *
 *  Created on: 20 de Jul de 2012
 *      Author: fabio32883
 */

#include "vga/vga.h"
#include "vga/ezdib/ezdib.h"
#include "input/input.h"
#include <stdio.h>
#include <sys/time.h>


int main()
{

	VGA_init();

	//VGA_fill(VGA_get16bpp_color(90,90,100) );

	VGA_rect(10,10,VGA_vinfo.xres-10, VGA_vinfo.yres-10,VGA_get16bpp_color(100,100,100));



	VGA_drawImage("back.bmp",0,0);
/*

	VGA_drawImage("fabio.bmp",50,100);
	VGA_drawImage_Transparent("fabio.bmp",460,100,-1);

	BITMAP * bmp;
	bmp = VGA_loadBitmap("isel.bmp");
	VGA_drawBitmap_Transparent(bmp,200,70,bmp->data[0]);
	VGA_drawBitmap(bmp,200,200);

	VGA_setFontSize(24);
	VGA_drawButton("   Iniciar   ",200,350);

	VGA_fill_circle(50,50,40,COLOR_PINK);

	VGA_fill_rect(300,300,320,320,COLOR_RED);

	VGA_setFontSize(24);
*/

	printf("END\r\n");
	VGA_exit();
	return 0;

}
