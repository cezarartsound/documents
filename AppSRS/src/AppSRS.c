/*
 * IamHere.c
 *
 *  Created on: 12 de Jul de 2012
 *      Author: fabio32883
 */

#define _GNU_SOURCE 1
#define _THREAD_SAFE 1


#include "so_tcplib/so_tcplib.h"




#include <stdio.h>
#include <unistd.h> /* exit, write, read, ... */
#include <stdlib.h> /* atoi, ... */
#include <string.h>
#include <pthread.h>

#include "GraphicInterface/vga/vga.h"
#include "GraphicInterface/window/window.h"


#ifndef _BOOL_
#define _BOOL_
typedef enum bool {TRUE, FALSE} bool;
#endif /* _BOOL_ */

int x = 50, y = 60;

void action_close(void * v){
	Window *w = (Window*)v;
	window_free(w);
	windows_update();

	x -= 30;
	y -= 30;
}

void action_timeout(void * v){
	Window *w = (Window*)v;
	window_free(w);
	windows_update();

	x = 50;
	y = 60;
}


void create_window(char* title, char* value){
	x += 30;
	y += 30;
	/*switch((window_nr++)%6){
	case 0: x = 50; y = 60; break;
	case 1: x = 340; y = 60; break;
	case 2: x = 50; y = 170; break;
	case 3: x = 340; y = 170; break;
	case 4: x = 50; y = 280; break;
	case 5: x = 340; y = 280; break;
	default: x = 50; y = 60; break;
	}*/

	Window * w = window_new(x,y,270,97,title);
	window_add_label(w,value,24,40,40,COLOR_DARK_BLUE);
	//window_add_button(w," Close ",18,10,30,action_close,w);
	window_activate(w);

	event_once_add(action_timeout,w, 5000);
}


int port;
bool exitProgram;

void* appCicle(void *v){
	int sock = (int)v;
	int i, exit = 0;
	char string [15];
	int size;

	create_window("Mensagem","Ligacao estabelecida!");
	printf("Ligacao estabelecida \r\n");

	char buff;
	do{ // testar a ligacao
		read(sock,&buff,1);
		//printf("%d\r\n",buff);
		buff++;
		write(sock,&buff,1);
		read(sock,&buff,1);
	}while(buff!='s');



	while(!exit){
		if(read(sock,&size,1)<=0)break;
		printf("size: %d \r\n",size);
		if(size>15)size=15;
		if(read(sock,string,size)<=0)break;
		string[14] = 0;
		create_window("Velocidade",string);
		printf(string);
	}

	close(sock);

	VGA_fill(COLOR_BLACK);
	VGA_drawText("Servidor encerrado.", 20, 100, COLOR_RED);
	exitProgram = TRUE;
	sleep(3);


	return 0;
}

void waitConnection(){
	exitProgram = FALSE;
	int socksrv;
	pthread_t t;

	socksrv = tcpCreateServerSocket(NULL, port );
	if(socksrv < 0){
		fprintf(stderr,"ERRO: Nao conseguiu criar o servidor.\n");
		create_window("Mensagem","ERRO!");
		exitProgram = TRUE;
	}

	if(exitProgram == FALSE){
		printf("\nServidor ligado.\n");

		int sock, i=0;

		printf("###waitConnection\n");

		while(exitProgram==FALSE){
			sock = tcpAccpetSocket(socksrv);
			if(sock < 0){
				fprintf(stderr,"	ERRO: Nao conseguiu estabelecer uma ligacao.\n");
				create_window("Mensagem","ERRO!");
				continue;
			}

			appCicle((void*)sock);
		}
	}

	close(socksrv);

	printf("Servidor desligado.\n");
}



int main(int argc, char ** argv){
	if(argc < 2){
		fprintf(stderr,"ERRO: Tem de introduzir o numero do porto.\n");
		create_window("Mensagem","ERRO!");
		exitProgram = TRUE;
	}

	port = atoi(argv[1]);

	widget_initall();

	windows_update();

	create_window("Mensagem","Espera de ligacao...");

	VGA_drawImage("back.bmp",1,1);

	waitConnection();

	return 0;
}
