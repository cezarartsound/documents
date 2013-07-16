/*
 * FBfabio.c
 *
 *  Created on: 15 de Jun de 2013
 *      Author: root
 */

#include <stdio.h>
#include <malloc.h>
#include <jpeglib.h>
#include <png.h>
#include "FBlib.h"

int i_;
#define fskip(FD,N) for(i_=0;i_<N;i_++)fgetc(FD)


BITMAP * BMPbitmapLoad(char * image_name){
	FILE *fp;
	long index;
	word num_colors, nr_bits;
	int x;

	BITMAP * bmp = (BITMAP*)malloc(sizeof(BITMAP));
	
	// open the file
	if ((fp = fopen(image_name,"rb" )) == NULL)
	{
		printf("Error opening file %s.\n",image_name);
		free(bmp);
		return 0;
	}

	char buff[2];
	fread(buff,1,2,fp);

	if (buff[0]!='B' || buff[1]!='M')
	{
		fclose(fp);
		printf("%s is not a bitmap file.\n",image_name);
		free(bmp);
		return 0;
	}
	// read in the width and height of the image, and the number of colors used; ignore the rest
	fskip(fp,8);
	word offset; fread(&offset, sizeof(word), 1, fp);
	fskip(fp,6);
	fread(&bmp->width, sizeof(word), 1, fp);
	fskip(fp,2);
	fread(&bmp->height,sizeof(word), 1, fp);
	fskip(fp,4);
	fread(&nr_bits,sizeof(word), 1, fp);
	fskip(fp,16);
	fread(&num_colors,sizeof(word), 1, fp);

	if (num_colors==0) num_colors=256;
	fskip(fp,offset-48);//fskip(fp,6+num_colors*4);	// Ignore the rest of header and palette information for now.


	// try to allocate memory
	if ((bmp->data = (FB_pixel *) malloc(sizeof(FB_pixel)*(bmp->width*bmp->height))) == NULL)
	{
		fclose(fp);
		printf("Error allocating memory for file %s.\n",image_name);
		free(bmp);
		return 0;
	}

	int r,g,b;
	// read the bitmap
	if(nr_bits==8){
	/*	for(index=(bmp->height-1)*bmp->width;index>=0;index-=bmp->width)
			for(x=0;x<bmp->width;x++)
				bmp->data[(word)index+x]=VGA_8bpp_to_16bpp(fgetc(fp));*/ // TODO
	}else if(nr_bits==24){
		int padd = 0;
		while ((bmp->width * 24 + padd*8)%32 != 0 ) padd++;
		for(index=(bmp->height-1)*bmp->width;index>=0;index-=bmp->width){
			for(x=0;x<bmp->width;x++){
				b = fgetc(fp);
				g = fgetc(fp);
				r = fgetc(fp);
				bmp->data[index+x]=FB_makecol(r,g,b,0);
			}
			fskip(fp,padd);
		}

	}
	fclose(fp);

 //	printf("BMP file readed: %d (%dx%d - %d bits) readed.\n",image_name,bmp->width,bmp->height,nr_bits);
	return bmp;
}

BITMAP * JPEGbitmapLoad(char * image_name){
	
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *infile;

	JSAMPARRAY buffer;

	int i,j,xx=0,yy=0;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);

	if((infile = fopen(image_name,"rb")) == NULL)
	{
		printf("Error opening file %s.\n",image_name);
		return 0;
	}

	jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	buffer=(*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width*cinfo.output_components, 1);

	i=cinfo.output_width*cinfo.output_components*sizeof(char);


	BITMAP * bmp = (BITMAP*)malloc(sizeof(BITMAP));

	bmp->width = cinfo.output_width;
	bmp->height = cinfo.output_height;	

	// try to allocate memory
	if ((bmp->data = (FB_pixel *) malloc(sizeof(FB_pixel)*(bmp->width*bmp->height))) == NULL)
	{
		fclose(infile);
		printf("Error allocating memory for file %s.\n",image_name);
		free(bmp);
		return NULL;
	}
	
	while(cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);
		j=0;
		while(j<=i)
		{
			bmp->data[yy*bmp->width + xx] = FB_makecol(buffer[0][j],buffer[0][j+1],buffer[0][j+2],0);
			j+=3;
			xx++;
		}
		yy++;
		xx=0;
	}

	jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	return bmp;
}

