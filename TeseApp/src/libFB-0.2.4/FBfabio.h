/*
 * FBfabio.h
 *
 *  Created on: 15 de Jun de 2013
 *      Author: root
 */

#ifndef FBFABIO_H_
#define FBFABIO_H_


#ifndef _WORD_
#define _WORD_
typedef unsigned short word;
#endif /* _WORD_ */

typedef __u32 FB_pixel;

typedef struct tagBITMAP              /* the structure for a bitmap. */
{
  int width;
  int height;
  FB_pixel * data;
} BITMAP;

/*! Load a BMP or JPEG file to a BITMAP struct.
 * \param image_name The complete name, including path, of the file */
BITMAP * FB_bitmapLoad(char * image_name);

/*! Destroy a BITMAP struct
 * \param bmp Pointer of struct to destroy */
void FB_bitmapDestroy(BITMAP* bmp);

/*! Draws a BITMAP struct with transparent color. Direct draw form file using FB_imageDraw_T.
 * \param bmp Pointer of the struct to draw
 * \param x X coordinate in pixel
 * \param y Y coordinate in pixel
 * \param transp_color Color of the pixel to be ignored */
void FB_bitmapDraw_T(BITMAP *bmp,int x,int y, FB_pixel transp_color);

/*! Draws a BITMAP struct. Direct draw form file using FB_imageDraw.
 * \param bmp Pointer of the struct to draw
 * \param x X coordinate in pixel
 * \param y Y coordinate in pixel */
void FB_bitmapDraw(BITMAP *bmp,int x,int y);

/*! Draws a BMP image with transprent color.
 * \param image_name The complete name, including path, of the file
 * \param x X coordinate in pixel
 * \param y Y coordinate in pixel
 * \param transp_color Color of the pixel to be ignored */
void FB_imageDraw_Transparent(char * image_name,int x,int y, FB_pixel transp_color);

/*! Draws a image, BMP or JPEG.
 * \param image_name The complete name, including path, of the file
 * \param x X coordinate in pixel
 * \param y Y coordinate in pixel */
void FB_imageDraw(char * image_name,int x,int y);

#endif /* FBFABIO_H_ */
