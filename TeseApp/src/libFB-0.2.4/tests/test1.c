/***************************************************************************
                       test1.c  -  test putpixel speed
                             -------------------
    begin                : Wed Dec 27 2000
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
#include <time.h>

#define MAX_SECS 30
#define UPDATE_INTERVAL 5

int update;

void sig_alarm(int sig)
{
	update=1;
	
}

int main()
{
	FB_pixel col;
	int x, xfont, y, yfont;
	long start_time, cur_time;
	long num_pixels = 0;
	
	FB_initlib("/dev/fb0");
	signal(SIGALRM, sig_alarm);
	srandom(time(NULL));
	FB_getres(&x, &y);
	y -= 20;
	yfont = y+2;
	xfont = 10;
	FB_rectfill(0, y, x, y+20, FB_makecol(0,0,0,0));
	alarm(UPDATE_INTERVAL);
	start_time = time(NULL);
	cur_time = time(NULL);
	update = 1;
	while((cur_time - start_time) < MAX_SECS)
	{
		if(update)
		{
			update=0;
			cur_time = time(NULL);
			signal(SIGALRM, sig_alarm);
			alarm(UPDATE_INTERVAL);
			FB_rectfill(0, y, x, y+20, FB_makecol(0,0,0,0));
			FB_printf(xfont, yfont, FB_makecol(255,255,255,0), "This test takes 30 seconds, drawn %g pixels/s", (double)num_pixels/(cur_time - start_time));
		}
		col=FB_makecol(random()%255,random()%255,random()%255,random()%255);
		FB_putpixel(random()%x,random()%y,col);
		num_pixels++;
	}
	FB_clear_screen(FB_makecol(0,0,0,0));
	FB_exit();
	printf("Drawn %g pixels in %d seconds, it's %g pixels/s\n", (double)num_pixels, MAX_SECS, (double)num_pixels/MAX_SECS);
	return 0;
}
