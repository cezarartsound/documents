/***************************************************************************
                    test3.c  -  test clear_screen speed
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
#include <time.h>

#define MAX_SECS 30
#define UPDATE_INTERVAL 5

int main()
{
	int i;
	long elapsed_time;
	
	FB_initlib("/dev/fb0");
	elapsed_time = time(NULL);
	for(i=255; i>=0; i--)
		FB_clear_screen(FB_makecol(i,0,0,0));
	FB_rectfill(10,10,20,20,0);
	elapsed_time = time(NULL) - elapsed_time;
	FB_exit();
	printf("The screen has been filled 256 times in %d seconds, it's %g clears/s\n", (int)elapsed_time, (double)256/elapsed_time);
	return 0;
}
