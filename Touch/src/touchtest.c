/*
 * touchtest.c
 *
 *  Created on: 21 de Set de 2012
 *      Author: fabio32883
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>


int main(){
	struct input_event event;
	FILE * fd = fopen("/dev/input/event2","r");
	char c;

	printf("Boraa!\r\n");

	if(fd>0){
		printf("Abrido :)\r\n");
		while(1){
			if((fread(&event,sizeof(struct input_event),1,fd))>0)
				printf("%x - %x - %d\r\n",event.type,event.code,event.value);
/*	
			if((fread(&c,1,1,fd))>0)
				printf("%d\r\n",c);
*/		
		}
	}else
		printf("Nao abriu :/\r\n");

	printf("Ja esta!\r\n");
	return 0;
}
