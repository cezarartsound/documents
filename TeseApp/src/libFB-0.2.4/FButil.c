/*! \file FButil.c 
 * \brief Helper functions
 *
 * Implementation of functions to get/set some framebuffer properties, and other ones 
 * that I had no other place for. \n
 * begin                : Mon Nov 20 2000\n
 * copyright            : (C) 2000 by Daniele Venzano\n
 * email                : venza@users.sf.net */

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License. */
 
#include "FBlib.h"
#include "FBpriv.h"

FB_pixel FB_makecol(u_char r, u_char g, u_char b, u_char t)
{
	FB_pixel color = 0;
	
	color |= ( (r >> (8 - work_var_info.red.length)) << (work_var_info.red.offset) );
	color |= ( (g >> (8 - work_var_info.green.length)) << (work_var_info.green.offset) );
	color |= ( (b >> (8 - work_var_info.blue.length)) << (work_var_info.blue.offset) );
	if(work_var_info.bits_per_pixel == 32)
		color |= ( (t >> (8 - work_var_info.transp.length)) << (work_var_info.transp.offset) );
	return color;
}

long int get_screensize()
{
	return fb_fix_info.smem_len;
}

int FB_getVisual()
{
	return fb_fix_info.visual;
}

void FB_getres(int *x, int *y)
{
	if (x != NULL) *x = work_var_info.xres;
	if (y != NULL) *y = work_var_info.yres;
	return;
}

int FB_getXVres()
{
	return work_var_info.xres_virtual;
}

int FB_getYVres()
{
	return work_var_info.yres_virtual;
}

int FB_getbpp()
{
	return work_var_info.bits_per_pixel;
}

int FB_setbpp(int bpp)
{
	int ioctl_err, oldbpp;	

	/* Check if depth is supported */
	if(bpp != 16 || bpp != 32)
		return -1;
	oldbpp = work_var_info.bits_per_pixel;
	work_var_info.bits_per_pixel = bpp;
	/* Try to set bpp */
	ioctl_err=ioctl(fbufd,FBIOPUT_VSCREENINFO,&work_var_info);
	if(ioctl_err < 0)
	{
		fprintf(stderr,"FBlib:unable to change bpp to %d, %s \n",bpp,strerror(errno));
		work_var_info.bits_per_pixel = oldbpp;
		return IOCTL_ERR;
	}
	/* If everything went well change inc_x */
	inc_x = work_var_info.bits_per_pixel / 8;

	return 0;
}
