/*
 * widget.c
 *
 *  Created on: 14 de Nov de 2012
 *      Author: fabio32883
 */

#include <sys/time.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "../event/event.h"
#include "../input/input.h"
#include "../vga/vga.h"
#include "widget.h"

W_List * widgets;

void widget_init(){
	widgets = 0;
}

void widget_initall(){
	VGA_init();
	input_init();
	event_init();
	widget_init();
}

void widget_exit(){
	widget_free_all();
}

void widget_exitall(){
	widget_exit();
	event_stop();
	input_stop();
	VGA_exit();
}

void w_clean_screen(){
	VGA_rect(10,10,VGA_vinfo.xres-10, VGA_vinfo.yres-10,COLOR_BLACK);
	widget_free_all();
}

/**
 * W_BUTTON
 */
void _w_button_press(void * v){
	W_Button * b = (W_Button*)v;

	Point p;
	int in = input_getClick(&p);

	if(in!=b->last_input){
		//printf("%d , %d\r\n",in, b->last_input);
		b->last_input = in;
		if(p.x > b->location.x && p.x < b->end.x && p.y >b->location.y && p.y<b->end.y){
			if(!b->pressed){
				VGA_setFontSize(b->font_size);
				VGA_drawButtonClick(b->text,b->location.x,b->location.y);
				b->pressed = 1;
				if(b->action != 0)
					b->action(b->parameter);
			}

			struct timeval curr_time;
			if ( gettimeofday( &curr_time,NULL ) >= 0 ){
				b->last_time = (curr_time.tv_usec/1000 + curr_time.tv_sec*1000)  ;
			}
		}
	}
}

void _w_button_release(void * v){
	W_Button * b = (W_Button*)v;

	if(b->pressed){
		struct timeval curr_time;

		if ( gettimeofday( &curr_time,NULL ) < 0 ){
			return ;
		}

		if((curr_time.tv_usec/1000 + curr_time.tv_sec*1000)  - b->last_time >= W_BUTTON_PRESS_MS){
			VGA_setFontSize(b->font_size);
			VGA_drawButton(b->text,b->location.x,b->location.y);
			b->pressed = 0;
		}
	}
}

void _w_button_free(void * obj){
	W_Button * b = (W_Button *)obj;
	event_remove(b->event_press);
	event_remove(b->event_release);
	free(b->text);
	free(b);
}

void _w_button_redraw(void * obj){
	W_Button * b = (W_Button *)obj;
	VGA_setFontSize(b->font_size);
	if(b->pressed)
		VGA_drawButtonClick(b->text,b->location.x,b->location.y);
	else
		VGA_drawButton(b->text,b->location.x,b->location.y);
}

void _w_button_move(void * obj, int x, int y){
	W_Button * b = (W_Button * )obj;
	b->location.x += x;
	b->location.y += y;
	b->end.x = VGA_drawButton_end(b->text,b->location.x,b->location.y);
	b->end.y = b->location.y+b->font_size+6;
}

W_Button* _w_button(char*text , int font_size , int x, int y, void (*action)(void*), void * param, bool draw){

	W_Button * b = (W_Button*)malloc(sizeof(W_Button));
	b->text_len = strlen(text)+1;
	b->text = (char*)malloc(b->text_len);
	memcpy(b->text,text,b->text_len);
	b->event_press  = event_add(_w_button_press,b);
	b->event_release  = event_add(_w_button_release,b);
	b->location.x = x;
	b->location.y = y;
	b->font_size = font_size;
	b->pressed = 0;
	b->action = action;
	b->parameter = param;
	b->last_input = input_getClick(0);
	VGA_setFontSize(font_size);
	b->end.x = (draw?VGA_drawButton(text,x,y):VGA_drawButton_end(text,x,y));
	b->end.y = y+font_size+6;

	struct timeval curr_time;
	if ( gettimeofday( &curr_time,NULL ) >= 0 ){
		b->last_time = (curr_time.tv_usec/1000 + curr_time.tv_sec*1000)  ;
	}

	return b;
}

W_Button* w_button_add(char*text , int font_size , int x, int y, void (*action)(void*), void * param){

	W_Button * b = _w_button(text,font_size,x,y,action,param,1);

	W_List * l = (W_List*)malloc(sizeof(W_List));
	l->type = W_BUTTON;
	l->component = b;
	l->destroy = _w_button_free;
	l->redraw = _w_button_redraw;
	l->move = _w_button_move;
	l->next = widgets;

	widgets = l;

	return b;
}

