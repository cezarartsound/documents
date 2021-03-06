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
#include <stdbool.h>
#include <pthread.h>

#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "GPS/GPS.h"

#include "libFB-0.2.4/FBlib.h"

#include "RoadView/RoadView.h"
#include "Coor.h"


typedef struct _CoorList{
	Coor * coor;
	struct _CoorList * next;
}CoorList;

#define TRACKS_NR 20
CoorList * tracks[TRACKS_NR];

bool exitProgram;
int ymax, xmax;

static pthread_attr_t tattr_app;
static pthread_t tid_app;

static int my_car_id = 0; // id do meu carro lido do ficheiro, 0 no caso das coordenadas virem do gps

int byteArray2int(unsigned char * array){
	return ((((int)array[3])<<(3*8)) | (((int)array[2])<<(2*8)) | (((int)array[1])<<(8)) | (array[0]));
}

double byteArray2double(unsigned char * array){
	double d;
	memcpy(&d,array,8);
	return d;
}

void* receiveData(void *v){
	int sock = (int)(intptr_t)v;
	int exit = 0;

	FB_rectfill(10, ymax - 10, 200, ymax - 40 ,FB_makecol(0,0,0,0));
	FB_printf(20, ymax - 40, FB_makecol(0,255,0,0),"Ligacao estabelecida!");

	unsigned char buff[50];

	while(!exit && !exitProgram){
		if(read(sock,buff,1) == 0){ exit = 1; continue;}
		if(buff[0]==0){
			printf("Ordem de despejo! \r\n");
			exit = 1;
			exitProgram = true;
		}else{
			if(read(sock,buff+1,17) == 0){ exit = 1; continue;}
			if(buff[1] == 1){
				RoadView_update_my(byteArray2int(buff+2),byteArray2int(buff+6),byteArray2int(buff+10),byteArray2int(buff+14));
			}else{
				RoadView_update(buff[0],byteArray2int(buff+2),byteArray2int(buff+6),byteArray2int(buff+10),byteArray2int(buff+14));
			}
		}
	}

	close(sock);

	FB_rectfill(10, ymax - 10, 200, ymax - 40 ,FB_makecol(0,0,0,0));
	FB_printf(20, ymax - 40, FB_makecol(255,0,0,0),"Servidor encerrado.");
	sleep(3);

	return 0;
}

void * startEthernetConnection(void * v){
	int port = (int)(intptr_t)v;

	exitProgram = false;
	int socksrv;
	//pthread_t t;

	printf("Espera de ligacao...\n");

	socksrv = tcpCreateServerSocket(NULL, port );
	if(socksrv < 0){
		fprintf(stderr,"ERRO: Nao conseguiu criar o servidor.\n");
		exitProgram = true;
	}

	if(exitProgram == false){
		printf("\nServidor ligado.\n");

		int sock;

		while(exitProgram==false){
			printf("A espera de ligacao...\n");
			sock = tcpAccpetSocket(socksrv);
			if(sock < 0){
				fprintf(stderr,"	ERRO: Nao conseguiu estabelecer uma ligacao.\n");
				continue;
			}

			receiveData((void*)(intptr_t)sock);
			close(sock);
		}
	}
	close(socksrv);

	printf("Servidor desligado.\n");
	
	return 0;
}

void GPSreceive(GPSCoor * c){
	Coor * coor = Coor_new2(c->lon_deg,c->lon_min,0,c->lat_deg,c->lat_min,0,c->asimuth,c->speed);
	RoadView_update_myCoor(coor);
	Coor_destroy(coor);
}

void moveCars(Coor * origin){
	int i;
	while(exitProgram!=true){
		if(my_car_id == 0 && origin != NULL)
			RoadView_update_myCoor(origin);
		
		for(i=0;tracks[i]!=0;i++){
			if(i+1 == my_car_id)
				RoadView_update_myCoor(tracks[i]->coor);
			else
				RoadView_updateCoor(i,tracks[i]->coor);
			tracks[i] = tracks[i]->next;
		}
		sleep(1);
	}
}

