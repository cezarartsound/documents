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
#include <stdio.h>
#include <sys/time.h>

int yy = 30;
int yy1 = 30;

void action(void*v){
	VGA_drawText("toma",20,yy,COLOR_BLACK);
	yy+=30;
}

void action1(void*v){
	VGA_drawText("la",90,yy1,COLOR_BLACK);
	yy1+=30;
}

long int last_time = 0; //ms
int b = 1;
void blink(void * v){
	struct timeval curr_time;
	if ( gettimeofday( &curr_time,NULL ) >= 0 ){
		if((curr_time.tv_usec/1000 + curr_time.tv_sec*1000) - last_time>250){
			if(b)
				VGA_drawText("CUIDADO",300,100,COLOR_RED);
			else
				VGA_fill_rect(300,80,420,100,COLOR_WHITE);
			b = !b;
			last_time = (curr_time.tv_usec/1000 + curr_time.tv_sec*1000);
		}
	}
}

int main()
{

	widget_initall();
	VGA_fill(COLOR_WHITE);
	w_button_add("  Iniciar  ",40,120,300,action,0);
	widget_set_backgroundColor(COLOR_WHITE);
	w_imagebutton_add("isel.bmp",400,250,action1,0);

	event_add(blink,0);
	while(1);

	printf("END\r\n");
	return 0;

}
