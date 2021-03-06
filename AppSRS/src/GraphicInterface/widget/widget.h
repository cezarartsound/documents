/*
 * widget.h
 *
 *  Created on: 14 de Nov de 2012
 *      Author: fabio32883
 */

#ifndef WIDGET_H_
#define WIDGET_H_

#include "../input/input.h"
#include "../event/event.h"
#include "../vga/vga.h"

typedef enum _W_Type { W_BUTTON , W_IMAGEBUTTON, W_LABEL} W_Type;

typedef struct _W_list{
	W_Type type;
	void * component;
	void (*destroy)(void*);
	void (*redraw)(void*);
	void (*move)(void*, int x, int y);
	struct _W_list * next;
}W_List;


#define W_BUTTON_PRESS_MS 250 // tempo que o butao fica premido
typedef struct _W_Button{
	Point location;
	Point end;
	int text_len;
	char* text;
	int font_size;
	char pressed;
	Event * event_press;
	Event * event_release;
	int last_time; //miliseconds
	int last_input;
	void * parameter;
	void (*action)(void*);
}W_Button;

typedef struct _W_ImageButton{
	Point location;
	Point end;
	BITMAP * image;
	int trans_color;
	char pressed;
	Event * event_press;
	Event * event_release;
	int last_time;
	int last_input;
	void * parameter;
	void (*action)(void*);
}W_ImageButton;

typedef struct _W_Label{
	Point location;
	int text_len;
	char* text;
	int font_size;
	int color;
}W_Label;

void widget_init();
void widget_initall();// inicia o vga input e event
void widget_exit();
void widget_exitall();

void widget_free_all();
void w_clean_screen();

W_Button* w_button_add(char*text , int font_size , int x, int y,void (*action)(void*), void * param);
W_Button* w_button_addto(W_List** list, char*text , int font_size , int x, int y,void (*action)(void*), void * param); // nao desenha

W_Label* w_label_add(char*text , int font_size , int x, int y,int color);
W_Label* w_label_addto(W_List** list, char*text , int font_size , int x, int y,int color); // nao desenha
void w_label_text(W_Label * l, char * text);

W_ImageButton* w_imagebutton_add(char * image_name , int x, int y, int trans_color,void (*action)(void*), void * param); // -1 to trans_color be first pixel
W_ImageButton* w_imagebutton_addto(W_List** list,char * image_name , int x, int y, int trans_color,void (*action)(void*), void * param); // nao desenha




#endif /* WIDGET_H_ */