void * startFileRead(void * v){
	char* filename = (char*)v;

	FILE * file = fopen(filename,"r");
	if(file == NULL){
		fprintf(stderr,"Error opening file %s. (%d)\n",filename,errno);
		goto END;
	}

	size_t len = 0;
	char * line = NULL;
	Coor * origin = NULL;
	CoorList * clist;
	int i,j;

	while(getline(&line,&len,file)> 0){
		switch(line[0]){
			case 'i':
				if(getline(&line,&len,file) < 0){fprintf(stderr,"Error loading image name from file %s.\n", filename); goto END;}
				// TODO background image
			   break;

			case 'z':
				if(getline(&line,&len,file) < 0){fprintf(stderr,"Error loading zoom value from file %s.\n", filename); goto END;}
			   // TODO zoom
			   //1 zoom = RULERS_ADJUST pixels = 1 metro
			   break;

			case 'o':
				if(getline(&line,&len,file) < 0){fprintf(stderr,"Error loading origin from file %s.\n", filename); goto END;}
				origin = Coor_new0();
				Coor_parce_str(line,origin);
				//RoadView_update_my(0,0,0,origin->asimuth);
				RoadView_update_myCoor(origin);
				break;

			case 'v':
				i = atoi(line+1)-1;
				if(getline(&line,&len,file) < 0){fprintf(stderr,"Error loading tracks from file %s.\n", filename); goto END;}
				j = atoi(line);

				tracks[i] = (CoorList*)malloc(sizeof(CoorList));
				clist = tracks[i];
				while(j--){
					if(getline(&line,&len,file) < 0){fprintf(stderr,"Error loading origin from file %s.\n", filename); goto END;}
					clist->coor = Coor_new0();
					Coor_parce_str(line,clist->coor);
					//Coor_parce_wO(line,clist->coor,origin);
					if(j==0)
						clist->next = tracks[i];
					else{
						clist->next = (CoorList*)malloc(sizeof(CoorList));
						clist = clist->next;
					}
				}

				break;
		}
	}

	FB_rectfill(10, ymax - 10, 200, ymax - 40 ,FB_makecol(0,0,0,0));
	FB_printf(20, ymax - 40, FB_makecol(0,255,0,0),"File %s readed.",filename);

	fclose(file);

	moveCars(origin);
END:
	// TODO Coor_delete(tracks[i]->coor); free(tracks[i]->next); free(tracks[i]);

	if(origin != NULL) free(origin);

	free(filename);
	return 0;
}

int showInfo(){
	char str[100];
	int s;
	struct ifconf ifconf;
	struct ifreq ifr[50];
	int ifs;
	int i;

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		perror("socket");
		return 0;
	}

	ifconf.ifc_buf = (char *) ifr;
	ifconf.ifc_len = sizeof ifr;

	if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
		perror("ioctl");
		return 0;
	}

	ifs = ifconf.ifc_len / sizeof(ifr[0]);
	//printf("interfaces = %d:\n", ifs);
	for (i = 1; i < ifs; i++) {
		char ip[INET_ADDRSTRLEN];
		struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;

		if (!inet_ntop(AF_INET, &s_in->sin_addr, ip, sizeof(ip))) {
			perror("inet_ntop");
			return 0;
		}

		sprintf(str,"%s - %s", ifr[i].ifr_name, ip);
		printf("%s\n",str);
		FB_printf(350+100*(i-1), ymax - 40, FB_makecol(255,255,255,0),str);
	}

	close(s);

	return 1;
}

