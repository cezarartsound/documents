/*! \file FBinit.c
 * \brief Initialization and clean up functions
 *
 * begin                : Fri Nov 24 2000\n
 * copyright            : (C) 2000 by Daniele Venzano\n
 * email                : venza@users.sf.net\n */

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License. */

#include "FBlib.h"
#include "FBpriv.h"

int FB_visible = 1;

struct fb_var_screeninfo work_var_info;
struct fb_var_screeninfo backup_var_info;
struct fb_fix_screeninfo fb_fix_info;

int FB_initlib(char *dev)
{
	int ioctl_err;
	char *errstr;
	struct vt_mode vtmode;
	
	if(!strcmp(dev, ""))
		dev = "/dev/fb0";
	fbufd = open(dev, O_RDWR); // Open framebuffer device
	if(fbufd < 0)
	{
		errstr = strerror(errno);
		fprintf(stderr, "FBlib:open %s failed: %s \n", dev, errstr);
		return -1;
	}
	ioctl_err = ioctl(fbufd,FBIOGET_FSCREENINFO,&fb_fix_info); // Get fixed informations 
	if(ioctl_err < 0)
	{
		errstr = strerror(errno);
		fprintf(stderr,"FBlib:ioctl FBIOGET_FSCREENINFO failed: %s \n",errstr);
		return IOCTL_ERR;
	}
	// Get backup variable infos 
	ioctl_err = ioctl(fbufd,FBIOGET_VSCREENINFO,&backup_var_info);
	// Get changeable variable infos 
	ioctl_err = ioctl(fbufd,FBIOGET_VSCREENINFO,&work_var_info);
	if(ioctl_err < 0)
	{
		errstr=strerror(errno);
		fprintf(stderr,"FBlib:ioctl FBIOGET_VSCREENINFO failed: %s \n",errstr);
		return IOCTL_ERR;
	}
	
	work_var_info.xoffset = 0;
	work_var_info.yoffset = 0;
	
	ioctl_err = ioctl(fbufd,FBIOPUT_VSCREENINFO,&work_var_info);
	if(ioctl_err < 0)
	{
		errstr = strerror(errno);
		fprintf(stderr,"FBlib:ioctl FBIOPUT_VSCREENINFO failed: %s \n",errstr);
		return IOCTL_ERR;
	}

	screensize = fb_fix_info.smem_len; // size in bytes of the virtual screen 
	visiblesize = work_var_info.xres * work_var_info.yres * (work_var_info.bits_per_pixel/8);
	
	ioctl_err = ioctl(TTY, VT_GETMODE, &vtmode);
	if(ioctl_err < 0)
	{
		errstr = strerror(errno);
		fprintf(stderr,"FBlib:ioctl VT_GETMODE failed: %s \n",errstr);
		return IOCTL_ERR;
	}
	vtmode.mode = VT_PROCESS;
	vtmode.relsig = SIGUSR2;
	vtmode.acqsig = SIGUSR2;
	
	signal(SIGUSR2, fb_switch);
	
	ioctl_err = ioctl(TTY, VT_SETMODE, &vtmode);
	if(ioctl_err < 0)
	{
		errstr = strerror(errno);
		fprintf(stderr,"FBlib:ioctl VT_SETMODE failed: %s \n",errstr);
		return IOCTL_ERR;
	}
	
	ioctl_err = fb_map();
	
	if(ioctl_err < 0)
		return ioctl_err;
	
	FB_change_font(DEFAULT_FONT);

	inc_x = work_var_info.bits_per_pixel/8;
	inc_y = fb_fix_info.line_length;

	return OK;
}

int FB_exit()
{
	int ioctl_err;
	char *errstr;
	
	if(!FB_visible && (b_store != NULL))
	{
		free(b_store);
		b_store = NULL;
	}
	
	ioctl_err = ioctl(fbufd,FBIOPUT_VSCREENINFO,&backup_var_info);
	if(ioctl_err < 0)
	{
		errstr = strerror(errno);
		fprintf(stderr,"FBlib:ioctl FBIOPUT_VSCREENINFO failed: %s \n",errstr);
		return IOCTL_ERR;
	}
	fb_unmap();
	close(fbufd);
	if(uses_keyboard)
		FB_kb_end();
	printf("\n"); /* Cause a redraw to align the bottom of the screen */
	return OK;
}

 /* Maps the framebuffer device */
int fb_map()
{
	fbp=(char*)mmap(0, screensize, PROT_WRITE | PROT_READ, MAP_SHARED, fbufd, 0);
	if(fbp<0)
	{
		fprintf(stderr,"FBlib:mmap failed: %s \n",strerror(errno));
		return MMAP_ERR;
	}
	fb_visp = fbp;
	return 0;
}

/* Unmaps the framebuffer device */
int fb_unmap()
{
	munmap(fbp, screensize);
	return 0;
}

void fb_switch(int n_sig)
{
	static int switching=0;
	
	signal(SIGUSR2, fb_switch); /* reset signal handler */
	if(switching && (n_sig == SIGUSR2))
		return;
	if(switching == 0)
		switching = 1;
	if(FB_visible)
	{
		b_store = (char*)malloc(visiblesize);
		if(b_store == NULL)
			fprintf(stderr,"FBlib:cannot use backing store for VT switch");
		else
			memcpy(b_store, fbp, visiblesize);
		fb_unmap();
		fbp = b_store;
		FB_visible = 0;
		if(ioctl(TTY,VT_RELDISP,1) < 0)
		{
			fprintf(stderr,"FBlib:ioctl VT_RELDISP failed: %s \n",strerror(errno));
			fb_map();
			free(b_store);
			FB_visible=1;
		}
	}
	else
	{
		if(ioctl(TTY,VT_RELDISP,VT_ACKACQ) < 0)
		{
			fprintf(stderr,"FBlib:ioctl VT_RELDISP failed: %s (FATAL error)\n",strerror(errno));
			FB_exit();
			exit(1);
		}
		fb_map();
		memcpy(fbp, b_store, visiblesize);
		free(b_store);
		b_store=NULL;
		FB_visible=1;
	}
	switching=0;
	return;
}
