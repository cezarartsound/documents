/***************************************************************************
                      fbdb.c  -  libFB debuging program for libFB library
                             -------------------
    begin                : Thu Apr 18 2002
    copyright            : (C) 2002 by Nathan P. Cole
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
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
	float slope;
	FB_pixel col;
	u_char r, g, b;
	char command[60];
	int sx, sy, ex, ey;
	int red, green, blue;

	printf ("\nThis is the libFB debug program. It tests using a shell various\n");
	printf ("libFB capabilities. For information type ? <ENT>. This program is\n");
	printf ("free software; you can distribute it and/or modify it under the\n");
	printf ("the terms of the GNU General Public License as published by the\n");
	printf ("Free Software Foundation; version 2 of the License.\n\n");
	
	while (1) {
		printf ("[fbdb (q for quit, ? for help)] ");
		scanf ("%s", command);

		if (!strcmp (command, "?")) {
			printf ("Currently the only command is for lines.\n");
			printf ("To create a line type in the string line,\n");
			printf ("vline, or hline to enter a line a\n");
			printf ("verticle line or horizontal line respectfully.\n");
			continue;
		}

		if (!strcmp (command, "q")) {
			printf ("Good Bye!\n\n");
			break;
		}
		
		if (!strcmp (command, "hline")) {
			printf ("line color::\n");
			
			printf ("red (0-255): \n");
			scanf ("%i", &red);
			if (red < 0 || red > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			r = (u_char) red;

			printf ("green (0-255): \n");
			scanf ("%i", &green);
			if (green < 0 || green > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			g = (u_char) green;
		
			printf ("blue (0-255): \n");
			scanf ("%i", &blue);
			if (blue < 0 || blue > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			b = (u_char) blue;

			printf ("y: ");
			scanf ("%i", &sy);
			printf ("start x: ");
			scanf ("%i", &sx);
			printf ("end x:   ");
			scanf ("%i", &ex);
		
			col = FB_makecol (r, g, b, 0);

			FB_initlib ("/dev/fb0");
			FB_clear_screen (FB_makecol(0,0,0,0));
			FB_hline(sx, ex, sy, col);
			
		        getchar ();
			getchar ();
			FB_clear_screen (FB_makecol(0,0,0,0));
        		FB_exit ();			
		}

		if (!strcmp (command, "vline")) {
			printf ("line color::\n");
			
			printf ("red (0-255): \n");
			scanf ("%i", &red);
			if (red < 0 || red > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			r = (u_char) red;

			printf ("green (0-255): \n");
			scanf ("%i", &green);
			if (green < 0 || green > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			g = (u_char) green;
		
			printf ("blue (0-255): \n");
			scanf ("%i", &blue);
			if (blue < 0 || blue > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			b = (u_char) blue;

			printf ("x: ");
			scanf ("%i", &sx);
			printf ("start y: ");
			scanf ("%i", &sy);
			printf ("end y:   ");
			scanf ("%i", &ey);

			col = FB_makecol (r, g, b, 0);

			FB_initlib ("/dev/fb0");
			FB_clear_screen(FB_makecol(0,0,0,0));
			FB_vline(sx, sy, ey, col);
			
		        getchar ();
			getchar ();
			FB_clear_screen (FB_makecol(0,0,0,0));
        		FB_exit ();			
		}
			
		if (!strcmp (command, "line")) {
			printf ("line color::\n");

			printf ("red (0-255): \n");
			scanf ("%i", &red);
			if (red < 0 || red > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			r = (u_char) red;

			printf ("green (0-255): \n");
			scanf ("%i", &green);
			if (green < 0 || green > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			g = (u_char) green;
		
			printf ("blue (0-255): \n");
			scanf ("%i", &blue);
			if (blue < 0 || blue > 255) {
				printf ("Value must be between 0 and 255\n");
				continue;
			}
			b = (u_char) blue;

			FB_makecol (r, g, b, 0);

			printf ("Line position::\n");

			printf ("start x: ");
			scanf ("%i", &sx);

			printf ("start y: ");
			scanf ("%i", &sy);

			printf ("end x: ");
			scanf ("%i", &ex);

			printf ("end y: ");
			scanf ("%i", &ey);
				 
			col = FB_makecol (r, g, b, 0);

			FB_initlib("/dev/fb0");
			FB_clear_screen(FB_makecol(0,0,0,0));
			FB_line(sx, sy, ex, ey, col);
			
		        getchar ();
			getchar ();
			FB_clear_screen (FB_makecol(0,0,0,0));
        		FB_exit ();

			slope = (float) (sx - ex);

			if (slope == 0.0) {
				printf ("Slope of line is Infinite.\n");
				continue;
			}

			slope = (float) (sy - ey) / slope;
			printf ("Slope of line is %.3f.\n", slope);
		}
	}
        return 0;
}
