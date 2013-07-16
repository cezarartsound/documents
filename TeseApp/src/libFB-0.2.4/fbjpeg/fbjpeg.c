/***************************************************************************
                          fbjpeg.c  -  test program for libFB library
                             -------------------
    begin                : Mon Nov 20 2000
    copyright            : (C) 2000 by Daniele Venzano
    email                : webvenza@libero.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/
#include "../FBlib.h"
#include <stdio.h>
#include <jpeglib.h>

int main(int argc, char *argv[])
{
	FB_pixel col;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *infile;
	JSAMPARRAY buffer;
	int i,j,x=0,y=0;
	
	if(argc < 2)
	{
		fprintf(stderr,"usage: test image.jpg\n");
		exit(1);
	}
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	if((infile = fopen(argv[1],"rb")) == NULL)
	{
		fprintf(stderr,"can't open %s\n",argv[1]);
		exit(1);
	}
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	buffer=(*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width*cinfo.output_components, 1);
	FB_initlib("/dev/fb0");
	i=cinfo.output_width*cinfo.output_components*sizeof(char);
	FB_clear_screen(FB_makecol(0,0,0,0));
	while(cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);
		j=0;
		while(j<=i)
		{
			col = FB_makecol(buffer[0][j],buffer[0][j+1],buffer[0][j+2],0);
			FB_putpixel(x,y,col);
			j+=3;
			x++;
		}
		y++;
		x=0;
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	getc(stdin);
        FB_exit();
        return 0;
}

