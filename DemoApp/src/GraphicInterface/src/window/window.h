/*
 * window.h
 *
 *  Created on: 18 de Mar de 2013
 *      Author: fabio32883
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include "../widget/widget.h"

#ifndef _POINT_
#define _POINT_
typedef struct point_{
	int x;
	int y;
}Point;
#endif /* _POINT_ */


typedef struct _window{
	Point p1;
	Point p2;
	char * title;
	int title_len;
	W_List * widgets;
}Window;

typedef struct _windows{
	Window * w;
	struct _windows * next;
}Windows;


void windows_update();

Window* window_new(int x, int y, int width, int height, char * title);
void window_free(Window * w);

void window_activate(Window * w);
void window_redraw(Window * w);
void window_move(Window * w, int x, int y);
void window_change_size(Window * w, int width, int height);

W_Button* window_add_button(Window* w, char*text , int font_size , int x, int y,void (*action)(void*), void * param);
W_Label* window_add_label(Window* w, char*text , int font_size , int x, int y,int color);
W_ImageButton* window_add_imagebutton(Window* w, char * image_name , int x, int y, int trans_color,void (*action)(void*), void * param);


#endif /* WINDOW_H_ */
