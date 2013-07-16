/***************************************************************************
                       test.c  -  test program for libFB library
                             -------------------
    begin                : Mon Nov 20 2000
    copyright            : (C) 2000 by Daniele Venzano
    email                : webvenza@libero.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/
#include "FBlib.h"
#include <stdio.h>

int main()
{
	FB_pixel col,col1,col2;
	int bpp;

	FB_initlib("/dev/fb0");

	col = FB_makecol(0,0,255,0);

	col1 = FB_makecol(0,255,0,0);
	
	col2 = FB_makecol(255,0,0,0);
	bpp = FB_getbpp();
	FB_clear_screen(FB_makecol(0,0,0,0));
	FB_line(700,20,10,500,col);
	FB_hline(10,700,20,col1);
	FB_vline(10,20,500,col2);
	
        getchar();
        FB_exit();
	printf("Bpp used was: %d\n", bpp);
        return 0;
}
