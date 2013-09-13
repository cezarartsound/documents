/*
 * vgatest.c
 *
 *  Created on: 20 de Jul de 2012
 *      Author: fabio32883
 */
#include <malloc.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <math.h>
#include "ezdib/ezdib.h"
#include "font/font.h"
#include "vga.h"



int fbfd = 0;
struct fb_var_screeninfo VGA_vinfo;
struct fb_fix_screeninfo VGA_finfo;
long int screensize = 0;
char *fbp = 0;

HEZDIMAGE hDib;

#define MAX_FONT_SIZE 30

Font * fonts[MAX_FONT_SIZE];
Font * font;
int font_size;

void set_pixel(void* pUser, int x, int y, int color , int f){
	VGA_set_pixel(x,y,color);
}

void VGA_init(){

	// Open the file for reading and writing
	fbfd = open("/dev/fb0", O_RDWR);
	if (!fbfd) {
		printf("Error: cannot open framebuffer device.\n");
		exit(1);
	}
	printf("The framebuffer device was opened successfully.\n");

	// Get fixed screen information
	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &VGA_finfo)) {
		printf("Error reading fixed information. (%d)\n",errno);
		exit(2);
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &VGA_vinfo)) {
		printf("Error reading variable information. (%d)\n",errno);
		exit(3);
	}

	printf("%dx%d, %dbpp\n", VGA_vinfo.xres, VGA_vinfo.yres, VGA_vinfo.bits_per_pixel );


	// Figure out the size of the screen in bytes
	screensize = VGA_vinfo.xres * VGA_vinfo.yres * VGA_vinfo.bits_per_pixel / 8;

	// Map the device to memory
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
					   fbfd, 0);
	if ((int)fbp == -1) {
		printf("Error: failed to map framebuffer device to memory.\n");
		exit(4);
	}
	printf("The framebuffer device was mapped to memory successfully.\n");



	char font_name[18] = DEFAULT_FONT_NAME;
	font_name[11] += DEFAULT_FONT_SIZE/10;
	font_name[12] += DEFAULT_FONT_SIZE%10;

	font = Font_create(font_name);
	font_size = DEFAULT_FONT_SIZE;
	fonts[DEFAULT_FONT_SIZE] = font;

	hDib = ezd_create( VGA_vinfo.xres, VGA_vinfo.yres, VGA_vinfo.bits_per_pixel, 0 );

	if ( !hDib ){printf("ERROR creating hDib\r\n");return;}

	ezd_set_pixel_callback( hDib, (t_ezd_set_pixel) set_pixel, 0 );
}

void VGA_exit(){
    munmap(fbp, screensize);
    close(fbfd);
}

Font* VGA_loadFont(char* font_name){
	Font * new_font = Font_create(font_name);
	if(new_font == 0){
		printf("Error loading font: %s ...\r\n", font_name);
		return 0;
	}else{
		printf("Loaded font: %s ...\r\n", font_name);
	}

	return new_font;
}

int VGA_setFontSize(int size){
	if(size>MAX_FONT_SIZE || size < 0) return 0;

	if(size != font_size){
		if(fonts[size] != 0){
			font = fonts[size];
			font_size = size;
			return 1;
		}

		char font_name[18] = DEFAULT_FONT_NAME;
		font_name[11] += size/10;
		font_name[12] += size%10;

		if((fonts[size]=VGA_loadFont(font_name))!=0){
			font = fonts[size];
			font_size = size;
			return 1;
		}
		return 0;
	}
	return 1;
}

void VGA_paint(int x, int y,int w, int h, int r, int g, int b){
	long int location;

	for ( ; y < 300; y++ )
			for ( ; x < 300; x++ ) {

				location = (x+VGA_vinfo.xoffset) * (VGA_vinfo.bits_per_pixel/8) +
						   (y+VGA_vinfo.yoffset) * VGA_finfo.line_length;

			/*	if ( vinfo.bits_per_pixel == 32 ) {
					*(fbp + location) = 100;        // Some blue
					*(fbp + location + 1) = 200+(x-100)/2;     // A little green
					*(fbp + location + 2) = 15-(y-100)/5;    // A lot of red
					*(fbp + location + 3) = 0;      // No transparency
				} else  { //assume 16bpp*/
					unsigned short int t = r<<11 | g << 5 | b;
					*((unsigned short int*)(fbp + location)) = t;
				//}

			}
}

#define VGA_direct_set_pixel(fbp_idx, value) (*((unsigned short int*)(fbp + (fbp_idx))) =(unsigned short) value)

