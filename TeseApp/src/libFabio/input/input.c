#define GNU_SOURCE  

#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h> //malloc
#include <pthread.h>
#include <unistd.h> //sleep
#include "input.h"
#include "../vga.h"

static pthread_mutex_t* input_mutex;
static pthread_attr_t tattr_input;
static pthread_t tid_input;

#define INPUTS_LEN 10
static Point inputs[INPUTS_LEN];
static int inputs_idx;
static int input_x, input_y;

static float cal_mx, cal_bx, cal_my, cal_by;

static unsigned char inputstop;

static bool input_initialized = false;

void * input_routine(void * v){
	FILE * input_fd = (FILE *)v;
	
	int i;

	pthread_mutex_lock(input_mutex);
	input_x = input_y = -1;
	pthread_mutex_unlock(input_mutex);
	
	struct input_event event;
	while(!inputstop){
		if((fread(&event,sizeof(struct input_event),1,input_fd))>0){
			if(event.type == EV_ABS){
				if(event.code == ABS_Y)// ABS_RX)
					input_y = event.value;
				if(event.code == ABS_X)//ABS_Z)
					input_x = event.value;
				if(input_x!=-1 && input_y !=-1){
					pthread_mutex_lock(input_mutex);
					i = inputs_idx%INPUTS_LEN;
					inputs[i].x = input_x;
					inputs[i].y = input_y;
					input_x = input_y = -1;
					inputs_idx ++;
					pthread_mutex_unlock(input_mutex);
				}
			}
		}else{ break;}
	}
	printf("Input thread finished!\n");
	return 0;
}

void input_init(char * input_dev){
	FILE * input_fd = fopen(input_dev,"r");

	if(input_fd>0)
		printf("Input on\r\n");
	else{
		printf("Input open error\r\n");
		return;
	}

	cal_mx = -0.221836;
	cal_my = 0.103645;
	cal_bx = 843.36;
	cal_by = 13.76;

	input_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(input_mutex, NULL);

	inputstop = 0;

	pthread_create(&tid_input, &tattr_input, input_routine,(void*)input_fd);
	printf("\nThread input created %d\n",(int)tid_input);
	input_initialized = true;
}

void input_stop(){
	input_initialized = false;
	inputstop = 1;
	free(input_mutex);
	//TODO free all
}

void input_flush(){
	if(input_initialized==false) return;

	pthread_mutex_lock(input_mutex);
	inputs_idx = 0;
	input_x = input_y = -1;
	pthread_mutex_unlock(input_mutex);
	//pthread_yield(); // TODO: warnning i dont know why
}

void input_get_next_point(Point * point){
	if(input_initialized==false) return;

	pthread_mutex_lock(input_mutex);
	int i = inputs_idx;
	input_x = input_y = -1;
	pthread_mutex_unlock(input_mutex);
	//pthread_yield(); // TODO: warnning i dont know why

	bool exit = 1;
	while(exit){
		pthread_mutex_lock(input_mutex);
		if(inputs_idx != i){
			i = (inputs_idx-1)%INPUTS_LEN;
			point->x = inputs[i].x;
			point->y = inputs[i].y;
			exit = 0;
		}
		pthread_mutex_unlock(input_mutex);
		usleep(50000);
		//pthread_yield(); // TODO: warnning i dont know why
	}
}

void input_calibration(){
	if(input_initialized==false) return;

	Point p1, p2;


	VGA_fill(COLOR_WHITE);

	//VGA_setFontSize(18);





	VGA_rect(40,60,50,70,COLOR_RED);
	VGA_set_pixel(45,65,COLOR_RED);

	VGA_drawText("Clique aqui!", 65,70,COLOR_RED);

	input_get_next_point(&p1);

	printf("(%d,%d)\r\n",p1.x,p1.y);




	VGA_fill(COLOR_WHITE);

	VGA_drawText("Guardando os dados...", 150,220,COLOR_RED);
	sleep(1);


	VGA_fill(COLOR_WHITE);




	VGA_rect(550,350,560,360,COLOR_RED);
	VGA_set_pixel(555,355,COLOR_RED);

	VGA_drawText("Clique aqui!", 400,360,COLOR_RED);

	input_get_next_point(&p2);
	printf("(%d,%d)\r\n",p2.x,p2.y);


	VGA_fill(COLOR_WHITE);



	cal_mx = ((float)(555-45))/((float)(p2.x-p1.x));
	cal_bx = 45-cal_mx*p1.x;
	cal_my = ((float)(355-65))/((float)(p2.y-p1.y));
	cal_by = 65-cal_my*p1.y;

	printf("Calibrations values: %f  |  %f  |  %f  |  %f\r\n", cal_mx, cal_bx, cal_my, cal_by);

	VGA_drawText("Calibracao concluida!", 150,220,COLOR_RED);
	sleep(1);

//	VGA_fill(COLOR_WHITE);
}

void input_getClickLock(Point * p){
	if(input_initialized==false) return;

	input_get_next_point(p);
	p->x = (int)(p->x*cal_mx+cal_bx);
	p->y = (int)(p->y*cal_my+cal_by);
}

int input_getClick(Point * p){
	if(input_initialized==false) return -1;

	int i;
	pthread_mutex_lock(input_mutex);

	i = (inputs_idx-1)%INPUTS_LEN;
	if(p!=0) {
		p->x = (int)(inputs[i].x*cal_mx+cal_bx);
		p->y = (int)(inputs[i].y*cal_my+cal_by);
	}

	pthread_mutex_unlock(input_mutex);

	return i;
}

void input_print_values(int nr_values){
	if(input_initialized==false) return;

	Point p;

	while(nr_values--){
		input_get_next_point(&p);
		printf("(%d,%d)\r\n",(int)(p.x*cal_mx+cal_bx),(int)(p.y*cal_my+cal_by));
		sleep(1);
	}
}
