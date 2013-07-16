/*! \file FBpriv.h
 * \brief Private definitions, don't include
 *
 * This file contains only private global definitions used by FBlib, you 
 * shouldn't use any of these in your programs because they can change
 * a lot between different versions of FBlib. \n
 * begin                : Mon Nov 20 2000\n
 * copyright            : (C) 2000 by Daniele Venzano\n
 * email                : venza@users.sf.net
 */

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 */


#ifndef FBPRIV_H
#define FBPRIV_H

#define TTY 0

#define GETPOS(a,b) ((a * inc_x) + (b * inc_y))

int fbufd,oldxres,oldyres;
struct fb_fix_screeninfo fb_fix_info;
struct fb_var_screeninfo backup_var_info;
struct fb_var_screeninfo work_var_info;

char *fbp, *b_store, *fb_visp;
long int screensize;
long int visiblesize;
int uses_keyboard;
int inc_x, inc_y;

inline void setpixel(char *pos, FB_pixel col);
long int get_screensize();
int fb_map();
int fb_unmap();
void fb_switch(int n_sig);


#endif
