/*! \file FBfont.c
 * \brief Font functions
 * 
 * All font loading and printing is grouped here.\n
 * begin                : Fri Feb 8 2002\n
 * copyright            : (C) 2000 by Daniele Venzano\n
 * email                : venza@users.sf.net */


/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License. */

#include <stdio.h>
#include <stdarg.h>
#include "FBlib.h"

struct font
{
	char **glyphs;
	char filemode;
	char fontheight;
	int len;
};

void insert_glyph(int num, char* glyph);
void delete_all_glyphs();
char *get_glyph(int num);

FILE *gunzip(FILE* fp, char* path);

struct font current_font;

int FB_change_font(char *psf_file)
{
	FILE *font;
	unsigned char byte;
	int j,k;
	char *glyph;

	printf("Try open font: %s\n", psf_file);
	
	font = fopen(psf_file, "r");
	if(font == NULL)
	{
		fprintf(stderr, "%s was not found", psf_file);
		return PARAM_ERR;
	}
		byte = fgetc(font);
	if(byte == 31) /* gzipped psf */
	{
		byte = fgetc(font);
		if(byte == 139)
		{
			font = gunzip(font, psf_file);
			byte = fgetc(font);
		}
	}
	if(byte != 0x36)
	{
		fprintf(stderr, "%s is not a psf font file\n", psf_file);
		return PARAM_ERR;
	}
	byte = fgetc(font);
	if(byte != 0x04)
	{
		fprintf(stderr, "%s is not a psf font file\n", psf_file);
		return PARAM_ERR;
	}
	delete_all_glyphs();
	current_font.filemode = fgetc(font);
	if(current_font.filemode == 1 || current_font.filemode == 3)
		current_font.len = 512;
	if(current_font.filemode == 0 || current_font.filemode == 2)
		current_font.len = 256;
	current_font.fontheight = fgetc(font);
	glyph = malloc(current_font.fontheight);
	for(k=0; k < current_font.len; k++)
	{
		for(j=0; j < current_font.fontheight; j++)
			glyph[j] = fgetc(font);
		insert_glyph(k, glyph);
	}
	free(glyph);

	printf("Font readed: %s\n",psf_file);

	return OK;
}

int FB_putc(int c, int x, int y, FB_pixel col)
{
	char *glyph;
	int i,j,k=0;
	
	if(c > 256 && current_font.len == 256)
		return PARAM_ERR;
	if((glyph = get_glyph(c)) != NULL)
		for(i = 0; i < current_font.fontheight; i++)
		{
			for(j = 128; j > 0; j/=2)
			{
				if(glyph[i] & j)
					FB_putpixel(x+k, y+i, col);
				k++;
			}
			k=0;
		}
	return OK;
}

int FB_printf(int x, int y, FB_pixel col, char *format, ...)
{
	va_list ap;
	char string[20];
	int len,i,row=0, column=0, j;

	len = strlen(format);
	va_start(ap, format);
	for(i=0; i < len; i++)
	{
		if(format[i] != '%' && format[i] != '\\')
		{
			FB_putc(format[i], x+(column*8), y+row, col);
			column++;
		}
		if(format[i] == '\\')
		{
			if(format[i+1] == 'n')
			{
				row++;
				column=0;
			}
			else if(format[i+1] == 'r')
				column=0;
			i++;
			continue;
		}
		if(format[i] == '%')
		{
			switch(format[i+1])
			{
				case '%': sprintf(string, "%%"); break;
				case 'd': sprintf(string, "%d", va_arg(ap, int)); break;
				case 'x': sprintf(string, "%x", va_arg(ap, int)); break;
//				case 'c': sprintf(string, "%c", va_arg(ap, char)); break;
				case 'c': sprintf(string, "%c", (char)va_arg(ap, int)); break;
				case 'g': sprintf(string, "%g", va_arg(ap, double)); break;
			}	  
			for(j=0; j < strlen(string); j++)
			{
				FB_putc(string[j], x+(column*8), y+row, col);
				column++;
			}
			i++;
			continue;
		}
	}
	va_end(ap);
	return 0;
}

void insert_glyph(int num, char* glyph)
{
	char* aux;
	
	if(current_font.glyphs == NULL)
		current_font.glyphs = malloc(sizeof(char*) * current_font.len);
	aux = malloc(current_font.fontheight);
	memcpy(aux, glyph, current_font.fontheight);
	current_font.glyphs[num] = aux;

	return;
}

void delete_all_glyphs()
{
	int i;
	
	for(i=0; i<current_font.len; i++)
		free(current_font.glyphs[i]);
	free(current_font.glyphs);

	return;
}

char *get_glyph(int num)
{
	if(current_font.glyphs != NULL)
		return current_font.glyphs[num];
	else
		return NULL;
}

FILE *gunzip(FILE* fp, char* path)
{
	char command[200];
	
	fclose(fp);
	sprintf(command, "gunzip -c %s > /tmp/FBfont.psf", path);
	system(command);
	fp = fopen("/tmp/FBfont.psf", "r");
	system("rm /tmp/FBfont.psf");
	return fp;
}

