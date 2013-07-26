/*
 * vgatest.c
 *
 *  Created on: 20 de Jul de 2012
 *      Author: fabio32883
 */

#include "vga/vga.h"
#include "vga/ezdib/ezdib.h"
#include "event/event.h"
#include <stdio.h>
#include <sys/time.h>


int main()
{

	VGA_init();

	input_init();

	input_calibration();

	//input_print_values(10);

	VGA_fill(VGA_get16bpp_color(90,90,100) );

	VGA_rect(10,10,VGA_vinfo.xres-10, VGA_vinfo.yres-10,VGA_get16bpp_color(100,100,100));


	VGA_setFontSize(24);
	VGA_drawButton("   Iniciar   ",200,320);

	Point p;
	while(1){
		input_getClickLock(&p);
		printf("(x,y) = (%d,%d)\r\n", p.x , p.y );
		if(p.x > 200 && p.x < 400 && p.y > 320 && p.y <470){
			VGA_drawButtonClick("   Iniciar   ",200,320);
		}else{
			VGA_drawButton("   Iniciar   ",200,320);
		}
	}


/* Forma nao bloqueante

	struct timeval curr_time;
	gettimeofday( &curr_time,NULL );
	int time = curr_time.tv_usec;

	while(1){
		if(input_getClick(&p)){
			//printf("(%d,%d)\r\n",p.x,p.y);
			if(p.x > 200 && p.x < 400 && p.y > 320 && p.y <470){
				VGA_drawButtonClick("   Iniciar   ",200,320);
				gettimeofday( &curr_time,NULL );
				time = curr_time.tv_usec;
			}
		}else{
			gettimeofday( &curr_time,NULL );
			if(curr_time.tv_usec-time>=100000){
				time = curr_time.tv_usec;
				VGA_drawButton("   Iniciar   ",200,320);
			}
		}
	}
*/


	printf("END\r\n");
	VGA_exit();
	return 0;

}
