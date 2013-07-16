/***************************************************************************
                       test.c  -  tests if rectfill works
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

int update;

void sig_alarm(int sig)
{
	update=1;
	
}

int main()
{
	FB_initlib("/dev/fb0");
	FB_clear_screen(FB_makecol(0,0,0,0));
	FB_rectfill(10, 12, 100, 120, FB_makecol(255,0,0,0));
	FB_rectfill(90, 12, 300, 120, FB_makecol(0,255,0,0));
	FB_rectfill(20, 100, 120, 400, FB_makecol(0,0,255,0));
	FB_rectfill(90, 100, 300, 400, FB_makecol(213,31,125,0));
	getchar();
	FB_clear_screen(FB_makecol(0,0,0,0));
	FB_exit();
	return 0;
}