void VGA_set_pixel(int x, int y, int color ){
	long int location = (x+VGA_vinfo.xoffset) * (VGA_vinfo.bits_per_pixel/8) +
			(y+VGA_vinfo.yoffset) * VGA_finfo.line_length;

	unsigned short int t = color;
	*((unsigned short int*)(fbp + location)) = t;
}

int color_ref;
void set_pixel_ref(int x, int y){
	VGA_set_pixel( x, y, color_ref);
}

unsigned short VGA_8bpp_to_16bpp(byte b8){
	int r,g,b;
	/*r = b8>>5;
	g = b8>>2&0b111;
	b = b8&0b11;
	return VGA_get16bpp_color((r*100)/8,(g*100)/8,(b*100)/4);*/


	r = (b8>>2)&1;
	g = (b8>>1)&1;
	b = b8&1;
	if(b8>>3){r*=100;g*=100;b*=100;}
	else{r*=50;g*=50;b*=50;}
	return VGA_get16bpp_color(r,g,b);
}


int VGA_drawButton(char* text,int xpos, int ypos){
	int xf = Font_getFinalText(font,text,xpos+font->sizex);


	ezd_fill_rect(hDib,xpos, ypos, xf+font->sizex, ypos+font->sizey+font->sizey/2,WIN_COLOR);

	ezd_rect(hDib,xpos, ypos, xf+font->sizex, ypos+font->sizey+font->sizey/2,0);

	color_ref = 0;
	Font_drawText(font, text,xpos+font->sizex,ypos+font->sizey/6+font->sizey, set_pixel_ref);

	ezd_line(hDib,xpos+1, ypos+1, xf+font->sizex-1, ypos+1,VGA_get16bpp_color(100, 100, 100));
	ezd_line(hDib,xf+font->sizex-1, ypos+1, xf+font->sizex-1, ypos+font->sizey+font->sizey/2-1,VGA_get16bpp_color(100, 100, 100));

	ezd_line(hDib,xpos+1, ypos+2, xpos+1, ypos+font->sizey+font->sizey/2-1,VGA_get16bpp_color(30, 30, 30));
	ezd_line(hDib,xpos+1, ypos+font->sizey+font->sizey/2-1, xf+font->sizex-2, ypos+font->sizey+font->sizey/2-1,VGA_get16bpp_color(30, 30, 30));

	return xf+font->sizex;
}

int VGA_drawButton_end(char* text,int xpos, int ypos){
	int xf = Font_getFinalText(font,text,xpos+font->sizex);

	return xf+font->sizex;
}

void VGA_drawButtonClick(char * text, int xpos, int ypos){
	int xf = Font_getFinalText(font,text,xpos+font->sizex);

	ezd_fill_rect(hDib,xpos, ypos, xf+font->sizex, ypos+font->sizey+font->sizey/2,WIN_COLOR);

	ezd_rect(hDib,xpos, ypos, xf+font->sizex, ypos+font->sizey+font->sizey/2,0);

	color_ref = 0;
	Font_drawText(font, text,xpos+font->sizex,ypos+font->sizey/6+font->sizey, set_pixel_ref);

	ezd_line(hDib,xpos+1, ypos+1, xf+font->sizex-1, ypos+1,VGA_get16bpp_color(30, 30, 30));
	ezd_line(hDib,xf+font->sizex-1, ypos+1, xf+font->sizex-1, ypos+font->sizey+font->sizey/2-1,VGA_get16bpp_color(30, 30, 30));

	ezd_line(hDib,xpos+1, ypos+2, xpos+1, ypos+font->sizey+font->sizey/2-1,VGA_get16bpp_color(100, 100, 100));
	ezd_line(hDib,xpos+1, ypos+font->sizey+font->sizey/2-1, xf+font->sizex-2, ypos+font->sizey+font->sizey/2-1,VGA_get16bpp_color(100, 100, 100));

}

void VGA_drawText(char*text, int xpos, int ypos, int color){
	color_ref = color;
	Font_drawText(font,text,xpos,ypos,set_pixel_ref);
}

inline void VGA_line(int x1,int y1, int x2, int y2, int color){
	ezd_line(hDib,x1, y1, x2, y2,color);
}

inline void VGA_rect(int x1,int y1, int x2, int y2, int color){
	ezd_rect(hDib,x1,y1,x2, y2,color);
}

inline void VGA_circle(int x, int y, int r, int color){
	ezd_circle( hDib, x, y, r, color );
}

void VGA_fill_circle(int x, int y, int r, int color){
	while((r--)>1)
		ezd_circle( hDib, x, y, r, color );
}

inline void VGA_fill_rect(int x1,int y1, int x2, int y2, int color){
	ezd_fill_rect(hDib,x1,y1,x2, y2,color);
}

