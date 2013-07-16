/***************************************************************************
                       fbstat.c  -  status of various settings
                             -------------------
    begin                : Thu Apr 18 2002
    copyright            : (C) 2002 Nathan P. Cole
    email                : qmagick@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/
#include "FBlib.h"
#include "FBpriv.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	FB_initlib("/dev/fb0");
	
	printf ("\nsize of red, green and blue: %i %i %i\n",
		work_var_info.red.length,
		work_var_info.green.length,
		work_var_info.blue.length
	);

	printf ("offsets of red, green and blue: %i %i %i\n",
		work_var_info.red.offset,
		work_var_info.green.offset,
		work_var_info.blue.offset
	);
	
	printf ("bytes per pixel: %i\n", work_var_info.bits_per_pixel/8);
	printf ("bytes per line:  %i\n", fb_fix_info.line_length);
	printf ("total number of bytes, screen: %ld %ld\n", visiblesize, screensize);
	printf ("max screen width and height: %i %i\n", work_var_info.xres, work_var_info.yres);

	FB_exit();
	return 0;
}
