/*
 * vgatest.c
 *
 *  Created on: 20 de Jul de 2012
 *      Author: fabio32883
 */

#include "vga/vga.h"
#include "vga/ezdib/ezdib.h"
#include "event/event.h"
#include "input/input.h"
#include "widget/widget.h"
#include "window/window.h"
#include <stdio.h>
#include <sys/time.h>


char str[2] = "1";

void action_up(void*v){
	Window * w = (Window*)v;
	W_Label * label = (W_Label*)w->widgets->component;
	str[0]++;
	w_label_text(label,str);
	window_redraw(w);
}

void action_down(void*v){
	Window * w = (Window*)v;
	W_Label * label = (W_Label*)w->widgets->component;
	str[0]--;
	w_label_text(label,str);
	window_redraw(w);
}

int mov;

void action_move(void*v){
	Window * w = (Window*)v;
	window_move(w,mov?-50:50,0);
	mov = !mov;
	windows_update();
}

void create_test_window(int x, int y){
	Window * w = window_new(x,y,250,250,"Janela test");
	window_add_button(w,"  Up  ",18,10,10,action_up,w);
	window_add_button(w," Down ",18,10,50,action_down,w);
	window_add_button(w," Move ",18,10,90,action_move,w);
	window_add_label(w,"1",24,20,200,COLOR_BLUE);
	window_activate(w);
}

int main()
{

	widget_initall();

	windows_update();

	create_test_window(10,60);
	create_test_window(300,100);

	while(1);

	printf("END\r\n");
	return 0;

}
