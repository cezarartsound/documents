/***************************************************************************
                       test6.c  -  tests if FB_circle works
                             -------------------
    begin                : sat may 18 2002
    copyright            : (C) 2002 by Daniele Venzano
    email                : venza@users.sourceforge.net
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
	long num_lines = 0;
	int dir = 0, cx, cy, radius = 1;
	
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
	cx = x/2;
	cy = y/2;
	while((cur_time - start_time) < MAX_SECS)
	{
		if(update)
		{
			update=0;
			cur_time = time(NULL);
			signal(SIGALRM, sig_alarm);
			alarm(UPDATE_INTERVAL);
			FB_rectfill(0, y, x, y+20, FB_makecol(0,0,0,0));
			FB_printf(xfont, yfont, FB_makecol(255,255,255,0), "This test takes 30 seconds, drawn %g lines/s", (double)num_lines/(cur_time - start_time));
		}
		if(dir == 0)
		{
			col=FB_makecol(random()%255,random()%255,random()%255,random()%255);
			FB_circle(cx, cy, radius, col);
			radius++;
			if(radius > x/2)
				dir=1;
		}
		else
		{
			col=FB_makecol(random()%255,random()%255,random()%255,random()%255);
			FB_circle(cx, cy, radius, col);
			radius--;
			if(radius < 1)
				dir=0;
		}
		num_lines++;
	}
	FB_clear_screen(FB_makecol(0,0,0,0));
	FB_exit();
	printf("Drawn %g circles in %d seconds, it's %g circles/s\n", (double)num_lines, MAX_SECS, (double)num_lines/MAX_SECS);
	return 0;
}