inline void VGA_fill(int color){
	VGA_fill_rect(1,1,VGA_vinfo.xres-1, VGA_vinfo.yres-1,color);
	//ezd_fill(hDib,color);
}

int i_;
#define fskip(FD,N) for(i_=0;i_<N;i_++)fgetc(FD)

BITMAP * VGA_loadBitmap(char * image_name){
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
	if ((bmp->data = (word *) malloc(sizeof(word)*(bmp->width*bmp->height))) == NULL)
	{
		fclose(fp);
		printf("Error allocating memory for file %s.\n",image_name);
		free(bmp);
		return 0;
	}

	int r,g,b;
	// read the bitmap
	if(nr_bits==8){
		for(index=(bmp->height-1)*bmp->width;index>=0;index-=bmp->width)
			for(x=0;x<bmp->width;x++)
				bmp->data[(word)index+x]=VGA_8bpp_to_16bpp(fgetc(fp));
	}else if(nr_bits==24){
		int padd = 0;
		while ((bmp->width * 24 + padd*8)%32 != 0 ) padd++;
		for(index=(bmp->height-1)*bmp->width;index>=0;index-=bmp->width){
			for(x=0;x<bmp->width;x++){
				b = fgetc(fp);
				g = fgetc(fp);
				r = fgetc(fp);
				bmp->data[(word)index+x]=(word)VGA_get16bpp_color((r*100)/255,(g*100)/255,(b*100)/255);
			}
			fskip(fp,padd);
		}

	}
	fclose(fp);

	printf("BMP (%dx%d - %d bits) readed: %s.\n",bmp->width,bmp->height,nr_bits,image_name);
	return bmp;
}

void VGA_destroyBitmap(BITMAP* bmp){
	free(bmp->data);
	free(bmp);
}

void VGA_drawBitmap_Transparent(BITMAP *bmp,int x,int y, int transp_color){

	int j,l;
	word bitmap_offset = 0;
	long int screen_offset = (x+VGA_vinfo.xoffset) * (VGA_vinfo.bits_per_pixel/8) +
	(y+VGA_vinfo.yoffset) * VGA_finfo.line_length;

	if(transp_color == -1) transp_color = bmp->data[0];
	for(j=0;j<bmp->height;j++)
	{
		for(l=0;l<bmp->width;l++)
			if(bmp->data[bitmap_offset+l] != transp_color)
				VGA_direct_set_pixel(screen_offset+l*(VGA_vinfo.bits_per_pixel/8),bmp->data[bitmap_offset+l]);
		bitmap_offset+=bmp->width;
		screen_offset+=VGA_finfo.line_length;
	}
}

void VGA_drawBitmap(BITMAP *bmp,int x,int y){
	int j,l;

	long int screen_offset = (x+VGA_vinfo.xoffset) * (VGA_vinfo.bits_per_pixel/8) +
						(y+VGA_vinfo.yoffset) * VGA_finfo.line_length;
	word bitmap_offset = 0;

	for(j=0;j<bmp->height;j++)
	{
		memcpy((&fbp[screen_offset]),(&bmp->data[bitmap_offset]),bmp->width*(VGA_vinfo.bits_per_pixel/8));

		bitmap_offset+=bmp->width;
		screen_offset+=VGA_finfo.line_length;
	}
}


