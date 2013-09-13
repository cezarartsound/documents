/*
 * window.c
 *
 *  Created on: 18 de Mar de 2013
 *      Author: fabio32883
 */

#include "../widget/widget.h"
#include "../vga/vga.h"
#include "window.h"

#include <string.h>
#include <malloc.h>


#define WINDOW_CABSIZE 				25
#define WINDOW_CAB_FONT_COLOR 		VGA_get16bpp_color(90, 90, 100)
#define WINDOW_CAB_COLOR 			VGA_get16bpp_color(40, 40, 30)
#define WINDOW_COLOR 				WIN_COLOR
#define BACKGROUND_COLOR 			COLOR_BLACK


Windows * windows = 0;



void windows_update(){
	VGA_fill(BACKGROUND_COLOR);
	Windows * ww = windows;
	while(ww){
		window_redraw(ww->w);
		ww = ww->next;
	}
}




Window* window_new(int x, int y, int width, int height, char * title){
	Window * w = (Window*)malloc(sizeof(Window));
	w->p1.x = x;
	w->p1.y = y;
	w->p2.x = x+width;
	w->p2.y = y+height;
	w->title_len = strlen(title)+1;
	w->title = (char*)malloc(w->title_len);
	memcpy(w->title,title,w->title_len);
	w->widgets = 0;

	/*Windows * ww = windows; // Adicionar à cabeça
	windows = (Windows *)malloc(sizeof(Windows));
	windows->next = ww;
	windows->w = w;*/

	if(windows == 0){ // Adicionar à cauda
		windows = (Windows *)malloc(sizeof(Windows));
		windows->next = 0;
		windows->w = w;
	}else{
		Windows * ww = windows;
		while(ww->next != 0) ww = ww->next;

		ww->next = (Windows *)malloc(sizeof(Windows));
		ww->next->next = 0;
		ww->next->w = w;
	}



	return w;
}

void window_free(Window * w){
	if(w==0) return;

// Eliminar da lista de windows

	bool find = 0;
	Windows * ww = windows;
	if(ww->w == w){
		find = 1;
		windows = windows->next;
		free(ww);
	}else
		while(ww->next!=0){
			if(ww->next->w == w){
				find=1;
				Windows * ww1 = ww->next;
				ww->next = ww1->next;
				free(ww1);
				break;
			}
			ww = ww->next;
		}

	if(!find){ printf("Window to free not finded! \r\n");return;}
	else printf("Window is free!\r\n");

// Eliminar windgets desta janela

	W_List * l = w->widgets;
	while(l!=0){
		l->destroy(l->component);
		l = l->next;
	}


// Eliminar a janela

	free(w->title);
	free(w);
}

inline void window_activate(Window * w){
	if(w==0) return;
	window_redraw(w);
}

void window_redraw(Window * w){
	if(w==0) return;
	int x1 = w->p1.x;
	int y1 = w->p1.y;
	int x2 = w->p2.x;
	int y2 = w->p2.y;

	VGA_fill_rect(x1,y1,x2,y2,WINDOW_COLOR);
	VGA_rect(x1,y1,x2,y2,COLOR_BLACK);


	VGA_line(x1+2, y1+2, x1+2, y2-2,COLOR_DARK_GRAY);
	VGA_line(x1+2, y2-2, x2-2, y2-2,COLOR_DARK_GRAY);

	VGA_line(x1+2, y1+2, x2-2, y1+2,COLOR_LIGHT_GRAY);
	VGA_line(x2-2, y1+2, x2-2, y2-2,COLOR_LIGHT_GRAY);

	VGA_fill_rect(x1+3,y1+3,x2-3,y1+WINDOW_CABSIZE-1,WINDOW_CAB_COLOR);
	VGA_line(x1+1, y1+WINDOW_CABSIZE, x2-1, y1+WINDOW_CABSIZE,COLOR_BLACK);
	VGA_setFontSize(18);
	VGA_drawText(w->title,x1+7,y1+WINDOW_CABSIZE-6,WINDOW_CAB_FONT_COLOR);

	W_List * l = w->widgets;
	while(l!=0){
		l->redraw(l->component);
		l = l->next;
	}
}

void window_move(Window * w, int x, int y){
	if(w==0) return;
	int width = w->p2.x-w->p1.x;
	int height = w->p2.y-w->p1.y;
	w->p1.x += x;
	w->p1.y += y;
	w->p2.x = w->p1.x+width;
	w->p2.y = w->p1.y+height;


	W_List * l = w->widgets;
	while(l!=0){
		l->move(l->component,x,y);
		l = l->next;
	}
}

void window_change_size(Window * w, int width, int height){
	if(w==0) return;
	w->p2.x = w->p1.x+width;
	w->p2.y = w->p1.y+height;
	window_redraw(w);
}








W_Button* window_add_button(Window* w, char*text , int font_size , int x, int y,void (*action)(void*), void * param){
	if(w==0) return 0;
	W_Button* b = w_button_addto(&(w->widgets),text,font_size,w->p1.x+x+2,w->p1.y+y+WINDOW_CABSIZE+2,action,param);
	return b;
}

W_Label* window_add_label(Window* w, char*text , int font_size , int x, int y,int color){
	if(w==0) return 0;
	W_Label* b = w_label_addto(&(w->widgets),text,font_size,w->p1.x+x+2,w->p1.y+y+WINDOW_CABSIZE+2,color);
	return b;
}

W_ImageButton* window_add_imagebutton(Window* w, char * image_name , int x, int y, int trans_color,void (*action)(void*), void * param){
	if(w==0) return 0;
	W_ImageButton* b = w_imagebutton_addto(&(w->widgets),image_name,w->p1.x+x+2,w->p1.y+y+WINDOW_CABSIZE+2,trans_color,action,param);
	return b;
}

