#include "FBlib.h"

FB_pixel iterate (double x, double y)
{
	int i;
	float val;
	float xn, yn;
	float xn1;

	xn = 0;
	yn = 0;

	for (i = 0; i < 256; i++) {
		xn1 = xn * xn - yn * yn + x;
		yn = 2.0 * xn * yn + y;
		xn = xn1;
		val = xn * xn + yn * yn;
		if (val > 9.0) break;
	}
	if (i > 250) return FB_makecol (0,0,0,0);
	return FB_makecol (0, i, (2 * i * (i/2) + i/20) % 256,0);	
}

int main ()
{
	int i, j;
	float dx, dy;
	float x = -2.3;
	float y = -1.5;
	int max_x, max_y;
	dx = 0.004;
	dy = 0.004;

	FB_initlib ("/dev/fb0");
	FB_clear_screen (FB_makecol (0,0,255,0));
	FB_getres(&max_x, &max_y);

	for (j = 10; j < max_y - 10; j++) {
		for (i = 10; i < max_x - 10; i++) {
			FB_putpixel(i, j, iterate (x, y));
			x += dx;
		}
		x = -2.3;
		y += dy;
	}

	getchar ();
	FB_clear_screen (FB_makecol(0,0,0,0));
	FB_exit ();

	return 0;
}	
