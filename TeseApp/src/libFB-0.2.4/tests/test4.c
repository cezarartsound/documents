/***************************************************************************
                       test2.c  -  test putpixel speed
                             -------------------
    begin                : Sat Feb 16 2002
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
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define DEGREES M_PI/12
#define LENGTH 100

int main()
{
	FB_pixel col;
	double deg=0;
	int x, xfont, y, yfont, centerx, centery;
	
	FB_initlib("/dev/fb0");
	srandom(time(NULL));
	FB_getres(&x, &y);
	y -= 20;
	yfont = y+2;
	xfont = 10;
	FB_rectfill(0, y, x, y+20, FB_makecol(0,0,0,0));
	centerx = x/2;
	centery = y/2;
	while(deg <= 2*M_PI)
	{
		col=FB_makecol(random()%255,random()%255,random()%255,random()%255);
		FB_line(centerx, centery, centerx+(LENGTH*cos(deg)), centery+(LENGTH*sin(deg)), col);
		deg+=DEGREES;
	}
	getchar();
	FB_clear_screen(FB_makecol(0,0,0,0));
	FB_exit();
	return 0;
}