W_Button* w_button_addto(W_List** list,char*text , int font_size , int x, int y, void (*action)(void*), void * param){

	W_Button * b = _w_button(text,font_size,x,y,action,param,0);

	W_List * l = (W_List*)malloc(sizeof(W_List));
	l->type = W_BUTTON;
	l->component = b;
	l->destroy = _w_button_free;
	l->redraw = _w_button_redraw;
	l->move = _w_button_move;
	l->next = (*list);

	(*list) = l;

	return b;
}


/**
 * W_IMAGEBUTTON
 */
void _w_imagebutton_press(void * v){ // TODO criar event em funcao de input, para evitar estas funcoes
	W_ImageButton * b = (W_ImageButton*)v;

	Point p;
	int in = input_getClick(&p);

	if(in!=b->last_input){
		b->last_input = in;
		if(p.x > b->location.x && p.x < b->end.x && p.y >b->location.y && p.y<b->end.y){
			if(!b->pressed){
				VGA_rect(b->location.x+2,b->location.y+2,b->end.x-2,b->end.y-2,COLOR_LIGHT_GRAY);
				b->pressed = 1;
				if(b->action != 0)
					b->action(b->parameter);
			}

			struct timeval curr_time;
			if ( gettimeofday( &curr_time,NULL ) >= 0 ){
				b->last_time = (curr_time.tv_usec/1000 + curr_time.tv_sec*1000)  ;
			}
		}
	}
}

void _w_imagebutton_releasedraw(W_ImageButton * b){
	BITMAP * bmp = b->image;
	int x = b->location.x;
	int y = b->location.y;

	int j,l;
	word bitmap_offset = 0;

	int transp_color = bmp->data[0];
	for(j=0;j<bmp->height;j++)
	{
		if(j==2 || j==bmp->height-2)
			for(l=0;l<bmp->width;l++)
				if(bmp->data[bitmap_offset+l] != transp_color)
					VGA_set_pixel(l+x,j+y,bmp->data[bitmap_offset+l]);
				else
					VGA_set_pixel(l+x,j+y,b->trans_color);
		else
			for(l=2;l<=bmp->width-2;l+=bmp->width-4)
				if(bmp->data[bitmap_offset+l] != transp_color)
					VGA_set_pixel(l+x,j+y,bmp->data[bitmap_offset+l]);
				else
					VGA_set_pixel(l+x,j+y,b->trans_color);
		bitmap_offset+=bmp->width;
	}
}

void _w_imagebutton_release(void * v){
	W_ImageButton * b = (W_ImageButton*)v;

	if(b->pressed){
		struct timeval curr_time;

		if ( gettimeofday( &curr_time,NULL ) < 0 ){
			return ;
		}

		if((curr_time.tv_usec/1000 + curr_time.tv_sec*1000) - b->last_time >= 500){
			_w_imagebutton_releasedraw(b);
			b->pressed = 0;
		}
	}
}

void _w_imagebutton_free(void * obj){
	W_ImageButton * b = (W_ImageButton * )obj;
	event_remove(b->event_press);
	event_remove(b->event_release);
	VGA_destroyBitmap(b->image);
	free(b);
}

void _w_imagebutton_redraw(void * obj){
	W_ImageButton * b = (W_ImageButton * )obj;
	if(b->pressed){
		VGA_drawBitmap_Transparent(b->image,b->location.x,b->location.y,b->trans_color);
		VGA_rect(b->location.x+2,b->location.y+2,b->end.x-2,b->end.y-2,COLOR_LIGHT_GRAY);
	}else
		_w_imagebutton_releasedraw(b);
}

void _w_imagebutton_move(void * obj, int x, int y){
	W_ImageButton * b = (W_ImageButton * )obj;
	b->location.x += x;
	b->location.y += y;
	b->end.x = b->location.x+b->image->width;
	b->end.y = b->location.y+b->image->height;
}

