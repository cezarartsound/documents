/*
 * font.h
 *
 *  Created on: 17 de Set de 2012
 *      Author: fabio32883
 */

#ifndef FONT_H_
#define FONT_H_


typedef struct _Font{
	int lenght;
	int sizex;
	int sizey;
	int * data;
}Font;


Font* Font_create(char* filename);
void Font_destroy(Font * font);
int Font_getFinalText(Font * font,char * text,int xpos);
int Font_findChar(Font * font,char c);
int Font_drawText(Font * font, char* text,int xpos, int ypos, void (*setpixel)(int x, int y));

#endif /* FONT_H_ */