void wait_input(){
	bool appExit = false;
	
	int meters = 0;
	
	char c;
	char status[100];
	status[0] = '\0';

	FB_pixel cor = FB_makecol(255,255,255,0);

	while(!appExit){
		switch(c=getchar()){
			case 'e':
				appExit = true;
				break;
			case '+':
				meters -= 20;
				RoadView_add_meters_visible(meters);
				sprintf(status,"Zoom in (range: %d m).", METERS_VISIBLE+meters);
				cor = FB_makecol(0,0,255,0);
				break;
			case '-':
				meters += 20;
				RoadView_add_meters_visible(meters);
				sprintf(status,"Zoom out (range: %d m).", METERS_VISIBLE+meters);
				cor = FB_makecol(0,0,255,0);
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				my_car_id = c-'0';
				sprintf(status,"My car is the number %d.", my_car_id);
				cor = FB_makecol(0,0,255,0);
				break;
			case '\n':	
				RoadView_redraw();
				FB_printf(20, ymax - 40, cor, status);
				break;
			default:
				strcpy(status,"Comando invalido.");
				cor = FB_makecol(255,0,0,0);
				break;
		}
	}
}

void print_usage(void)
{
    printf("App [-p <port>] [-f <filename>] [-g /dev/<serialport>] [-t] [-h]\n");
    printf("  -h : this help\n");
	printf("  -p : enable receive coordinates of the other vehicles \n"
		   "       by socket in port indicated\n");
    printf("  -f : file with the coordinates of the other vehicles, \n"
    		"       without -g option current position is origin in file\n");
    printf("  -g : turn on the GPS, in serial port indicated\n");
    printf("  -t : turn on transmition the coordinates of the other\n "
    		"       vehicles by radio\n");
    printf("  -n : no graphics\n");
}

int main (int argc, char **argv){
/*	
	FB_initlib ("/dev/fb0");
	FB_rectfill(20,20,200,200,FB_makecol(255,0,0,0));
	getchar();
	return 0;
*/
	int optch;
	int port = -1;
	int no_graph = -1;
	char * filename = (char*)malloc(100);
	char * serialname = (char*)malloc(100);
	int filer = -1, serialr = -1;

	do {
	    optch = getopt(argc, argv, "hnt:p:f:g:");

	    switch (optch) {
	        case 'h':
	            print_usage();
	            return 0;
	            break;
	        case 'p':
	            sscanf(optarg, "%d", &port);
	            break;
	        case 'f':
	        	filer = 0;
	            sscanf(optarg, "%s", filename);
	            break;
	        case 'g':
	        	serialr = 0;
	            sscanf(optarg, "%s", serialname);
	            break;
	        case 't':
	        	printf("-t option not yet implemented."); //TODO
	            break;
		case 'n':
	        	no_graph = 1;
			break;
	        case -1:
	            break;
	        default:
	            print_usage();
	            return -1;
	            break;
	    }
	} while (optch != -1);

	if((port == -1 && filer == -1) || (port != -1 && filer != -1)){
		fprintf(stderr, "Port or filename must be indicated.\n");
		print_usage();
		return -1;
	}

	if(no_graph  == -1){
		FB_initlib ("/dev/fb0");
		FB_change_font("font");
	
		FB_getres(&xmax,&ymax);
		printf("Resolution %dx%d\n",xmax,ymax);
		FB_clear_screen (FB_makecol (0,0,0,0));
	}else
		printf("Graphics disable\n");
		
	RoadView_start();
	
	if(port != -1){
		showInfo();
		FB_printf(20, ymax - 40, FB_makecol(255,255,255,0),"Espera de ligacao...    port - %d",port);
		pthread_create(&tid_app, &tattr_app, startEthernetConnection,(void*)(intptr_t)port);
		//startEthernetConnection(0);
	}else if(filer != -1){
		FB_printf(20, ymax - 40, FB_makecol(255,255,255,0),"Lendo o ficheiro %s ...",filename);
		pthread_create(&tid_app, &tattr_app, startFileRead,(void*)filename);
	}

	if(serialr != -1)
		GPS_init(serialname,GPSreceive);

	wait_input();

	free(filename);
	free(serialname);

	RoadView_stop();

	FB_clear_screen (FB_makecol (0,0,0,0));
	FB_printf(20, 20, FB_makecol(255,255,255,0),"Terminado");

	printf("Terminado!");
	getc(stdin);
	printf("\n");

	return 0;
}