W_ImageButton * _w_imagebutton(char * image_name , int x, int y, int trans_color,void (*action)(void*), void * param, bool draw){
	W_ImageButton * b = (W_ImageButton*)malloc(sizeof(W_ImageButton));
	b->event_press  = event_add(_w_imagebutton_press,b);
	b->event_release  = event_add(_w_imagebutton_release,b);
	b->location.x = x;
	b->location.y = y;
	b->pressed = 0;
	b->action = action;
	b->parameter = param;
	b->last_input = -1;
	b->trans_color = trans_color;
	b->image = VGA_loadBitmap(image_name);
	if(draw) VGA_drawBitmap_Transparent(b->image,x,y,trans_color);
	b->end.x = x+b->image->width;
	b->end.y = y+b->image->height;

	struct timeval curr_time;
	if ( gettimeofday( &curr_time,NULL ) >= 0 ){
		b->last_time = (curr_time.tv_usec/1000 + curr_time.tv_sec*1000)  ;
	}

	return b;
}

W_ImageButton * w_imagebutton_add(char * image_name , int x, int y, int trans_color,void (*action)(void*), void * param){
	W_ImageButton * b = _w_imagebutton(image_name,x,y,trans_color,action,param,1);

	W_List * l = (W_List*)malloc(sizeof(W_List));
	l->type = W_IMAGEBUTTON;
	l->component = b;
	l->destroy = _w_imagebutton_free;
	l->redraw = _w_imagebutton_redraw;
	l->move = _w_imagebutton_move;
	l->next = widgets;

	widgets = l;

	return b;
}

W_ImageButton * w_imagebutton_addto(W_List ** list, char * image_name , int x, int y, int trans_color,void (*action)(void*), void * param){
	W_ImageButton * b = _w_imagebutton(image_name,x,y,trans_color,action,param,0);

	W_List * l = (W_List*)malloc(sizeof(W_List));
	l->type = W_IMAGEBUTTON;
	l->component = b;
	l->destroy = _w_imagebutton_free;
	l->redraw = _w_imagebutton_redraw;
	l->move = _w_imagebutton_move;
	l->next = (*list);

	(*list) = l;

	return b;
}


/**
 * W_LABEL
 */


void _w_label_free(void * obj){
	W_Label * l = (W_Label *)obj;
	free(l->text);
	free(l);
}

void _w_label_redraw(void * obj){
	W_Label * b = (W_Label *)obj;
	VGA_setFontSize(b->font_size);
	VGA_drawText(b->text,b->location.x,b->location.y,b->color);
}

void _w_label_move(void * obj, int x, int y){
	W_Label * b = (W_Label *)obj;
	b->location.x += x;
	b->location.y += y;
}

W_Label* _w_label(char*text , int font_size , int x, int y, int color, bool draw){

	W_Label * b = (W_Label*)malloc(sizeof(W_Label));
	b->text_len = strlen(text)+1;
	b->text = (char*)malloc(b->text_len);
	memcpy(b->text,text,b->text_len);
	b->location.x = x;
	b->location.y = y;
	b->font_size = font_size;
	b->color = color;
	VGA_setFontSize(font_size);
	if(draw) VGA_drawText(text,x,y,color);

	return b;
}

W_Label* w_label_add(char*text , int font_size , int x, int y, int color){

	W_Label * b = _w_label(text,font_size,x,y,color,1);

	W_List * l = (W_List*)malloc(sizeof(W_List));
	l->type = W_LABEL;
	l->component = b;
	l->destroy = _w_label_free;
	l->redraw = _w_label_redraw;
	l->move = _w_label_move;
	l->next = widgets;

	widgets = l;

	return b;
}

W_Label*  w_label_addto(W_List** list,char*text , int font_size , int x, int y, int color){

	W_Label * b = _w_label(text,font_size,x,y,color,0);

	W_List * l = (W_List*)malloc(sizeof(W_List));
	l->type = W_LABEL;
	l->component = b;
	l->destroy = _w_label_free;
	l->redraw = _w_label_redraw;
	l->move = _w_label_move;
	l->next = (*list);

	(*list) = l;

	return b;
}

void w_label_text(W_Label * l, char * text){
	free(l->text);
	l->text_len = strlen(text)+1;
	l->text = (char*)malloc(l->text_len);
	memcpy(l->text,text,l->text_len);
}

/**
 * FREE ALL COMPONENTS
 */

void widget_free_all(){
	printf("Widget free all...");

	W_List * aux;
	while(widgets!=0){
		aux = widgets->next;

		widgets->destroy(widgets->component);
		/*switch(widgets->type){
		case(W_BUTTON):_w_button_free((W_Button*)widgets->component);break;
		case(W_IMAGEBUTTON):_w_imagebutton_free((W_ImageButton*)widgets->component);break;
		default: printf("Error: incorrect widget to free.."); break;
		}*/

		free(widgets);
		widgets = aux;
	}
}