BITMAP * PNGbitmapLoad(char * image_name){


	int x, y;

	int width, height;

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep * row_pointers;

	char header[8];    // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(image_name, "rb");
	if (!fp){
		printf("Error opening file %s.\n",image_name);
		return 0;
	}
	fread(header, 1, 8, fp);
	if (png_sig_cmp((png_const_bytep) header, 0, 8)){
		printf("%s is not a png file.\n",image_name);
		fclose(fp);
		return 0;
	}

	// initialize stuff 
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr){
		printf("%s is not a png file.\n",image_name);
		fclose(fp);
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		printf("%s is not a png file.\n",image_name);
		fclose(fp);
		return 0;
	}

	if (setjmp(png_jmpbuf(png_ptr))){
		printf("%s is not a png file.\n",image_name);
		fclose(fp);
		return 0;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

//	png_byte color_type = png_get_color_type(png_ptr, info_ptr);
//	png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

//	int number_of_passes = png_set_interlace_handling(png_ptr);
//	png_read_update_info(png_ptr, info_ptr);


	// read file 
	if (setjmp(png_jmpbuf(png_ptr))){
		printf("%s is not a png file.\n",image_name);
		fclose(fp);
		return 0;
	}

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++)
			row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

	png_read_image(png_ptr, row_pointers);


	if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA){
		printf("%s color type not implemented. Must be RGBA.\n",image_name);
		fclose(fp);
		return 0;
	}





	BITMAP * bmp = (BITMAP*)malloc(sizeof(BITMAP));

	bmp->width = width;
	bmp->height = height;

	// try to allocate memory
	if ((bmp->data = (FB_pixel *) malloc(sizeof(FB_pixel)*(bmp->width*bmp->height))) == NULL)
	{
		fclose(fp);
		printf("Error allocating memory for file %s.\n",image_name);
		free(bmp);
		return NULL;
	}

	for (y=0; y<height; y++) {
			png_byte* row = row_pointers[y];
			for (x=0; x<width; x++) {
					png_byte* ptr = &(row[x*4]);
					bmp->data[y*width+x]=FB_makecol(ptr[0], ptr[1], ptr[2], ptr[3]);
			}
	}
	
	fclose(fp);
	return bmp;

}


BITMAP * FB_bitmapLoad(char * image_name){
	int siz = strlen(image_name);
	if(strcmp(image_name+siz-4,".bmp")==0)
		return BMPbitmapLoad(image_name);
	if(strcmp(image_name+siz-4,".jpg")==0 || strcmp(image_name+siz-5,".jpeg")==0)
		return JPEGbitmapLoad(image_name);
	if(strcmp(image_name+siz-4,".png")==0)
		return PNGbitmapLoad(image_name);
	
	printf("FB_bitmapLoad: file type not implemented.\n");
	
	return NULL;
} 

void FB_bitmapDestroy(BITMAP* bmp){
	if(bmp == NULL) return;

	free(bmp->data);
	free(bmp);
}

void FB_bitmapDraw_T(BITMAP *bmp,int x,int y, FB_pixel transp_color){
	if(bmp == NULL) return;

	int j,l;
	int bitmap_offset = 0;

	if(transp_color == -1) transp_color = bmp->data[0];
	for(j=0;j<bmp->height;j++)
	{
		for(l=0;l<bmp->width;l++)
			if(bmp->data[bitmap_offset+l] != transp_color)
				FB_putpixel(l+x,j+y,bmp->data[bitmap_offset+l]);
		bitmap_offset+=bmp->width;
	}
}

void FB_bitmapDraw(BITMAP *bmp,int x,int y){
	if(bmp == NULL) return;

	int j,l;
	int bitmap_offset = 0;

	for(j=0;j<bmp->height;j++)
	{
		for(l=0;l<bmp->width;l++)
			FB_putpixel(l+x,j+y,bmp->data[bitmap_offset+l]);
		bitmap_offset+=bmp->width;
	}
}

void FB_imageDraw_T(char * image_name, int x, int y,FB_pixel transp_color){
	BITMAP * bmp = FB_bitmapLoad(image_name);
	FB_bitmapDraw_T(bmp,x,y,transp_color);
	FB_bitmapDestroy(bmp);
}

void FB_imageDraw(char * image_name, int x, int y){
	BITMAP * bmp = FB_bitmapLoad(image_name);
	FB_bitmapDraw(bmp,x,y);
	FB_bitmapDestroy(bmp);
}
