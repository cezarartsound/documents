/*! \file FBlib.h 
 *  \brief Definitions common to all FBlib programs
 *
 * This file must be included in every program that will use libFB\n
 * begin                : Mon Nov 20 2000\n
 * copyright            : (C) 2000 by Daniele Venzano\n
 * email                : venza@users.sf.net\n */
 
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License. */

#ifndef FBLIB_H
#define FBLIB_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/kd.h>
#include <signal.h>
#include <linux/vt.h>


/*! Keys definition not in ncurses */
#include "keys.h"

/*! Default font used by font engine */
#ifndef DEFAULT_FONT
	#define DEFAULT_FONT "/usr/share/consolefonts/iso01.f14.psf.gz"
#endif

/* internal */
#define DEBUG
#define u_char unsigned char

/* Error handling */
#ifndef OK
#define OK         0
#endif
#define PARAM_ERR -1
#define IOCTL_ERR -2
#define MMAP_ERR  -3

/* Visuals */
#define FB_VISUAL_MONO01             0
#define FB_VISUAL_MONO10             1
#define FB_VISUAL_TRUECOLOR          2
#define FB_VISUAL_PSEUDOCOLOR        3
#define FB_VISUAL_DIRECTCOLOR        4
#define FB_VISUAL_STATIC_PSEUDOCOLOR 5

typedef __u32 FB_pixel;

/*! If TRUE user is on the same VT as FBlib program */
extern int FB_visible;

/*! This function must be called before any other call to other parts of FBlib,
 * it makes some initialization and sets up the handler for a nice VT switch.
 * \param dev The framebuffer device name (eg. "/dev/fb0") */
int FB_initlib(char *dev);  

/*! You must call this at the end of your program to free up memory and
 * reset the framebuffer device.
 * \retval OK No error
 * \retval IOCTL_ERR The framebuffer settings could't be restored to their
 * initial value */
int FB_exit();   

/*! Bit magic to create a framebuffer dependent representation of
 * a 16/32 bit color. The bpp used is the one the fb is currently set at.
 * \param r red component 0 - 255
 * \param g green component 0 - 255
 * \param b blue comoponent 0 - 255
 * \param t transparency 0 - 255 (not supported on some kernel drivers) */
inline FB_pixel FB_makecol(u_char r,u_char g, u_char b, u_char t);

/*! Fills all the screen with the same color. Currently VERY slow.
 * \param color Color used to redraw the screen */
void FB_clear_screen(FB_pixel color);

/*! Draws a pixel.
 * \param x X coordinate
 * \param y Y coordinate
 * \param color Color of the pixel being drawn */
inline void FB_putpixel(int x, int y, FB_pixel color);

/*! Reads the color of a pixel.
 * \param x X coordinate
 * \param y Y coordinate */
FB_pixel FB_getpixel(int x, int y);

/*! Draws a line.
 * \param sx Start x coordinate
 * \param sy Start y coordinate
 * \param ex End x coordinate
 * \param ey End y coordinate 
 * \param color Color of the line */
void FB_line(int sx, int sy, int ex, int ey, FB_pixel color);

/*! Optimized version of line() to draw a vertical line */
void FB_vline(int x, int sy, int ey, FB_pixel color);

/*! Optimized version of line() to draw an horizontal line */
void FB_hline(int sx, int ex, int y, FB_pixel color);

/*! Draws a rectangle (only borders).
 * \param sx,sy Upper left corner
 * \param ex,ey Lower right corner 
 */
void FB_rect(int sx, int sy, int ex, int ey, FB_pixel color);

/*! Same as FB_rect, but the rectangle is filled */
void FB_rectfill(int sx, int sy, int ex, int ey, FB_pixel color);

/*! Draws a triangle
 * \param x1,y1 First vertex
 * \param x2,y2 Second vertex
 * \param x3,y3 Third vertex
 * \param col Color 
 */
void FB_triangle(int x1,int y1,int x2,int y2,int x3,int y3,FB_pixel col);

/*! Draws a circle
 * \param cx,cy Position of the center
 * \param radius Radius of the circle
 * \param color Color
 */
void FB_circle(int cx, int cy, int radius, FB_pixel color);

/*! Returns one of the following values, they are all defined in
 * linux/fb.h
 * \retval FB_VISUAL_MONO01                0       Monochr. 1=Black 0=White
 * \retval FB_VISUAL_MONO10                1       Monochr. 1=White 0=Black
 * \retval FB_VISUAL_TRUECOLOR             2       True color
 * \retval FB_VISUAL_PSEUDOCOLOR           3       Pseudo color (like atari)
 * \retval FB_VISUAL_DIRECTCOLOR           4       Direct color
 * \retval FB_VISUAL_STATIC_PSEUDOCOLOR    5       Pseudo color readonly 
 */
int FB_getVisual();

/*! Sets x and y respectively to the width and height of the visible screen */
void FB_getres(int *x, int *y);

/*! Returns virtual screen width */
int FB_getXVres();

/*! Returns virtual screen height */
int FB_getYVres();

/*! Returns color depth the framebuffer is set to. It can be 16 or 32 */
int FB_getbpp();

/*! Change the color depth of the framebuffer, this function can fail,
 * so check always the return value:
 * \retval OK No error
 * \retval PARAM_ERR Color depth not supported
 * \retval IOCTL_ERR Probably the color depth is not supported by the graphic device */
int FB_setbpp(int bpp);

/* FBkeyb.c */
/*! Initialize keyboard handling, don't call if you don't need it */
int FB_kb_init();

/*! Look at input buffer if there is some event not handled.
 * \retval NO_KEY There are no keypresses unhandled
 * \retval -anything else- Keypress (look at ncurses.h and keys.h */
int FB_get_key();

/*! Ends keyboard usage, is called, if necessary by FB_end() */
void FB_kb_end();

/* FBfont.c */
/*! Change the font set used to draw caracters. Can also use psf files
 * compressed with gzip.
 * \param psf_file Path to the .psf or .psf.gz file that conteins the new font set
 * \retval OK No error
 * \retval PARAM_ERR The file was not found or it wasn't in psf format */
int FB_change_font(char *psf_file);

/*! Same as putc function from your favourite libc.
 * \param c ASCII code of the caracter to display
 * \param x,y position of the upper left corner of the caracter
 * \param col color of the caracter
 * \retval OK No error
 * \retval PARAM_ERR You requested a glyph with a code > 255 using a font set
 * with only 255 gliphs */
int FB_putc(int c, int x, int y, FB_pixel col);

/*! Same as usula printf, extend to draw everywhere in the screen
 * \param x,y Position of the upper left corner of the first caracter
 * \param col Color of the whole string
 * \param format Format string of the output, same format as printf, but only \r,
 * \\n \%d \%x \%c \%g %% are supported
 * \param ... format variables
 * \retval 0 Should return something usefull, but will return always 0 */
int FB_printf(int x, int y, FB_pixel col, char *format, ...);


#include "FBfabio.h"

#endif /* FBLIB_H */

