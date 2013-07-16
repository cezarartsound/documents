/* To create executable use: $(CC) cb.c -o cb -lFB */
#include "FBlib.h"
#include "FBpriv.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define CLAMP(a,b,c) MIN(MAX((a),(b)),(c))

int size = 2;
int mouse_fd;
int data_size;
int mouse_left;
int mouse_right;
int mouse_x = 50;
int mouse_y = 50;
int mouse_pix_x = 10;
int mouse_pix_y = 10;
static signed char data[4];
FB_pixel mouse_area[11][11];
 
void update_mouse(void)
{
        static int count = 0;
        int i, changed = 0;

        while (1) {
        	i = read(mouse_fd, data + count, data_size - count);
                if (i <= 0) {
                        break;
                }
                count += i;
                if (count == data_size) {
                        count = 0;
                        changed = 1;
			mouse_left  = data[0] & 1;
			mouse_right = (data[0] & 2) >> 1;
			mouse_x += (int) data[1];
			mouse_y -= (int) data[2];
                }
        }
        if (changed) {
		mouse_x = CLAMP (mouse_x, mouse_pix_x + 2 * size, work_var_info.xres - mouse_pix_x - 1);
		mouse_y = CLAMP (mouse_y, mouse_pix_y + 2 * size, work_var_info.yres - mouse_pix_y - 1);
        }
}

void under_mouse ()
{
	int i, j;
	
	for (i = 0; i < 11; i++) {
		for (j = 0; j < 11; j++) {
			mouse_area[i][j] = FB_getpixel (i + mouse_x, j + mouse_y);
		}
	}
}

void over_mouse ()
{
	int i, j;
	
	for (i = 0; i < 11; i++) {
		for (j = 0; j < 11; j++) {
			FB_putpixel (i + mouse_x, j + mouse_y, mouse_area[i][j]);
		}
	}
}

int main (int argc, char ** argv)
{
	struct pollfd p_mouse;
	FB_pixel bg, red, green, wierd;
	
	if (argc != 3) {
		printf ("Usage: %s -(type) size\n", argv[0]);
		printf ("Where (type) can so far be one of the following:\n");
		printf ("imps2 - for imps/2 mouse.\n");
		printf ("ps2 - for ps/2 mouse.\n");
		exit (0);
	}

	/*  This is so it will work for either IMPS/2 or PS/2. The only difference is in
         *  the size of the data returned by the mouse. Use -i for imps/2, -p for ps/2.
	 *  Hopefully in the future I will be able to make it work for more kind of mice.
         */

	if (!strcmp ("-imps2", argv[1])) data_size = 4;
	else if (!strcmp ("-ps2", argv[1])) data_size = 3;
	else {
		printf ("Unknown mouse type %s.\n", argv[1]);
		exit (0);
	};

	size = atoi (argv[2]) + 1;
	
	printf ("Move the mouse around and press left click to draw.\n");
	printf ("To exit press right click. Have fun!\n");
	getchar ();

        printf ("Initializing Mouse... ");
	mouse_fd = open ("/dev/mouse", O_RDONLY | O_NONBLOCK);
	if (mouse_fd < 0)
	{
		perror ("/dev/mouse");
	}
	p_mouse.fd = mouse_fd;
	p_mouse.events = POLLIN;
	printf ("[done]\n");
		
	printf ("initializing libFB... ");
	FB_initlib("/dev/fb0");
	bg = FB_makecol (0,0,0,0);
	red = FB_makecol (255,0,0,10);
	green = FB_makecol (0,255,0,0);
	wierd = FB_makecol (245, 0, 128, 0);

	FB_clear_screen (bg);
	under_mouse ();

        while (1)
        {
		poll (&p_mouse, 1, 0);
		if (p_mouse.revents & POLLIN)
		{
			over_mouse ();
			update_mouse ();
			if (mouse_left) 
			{
				FB_hline (mouse_x - 2 * size, mouse_x - size, mouse_y, red);
				FB_vline (mouse_x - 2 * size, mouse_y - 2 * size, mouse_y, wierd);
				FB_hline (mouse_x - 2 * size, mouse_x - size, mouse_y, red);
				FB_vline (mouse_x - size, mouse_y - 2 * size, mouse_y, wierd);
			}
			under_mouse ();
		 	FB_vline (mouse_x + 5, mouse_y, mouse_y + 10, green);
			FB_hline (mouse_x, mouse_x + 10, mouse_y + 5, green);
			if (mouse_right) {
				FB_clear_screen (bg);
				goto exit;
			}
		}
        }
exit:
	close (mouse_fd);
        FB_exit();
	printf ("Exit.\n");
        return 0;
}
