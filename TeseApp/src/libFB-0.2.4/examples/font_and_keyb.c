/***************************************************************************
            font_and_keyb.c  -  Example for keyboard and font usage
                             -------------------
    begin                : Fri Feb 8 2002
    copyright            : (C) 2001 by Daniele Venzano
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

int main()
{
	int key;

	FB_initlib("/dev/fb0");
	FB_kb_init();
	FB_clear_screen(FB_makecol(0,0,0,0));
	while(1)
	{
		FB_printf(30,30, FB_makecol(255,255,255,0), "Ciao!");
		if((key = FB_get_key()) != NO_KEY)
			if(key == 'q')
				break;
	}		
	FB_exit();
	return 0;
}
