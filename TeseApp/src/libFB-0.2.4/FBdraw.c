/*! \file FBdraw.c
 * \brief Drawing primitives
 *
 * begin                : Mon Nov 20 2000\n
 * copyright            : (C) 2000 by Daniele Venzano\n
 * email                : venza@users.sf.net\n
 * \n
 * Nathan P. Cole: added FB_triangle function */

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License. */
 
#include <stdint.h> // intptr_t
#include <math.h>
#include "FBlib.h"
#include "FBpriv.h"

inline void setpixel(char *pos, FB_pixel col)
{
	char *abs_pos;
	
	abs_pos = fb_visp + (long int)pos;
	if(work_var_info.bits_per_pixel == 16)
	{
		*abs_pos = (u_char)col;
		*(abs_pos + 1) = (u_char)(col>>8);
	}
	else if (work_var_info.bits_per_pixel == 32)
	{
		*abs_pos = (u_char)col;
		*(abs_pos + 1) = (col>>8);
		*(abs_pos + 2) = (u_char)(col>>16);
		*(abs_pos + 3) = (u_char)(col>>24);
	}

	return;
}

inline void FB_putpixel(int x, int y, FB_pixel color)
{
	char *pos;
	
	if(x < 0 || x > work_var_info.xres)
		return;
	if(y < 0 || y > work_var_info.yres)
		return;

	pos = (char*)(intptr_t)(GETPOS(x,y));
	setpixel(pos, color);

	return;
}

FB_pixel FB_getpixel(int x, int y)
{
	long int location;
	FB_pixel tmp;
	
	location=x * (work_var_info.bits_per_pixel/8) + y * (fb_fix_info.line_length);
	if(location > visiblesize)
		return 0; /* return black if out of bounds (weird) */
	tmp = *(fb_visp + location) | (*(fb_visp + location + 1)<<8);
	if(work_var_info.bits_per_pixel == 32)
		tmp |= (*(fb_visp + location + 2)<<16) + (*(fb_visp + location + 3)<<24);
	return tmp;
}

void FB_vline(int x, int sy, int ey, FB_pixel color)
{
	int i;
	char *pos;

	/* Parameter sanity checks */
	if(x < 0 || x > work_var_info.xres)
		return;
	
	if(sy > ey)
	{
		i=sy;
		sy=ey;
		ey=i;
	}

	if(sy < 0)
		sy = 0;

	if(ey > work_var_info.yres)
		ey = work_var_info.yres;
	/* end of checks */

	pos = (char*)(intptr_t)(GETPOS(x,sy));
	for(i = sy; i <= ey; i++)
	{
		setpixel(pos, color);
		pos+=inc_y;
	}
	return;
}

void FB_hline(int sx, int ex, int y, FB_pixel color)
{
	int i;
	char *pos;
	
	if(y < 0 || y > work_var_info.yres)
		return;
	
	if(sx > ex)
	{
		i=sx;
		sx=ex;
		ex=i;
	}

	if(sx < 0)
		sx = 0;
	if(ex > work_var_info.xres)
		ex = work_var_info.xres;
	
	pos = (char*)(intptr_t)(GETPOS(sx,y));
	for(i = sx; i <= ex; i++)
	{
		setpixel(pos, color);
		pos+=inc_x;
	}
	return;
}

/* This is Bresenham's line algorithm */
void FB_line(int sx, int sy, int ex, int ey, FB_pixel color)
{
	int dx,dy;
	int x,y,incx,incy;
	int balance;
	
	if(sx == ex)
		FB_vline(sx,sy,ey,color);
	if(sy == ey)
		FB_hline(sx,ex,sy,color);
	
	if (ex >= sx)
	{
		dx = ex - sx;
		incx = 1;
	}
	else
	{
		dx = sx - ex;
		incx = -1;
	}

	if (ey >= sy)
	{
		dy = ey - sy;
		incy = 1;
	}
	else
	{
		dy = sy - ey;
		incy = -1;
	}

	x = sx;
	y = sy;

	if (dx >= dy)
	{
		dy <<= 1;
		balance = dy - dx;
		dx <<= 1;

		while (x != ex)
		{
			FB_putpixel(x, y, color);
			if (balance >= 0)
			{
				y += incy;
				balance -= dx;
			}
			balance += dy;
			x += incx;
		} FB_putpixel(x, y, color);
	}
	else
	{
		dx <<= 1;
		balance = dx - dy;
		dy <<= 1;

		while (y != ey)
		{
			FB_putpixel(x, y, color);
			if (balance >= 0)
			{
				x += incx;
				balance -= dy;
			}
			balance += dx;
			y += incy;
		} FB_putpixel(x, y, color);
	}

	return;
}

void FB_rect(int sx, int sy, int ex, int ey, FB_pixel color)
{
	FB_hline(sx,ex,sy,color);
	FB_hline(sx,ex,ey,color);
	FB_vline(sx,sy,ey,color);
	FB_vline(ex,sy,ey,color);
	return;
}

void FB_rectfill(int sx, int sy, int ex, int ey, FB_pixel color)
{
	int i,j;
	char *pos;
	
	if(sx > ex)
	{
		i=sx;
		sx=ex;
		ex=i;
	}
	if(sy > ey)
	{
		i=sy;
		sy=ey;
		ey=i;
	}
	if(sx < 0)
		sx = 0;
	if(sy < 0)
		sy = 0;
	if(ex > work_var_info.xres)
		ex = work_var_info.xres;
	if(ey > work_var_info.yres)
		ey = work_var_info.yres;
	
	pos = (char*)(intptr_t)(GETPOS(sx,sy));
	for(i = sy; i <= ey; i++)
	{
		for(j = sx; j <= ex; j++ )
		{
			setpixel(pos,color);
			pos+=inc_x;
		}
		pos = (char*)(intptr_t)(GETPOS(sx,i));
	}
	return;
}

void FB_triangle(int x1,int y1,int x2,int y2,int x3,int y3,FB_pixel col)
{
	FB_line(x1, y1, x2, y2, col);
	FB_line(x3, y3, x2, y2, col);
	FB_line(x1, y1, x3, y3, col);
	
	return;
}

void FB_circle(int cx, int cy, int radius, FB_pixel color)
{
	double cosval;
	double sinval;
	double inc = (M_PI)/(radius*3.6); // Is this ok for all circles ?
	double angle=0;
	int x,y;

	while(angle <= M_PI)
	{
		cosval = radius*cos(angle);
		sinval = radius*sin(angle);
		x = (int)(cosval + cx);
		y = (int)(sinval + cy);
		FB_putpixel(x, y, color);
		x = (int)(-cosval + cx);
		y = (int)(-sinval + cy);
		FB_putpixel(x, y, color);
		angle+=inc;
	}
	return;
}

void FB_clear_screen(FB_pixel color)
{
	char *pos=0;
	int i,j;
	
	for(i = 0; i < work_var_info.yres; i++)
	{
		for(j = 0; j < work_var_info.xres; j++ )
		{
			setpixel(pos,color);
			pos+=inc_x;
		}
		pos = (char*)(intptr_t)(GETPOS(0,i));
	}

	return;

}
