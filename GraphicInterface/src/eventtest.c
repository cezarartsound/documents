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
#include <stdio.h>
#include <sys/time.h>


int button_press_last;
void button_press(void * v){
	Point p;
	int l;
	if(button_press_last != (l=input_getClick(&p))){
		if(p.x > 200 && p.x < 400 && p.y > 320 && p.y <470){
			VGA_drawButtonClick("   Iniciar   ",200,320);
		}else{
			VGA_drawButton("   Iniciar   ",200,320);
		}
		button_press_last = l;
	}
}

int main()
{

	VGA_init();

	input_init();

	//input_calibration();

	//input_print_values(10);

	VGA_fill(VGA_get16bpp_color(90,90,100) );

	VGA_rect(10,10,VGA_vinfo.xres-10, VGA_vinfo.yres-10,VGA_get16bpp_color(100,100,100));


	VGA_setFontSize(24);
	VGA_drawButton("   Iniciar   ",200,320);


	event_init();
	event_add(button_press,0,0);


	while(1);

	printf("END\r\n");
	VGA_exit();
	return 0;

}