void VGA_drawImage_Transparent(char * image_name, int x, int y, int transp_color){
	BITMAP bmp;
	FILE *fp;
		long index;
		word num_colors, nr_bits;
		int xx;

	// open the file
	if ((fp = fopen(image_name,"rb" )) == NULL)
	{
		printf("Error opening file %s.\n",image_name);
		exit(1);
	}

	char buff[2];
	fread(buff,1,2,fp);

	if (buff[0]!='B' || buff[1]!='M')
	{
		fclose(fp);
		printf("%s is not a bitmap file.\n",image_name);
		exit(1);
	}
	// read in the width and height of the image, and the number of colors used; ignore the rest
	fskip(fp,8);
	word offset; fread(&offset, sizeof(word), 1, fp);
	fskip(fp,6);
	fread(&bmp.width, sizeof(word), 1, fp);
	fskip(fp,2);
	fread(&bmp.height,sizeof(word), 1, fp);
	fskip(fp,4);
	fread(&nr_bits,sizeof(word), 1, fp);
	fskip(fp,16);
	fread(&num_colors,sizeof(word), 1, fp);

	if (num_colors==0) num_colors=256;
	fskip(fp,offset-48);//fskip(fp,6+num_colors*4);	// Ignore the rest of header and palette information for now.


	long int screen_offset = (x+VGA_vinfo.xoffset) * (VGA_vinfo.bits_per_pixel/8) +
							(y+bmp.height+VGA_vinfo.yoffset) * VGA_finfo.line_length;

	int r,g,b,value;
	// read the bitmap
	if(nr_bits==8){
		value = VGA_8bpp_to_16bpp(fgetc(fp));
		xx = 1;
		if(transp_color == -1) transp_color = value;
		for(index=(bmp.height-1)*bmp.width;index>=0;index-=bmp.width){
			for(;xx<bmp.width;xx++){
				value = VGA_8bpp_to_16bpp(fgetc(fp));
				if(value != transp_color)
					VGA_direct_set_pixel(screen_offset+xx*(VGA_vinfo.bits_per_pixel/8),value);
			}
			xx=0;
		}
	}else if(nr_bits==24){
		b = fgetc(fp);
		g = fgetc(fp);
		r = fgetc(fp);
		value = VGA_get16bpp_color((r*100)/255,(g*100)/255,(b*100)/255);
		xx = 1;
		if(transp_color == -1) transp_color = value;

		int padd = 0;
		while ((bmp.width * 24 + padd*8)%32 != 0 ) padd++;
		for(index=(bmp.height-1)*bmp.width;index>=0;index-=bmp.width){
			for(;xx<bmp.width;xx++){
				b = fgetc(fp);
				g = fgetc(fp);
				r = fgetc(fp);
				value = VGA_get16bpp_color((r*100)/255,(g*100)/255,(b*100)/255);
				if(value != transp_color)
					VGA_direct_set_pixel(screen_offset+xx*(VGA_vinfo.bits_per_pixel/8),value);
			}
			xx=0;
			fskip(fp,padd);
			screen_offset-=VGA_finfo.line_length;
		}

	}
	fclose(fp);

	printf("BMP (%dx%d - %d bits) readed: %s.\n",bmp.width,bmp.height,nr_bits,image_name);
}
void VGA_drawImage(char * image_name, int x, int y){

	BITMAP bmp;
	FILE *fp;
		long index;
		word num_colors, nr_bits;
		int xx;

	// open the file
	if ((fp = fopen(image_name,"rb" )) == NULL)
	{
		printf("Error opening file %s.\n",image_name);
		exit(1);
	}

	char buff[2];
	fread(buff,1,2,fp);

	if (buff[0]!='B' || buff[1]!='M')
	{
		fclose(fp);
		printf("%s is not a bitmap file.\n",image_name);
		exit(1);
	}

	// read in the width and height of the image, and the number of colors used; ignore the rest
	fskip(fp,8);
	word offset; fread(&offset, sizeof(word), 1, fp);
	fskip(fp,6);
	fread(&bmp.width, sizeof(word), 1, fp);
	fskip(fp,2);
	fread(&bmp.height,sizeof(word), 1, fp);
	fskip(fp,4);
	fread(&nr_bits,sizeof(word), 1, fp);
	fskip(fp,16);
	fread(&num_colors,sizeof(word), 1, fp);


	if (num_colors==0) num_colors=256;
	fskip(fp,offset-48);//fskip(fp,6+num_colors*4);	// Ignore the rest of header and palette information for now.


	long int screen_offset = (x+VGA_vinfo.xoffset) * (VGA_vinfo.bits_per_pixel/8) +
							(y+bmp.height+VGA_vinfo.yoffset) * VGA_finfo.line_length;


	int r,g,b;
	// read the bitmap
	if(nr_bits==8){
		for(index=(bmp.height-1)*bmp.width;index>=0;index-=bmp.width)
			for(xx=0;xx<bmp.width;xx++) // TODO Confirmar se não sai do ecra (FODE TUDO, palavra de quem sabe) (e nos outros)
				VGA_direct_set_pixel(screen_offset+xx*(VGA_vinfo.bits_per_pixel/8),VGA_8bpp_to_16bpp(fgetc(fp)));
	}else if(nr_bits==24){
		int padd = 0;
		while ((bmp.width * 24 + padd*8)%32 != 0 ) padd++;
		for(index=(bmp.height-1)*bmp.width;index>=0;index-=bmp.width){
			for(xx=0;xx<bmp.width;xx++){
				b = fgetc(fp);
				g = fgetc(fp);
				r = fgetc(fp);
				VGA_direct_set_pixel(screen_offset+xx*(VGA_vinfo.bits_per_pixel/8),VGA_get16bpp_color((r*100)/255,(g*100)/255,(b*100)/255));
			}
			fskip(fp,padd);
			screen_offset-=VGA_finfo.line_length;
		}

	}
	fclose(fp);

	printf("BMP (%dx%d - %d bits) readed: %s.\n",bmp.width,bmp.height,nr_bits,image_name);
}
