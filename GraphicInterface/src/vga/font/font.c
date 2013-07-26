/*
 * font.c
 *
 *  Created on: 17 de Set de 2012
 *      Author: fabio32883
 */

#include "font.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>


Font* Font_create(char* filename){
	int idx = 0,i;
	int nr_bytes = 0;
	char code[100], c;
	char * codep;

	FILE * filer = fopen(filename,"r");
	if (filer == 0) {perror ("Error opening file"); return 0;}

	Font* fontr = (Font*)malloc(sizeof(Font));
	int* data = (int*)malloc(50000);

	while(1){

		if(fread(code,1,4,filer)<4) break;
		code[4] = '\0';
		if(strcmp(code,"BBX ") == 0){
			codep = code;
			do{
			   fread(codep,1,1,filer);
			}while(*(codep++) != '\n');
			*codep = '\0';

			*(data+idx) = strtol (code,&codep,10);
			*(data+idx+1) = strtol (codep,&codep,10);
			*(data+idx+2) = strtol (codep,&codep,10);
			*(data+idx+3) = strtol (codep,&codep,10);
			nr_bytes = *(data+idx+1);
			idx+=4;
			//printf("## BBX %d %d %d %d\r\n",*(data+idx-4),*(data+idx-3),*(data+idx-2),*(data+idx-1));
			continue;
		}


		if(fread(code+4,1,3,filer)<3) break;
		code[7] = '\0';
		if(strcmp(code,"BITMAP\n") == 0){
			//printf("BITMAP ");
			for(i=0;i<nr_bytes;i++){
				codep = code;
				do{
				   fread(codep,1,1,filer);
				}while(*(codep++) != '\n');
				*codep = '\0';

				*(data+idx) = strtol (code,0,16);
				idx++;
				//printf("%x ",*(data+idx-1));
			}
			//printf("\r\n");
			continue;
		}


		if(fread(code+7,1,2,filer)<2) break;
		code[9] = '\0';
		if(strcmp(code,"ENCODING ") == 0){
			codep = code;
			do{
			   fread(codep,1,1,filer);
			}while(*(codep++) != '\n');
			*codep = '\0';

			*(data+idx) = strtol (code,0,10);
			if(*(data+idx) > 200) break;
			idx++;

			//printf("ENCONDING %d (%c) \r\n",*(data+idx-1),*(data+idx-1));
			continue;
		}else if(strcmp(code,"X_HEIGHT ") == 0){
			codep = code;
			do{
			   fread(codep,1,1,filer);
			}while(*(codep++) != '\n');
			*codep = '\0';

			fontr->sizex = strtol (code,0,10);

			//printf("X_H %d \r\n",fontr->sizex);
			continue;
		}


		if(fread(code+9,1,2,filer)<2) break;
		code[11] = '\0';
		if(strcmp(code,"PIXEL_SIZE ") == 0){
			codep = code;
			do{
			   fread(codep,1,1,filer);
			}while(*(codep++) != '\n');
			*codep = '\0';

			fontr->sizey = strtol (code,0,10);

			//printf("Y_H %d \r\n",fontr->sizey);
			continue;
		}

		while((c=fgetc(filer))!='\n')if(c==EOF)break;
	}

	fontr->lenght = idx;
	fontr->data = (int*)malloc(idx*sizeof(int));
	memcpy(fontr->data,data,idx*sizeof(int));

	printf("Created font with %d bytes. \r\n",idx);
	free(data);

	fclose(filer);
	return fontr;
}

int Font_getFinalText(Font * font,char * text,int xpos){
	int idx;
	int ptr = xpos;

	while(*text != '\0'){
		idx = Font_findChar(font,*text);
		if(idx>=0){
			ptr += font->sizex+font->sizex/3-font->data[idx+3];
		}
		text ++;
	}
	return ptr;
}

int Font_findChar(Font * font,char c){
	int idx = 0;
	char f;
	while(idx<font->lenght){
		f = (char)font->data[idx];
		if(f==c)return idx;
		idx += font->data[idx+2] + 5;
	}
	return -1;
}

int Font_drawText(Font * font, char* text,int xpos, int ypos, void (*setpixel)(int x, int y)){
	int idx, xf,yf, x,y,xx;
	int mask;
	int ptr = xpos;

	while(*text != '\0'){
		idx = Font_findChar(font, *text);

		if(font->data[idx+1]<9) mask = 0x80;
		else if(font->data[idx+1]<17) mask = 0x8000;
		else mask = 0;

		if(idx>=0){
			xf = ptr;//+scissFont[idx+3];
			yf= ypos-font->data[idx+4];
			for(y=yf-font->data[idx+2];y<yf;y++){
				xx = font->data[idx+5+y-(yf-font->data[idx+2])];
				for(x=xf;x<xf+font->data[idx+1];x++){
					if( (xx&mask) != 0)
						setpixel(x, y);
					xx=xx<<1;
				}
			}
			ptr += font->sizex+font->sizex/3-font->data[idx+3];//scissFont[idx+1];
		}

		text++;
	}

	return ptr;
}

void Font_destroy(Font * font){
	free(font->data);
	free(font);
}

