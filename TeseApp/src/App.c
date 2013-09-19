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



bool exitProgram;
int ymax, xmax;

static pthread_attr_t tattr_app;
static pthread_t tid_app;

bool noVehicle;




/* -----ETHERNET------------------------ */


int byteArray2int(unsigned char * array){
	return ((((int)array[3])<<(3*8)) | (((int)array[2])<<(2*8)) | (((int)array[1])<<(8)) | (array[0]));
}

double byteArray2double(unsigned char * array){
	double d;
	memcpy(&d,array,8);
	return d;
}

void RF_send();

void* receiveData(void *v){
	int sock = (int)(intptr_t)v;
	int exit = 0;
	
	if(!noVehicle){
		FB_rectfill(10, ymax - 10, 200, ymax - 40 ,FB_makecol(0,0,0,0));
		FB_printf(20, ymax - 40, FB_makecol(0,255,0,0),"Ligacao estabelecida!");
	}
	printf("Ligacao estabelecida!\n");

	unsigned char buff [100];
	Coor coor;

	while(!exit && !exitProgram){
		if(read(sock,buff,1) == 0){ exit = 1; continue;}
		if(buff[0]==0){
			printf("Ordem de despejo! \r\n");
			exit = 1;
			exitProgram = true;
		}else{
			if(read(sock,buff+1,33) == 0){ exit = 1; continue;}
			
			memcpy(&coor,buff+2,sizeof(Coor));	
			coor.calc = GEO;
			coor.origin = NULL;

			if(buff[1] == 1){
				//RoadView_update_my(byteArray2int(buff+2),byteArray2int(buff+6),byteArray2int(buff+10),byteArray2int(buff+14));
				if(!noVehicle)
					RoadView_update_myCoor(&coor);
				else
					RF_send(0,&coor);
			}else{
				//RoadView_update(buff[0],byteArray2int(buff+2),byteArray2int(buff+6),byteArray2int(buff+10),byteArray2int(buff+14));
				if(!noVehicle)
					RoadView_update_Coor(buff[0],&coor);
				else
					RF_send(buff[0],&coor);
			}
		}
	}

	close(sock);
	
	if(!noVehicle){
		FB_rectfill(10, ymax - 10, 200, ymax - 40 ,FB_makecol(0,0,0,0));
		FB_printf(20, ymax - 40, FB_makecol(255,0,0,0),"Servidor encerrado.");
	}
	printf("Servidor encerrado.\n");
	
	sleep(3);

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
		if(noVehicle==false)
			FB_printf(350+100*(i-1), ymax - 40, FB_makecol(255,255,255,0),str);
	}

	close(s);

	return 1;
}

void * startEthernetConnection(void * v){
	int port = (int)(intptr_t)v;

	exitProgram = false;
	int socksrv;
	//pthread_t t;

	printf("Espera de ligacao...\n");
	if(!noVehicle)
		FB_printf(20, ymax - 40, FB_makecol(255,255,255,0),"Espera de ligacao...    port - %d",port);

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


/* -----ETHERNET END------------------------ */












/* -----GPS-------------------------------------- */

void GPSreceive(GPSCoor * c){
	Coor * coor = Coor_new2(c->lon_deg,c->lon_min,0,c->lat_deg,c->lat_min,0,c->asimuth,c->speed);
	RoadView_update_myCoor(coor);
	Coor_destroy(coor);
}

/* -----GPS END-------------------------------------- */










/* ----FILE------------------------ */

typedef struct _CoorList{
	Coor * coor;
	struct _CoorList * next;
}CoorList;

#define TRACKS_NR 20
CoorList * tracks[TRACKS_NR];

static int my_car_id = 0; // id do meu carro lido do ficheiro, 0 no caso das coordenadas virem do gps

void moveCars(Coor * origin, bool my){
	int i;
	while(exitProgram!=true){
		if(my_car_id == 0 && origin != NULL)
			if(my) RoadView_update_myCoor(origin);
		
		for(i=0;tracks[i]!=0;i++){
			if(i+1 == my_car_id){
				if(!noVehicle && my) RoadView_update_myCoor(tracks[i]->coor);
			}else 
				if(!noVehicle)
					RoadView_update_Coor(i,tracks[i]->coor);
				else 
					RF_send(i,tracks[i]->coor);
			tracks[i] = tracks[i]->next;
		}
		sleep(1);
	}
}

void * startFileRead(void * v){
	bool my =  *((bool*)v);
	char* filename = (char*)(v+1);

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
				origin->vel = 50;
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
	
	if(!noVehicle){
		FB_rectfill(10, ymax - 10, 200, ymax - 40 ,FB_makecol(0,0,0,0));
		FB_printf(20, ymax - 40, FB_makecol(0,255,0,0),"File readed.");
	}
	printf("File readed.");

	fclose(file);

	moveCars(origin,my);
END:
	// TODO Coor_delete(tracks[i]->coor); free(tracks[i]->next); free(tracks[i]);

	if(origin != NULL) free(origin);

	return 0;
}

/* -----FILE END------------------------- */












/* -------RX----------------------------------------- */

#include "MAC_API/itdsrc_mac.h"

#define RF_INTERVAL_MS 2000 /*333*/

static pthread_attr_t RFtattr_app;
static pthread_t RFtid_app;

struct ma_dev *dev;

static uint8_t mac_addr[6];
static uint8_t mac_addr1[6];

static int transmission_power;
static int transmission_modulation;

void indication_cb(const struct ma_unitdata_indication *indication)
{
	// stop when device disconnection is detected
	if (indication->reception_status == 32) {
		printf("Indication: Device disconnected!\n");
		return;
	}

	
	if (indication->reception_status == 0) {
		if(indication->data_length < sizeof(Coor)){
			printf("Indication: Invalid data length!\n");
			return;
		}

		Coor * coor = (Coor*)(indication->data);
		
		int id = ((indication->source_address[2])<<(3*8)) |
			 ((indication->source_address[3])<<(2*8)) |
			 ((indication->source_address[4])<<(8)) |
			 (indication->source_address[5]);


		printf("Received: vehicle %d at %f,%f\n",id,coor->lat,coor->lon);
		if(!noVehicle)	
			if(id==0)
				RoadView_update_myCoor(coor);
			else
				RoadView_update_Coor(id,coor);
	}
}

void status_indication_cb(const struct ma_unitdata_status_indication *indication)
{
	//printf("Status indication: %u.\n", indication->transmission_status);
}

void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication)
{
	printf("Status indication: %u.\n", indication->transmission_status);
}

void RF_send(int id, Coor * c){
	mac_addr[5] = (uint8_t) id;
		
	uint16_t data_length = sizeof(Coor);
	char *  data;

	data = (char*) c;
		
	printf("Sending: vehicle %d at %f, %f\n",id,c->lat,c->lon);	
	
	int retval = ma_unitdatax_request(dev, mac_addr,
		mac_addr1, (uint8_t *) data, (uint16_t) data_length, 0, 0, 0,
		 (uint8_t)transmission_modulation, (uint8_t)transmission_power, 0);

	if (retval != 0) {
		printf("ma_unitdatax_request failed: %s\n", strerror(retval));
	}

}

void* startRF(void * arg){
	unsigned int tid = (((unsigned int)arg) >> 16) & 255;
	transmission_power = (((unsigned int)arg) >> 8) & 255;
	transmission_modulation = ((unsigned int)arg) & 255;
	
	printf("Start RF transmission: tid=%u, power=%u, mod=%u\n",tid,transmission_power,transmission_modulation);

	int retval;	
	
	mac_addr1[0] = 255;
	mac_addr1[1] = 255;
	mac_addr1[2] = 255;
	mac_addr1[3] = 255;
	mac_addr1[4] = 255;
	mac_addr1[5] = 255;

	mac_addr[0] = 0;
	mac_addr[1] = 0;
	mac_addr[2] = 0;
	mac_addr[3] = 0;
	mac_addr[4] = 0;
	mac_addr[5] = (uint8_t) tid;
		
		
	uint16_t data_length = sizeof(Coor);
	char * data;
	Coor* coor;

	while(exitProgram!=true && !noVehicle){
		coor = RoadView_get_myCoor();
		data = (char*) coor;
		
		printf("Sending my: vehicle %d at %f, %f\n",tid,coor->lat,coor->lon);	
	
		retval = ma_unitdatax_request(dev, mac_addr,
				mac_addr1, (uint8_t *) data, (uint16_t) data_length, 0, 0, 0, (uint8_t)transmission_modulation, (uint8_t)transmission_power, 0);


		if (retval != 0) {
			printf("ma_unitdatax_request failed: %s\n", strerror(retval));
		}
		usleep(RF_INTERVAL_MS*1000);
	}

	return 0;
}

/* -------RX END----------------------------------------- */











/* -----APP----------------------------------------*/

void wait_input(){
	bool appExit = false;
	
	int meters = 0;
	
	char c;
	char status[100];
	status[0] = '\0';

	FB_pixel cor = FB_makecol(255,255,255,0);

	bool curves = false;

	while(!appExit){
		switch(c=getchar()){
			case 'e':
				appExit = true;
				break;
			case 'u':
				curves = true;
				break;
			case '+':
				meters = RoadView_ZoomIn();
				sprintf(status,"Zoom in (range: %d m).", METERS_VISIBLE+meters);
				cor = FB_makecol(0,0,255,0);
				break;
			case '-':
				meters = RoadView_ZoomOut();
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
				if(noVehicle) break;
	
				RoadView_redraw();
				if(curves){ RoadView_drawCurves(); curves = false;}
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
    	printf("App [-p <port>] [-f <filename>] [-g /dev/<serialport>] [-i /dev/input/<inputdev>] [-t <id>] [-P <power>] [-c] [-h]\n");
    	printf("  -h : this help\n");
	printf("  -p : enable receive coordinates of the other vehicles \n"
		   "       by socket in port indicated\n");
    	printf("  -f : file with the coordinates of the other vehicles, \n"
    		"       without -g option current position is origin in file\n");
    	printf("  -g : turn on the GPS, in serial port indicated\n");
    	printf("  -t : turn on transmition the coordinates of the other\n "
    		"       vehicles by radio, must indicated an id number\n");
    	printf("  -P : transmission power (0), only used with -t command\n");
    	printf("  -M : transmission modulation (3), only used with -t command\n");
    	printf("  -n : no graphics\n");
	printf("  -i : select touch input device (default: /dev/input/event1)\n");
    	printf("  -c : calibrate touch input on start \n");
	printf("\nExamples:\n"
		"  sudo ./App -f /tracks/bairro.tck\n"
		"  sudo ./App -i /dev/input/event0 -c -g -f /tracks/bairro.tck\n"
		"  sudo ./App -p 1234\n"
		"  sudo ./App -t -p 1234\n");
}

int main (int argc, char **argv){
	int optch;
	unsigned int port = -1, tpower = -1, tmodulation = -1;
	int tid = -1;
	int no_vehicle = -1;
	bool input_cal = false;
	char filename[100];	
	char serialname[100];
	char inputdev[100] = "/dev/input/event1";
	int filer = -1, serialr = -1;

	do {
	    optch = getopt(argc, argv, "hnc:p:f:g:i:t:P:M:");

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
	            sscanf(optarg, "%s", filename+1);
	            break;
	        case 'g':
	        	serialr = 0;
	            sscanf(optarg, "%s", serialname);
	            break;
	        case 't':
	            sscanf(optarg, "%d", &tid);
	            break;
	        case 'P':
	            sscanf(optarg, "%d", &tpower);
	            break;
	        case 'M':
	            sscanf(optarg, "%d", &tmodulation);
	            break;
		case 'n':
			no_vehicle = 1;
			break;
		case 'i':
			sscanf(optarg, "%s", inputdev);
			break;
		case 'c':
			input_cal = true;
			break;
		case -1:
			break;
		default:
			print_usage();
			return -1;
			break;
	    }
	} while (optch != -1);

	if((port == -1 && filer == -1 && tid == -1) || (port != -1 && filer != -1)){
		fprintf(stderr, "Port, filename or transmission id must be indicated.\n");
		print_usage();
		return -1;
	}

	if (tmodulation == -1) tmodulation = 3;
	else if((tmodulation > 7) || (tmodulation < 0)) {
		fprintf(stderr, "Modulation must be positive and not exceed %d\n", 7);
		print_usage();
		return -1;
	}

	if (tpower == -1) tpower = 0;
	else if ((tpower > 63) || (tpower < 0)) {
		fprintf(stderr, "Power must be positive and not exceed %d\n", 63);
		print_usage();
		return -1;
	}
	
	if(no_vehicle  == -1){
		noVehicle = false;

		FB_initlib ("/dev/fb0");
		FB_change_font("font");
	
		FB_getres(&xmax,&ymax);
		printf("Resolution %dx%d\n",xmax,ymax);
		FB_clear_screen (FB_makecol (0,0,0,0));
	
		RoadView_start(input_cal,inputdev);
	}else{
		noVehicle = true;
		printf("Graphics disable\n");
	}
	
	if(port != -1){
		showInfo();
		pthread_create(&tid_app, &tattr_app, startEthernetConnection,(void*)(intptr_t)port);
		
	}else if(filer != -1){
	
		if(serialr != -1) // ler minhas coordenadas do ficheiro se nao activar o gps
			filename[0] = false;
		else
			filename[0] = true;
		pthread_create(&tid_app, &tattr_app, startFileRead,(void*)filename);
		
	} 
	
	if(tid != -1){
	
		int retval = ma_init(0, &dev, &indication_cb, &status_indication_cb, &xstatus_indication_cb);
		if (retval != 0) {
			fprintf(stderr, "ERROR: failed to initialize mac layer: %s.\n", strerror(retval));
			return -1;
		}
		//if(!noVehicle)
		pthread_create(&RFtid_app, &RFtattr_app, startRF,(void*)(tid<<16 | tpower<<8 | tmodulation));
		
	}

	if(serialr != -1)
		GPS_init(serialname,GPSreceive);

	wait_input();

	if(tid != -1) 
		ma_stop(dev);
	
	if(!noVehicle){
		RoadView_stop();

		FB_clear_screen (FB_makecol (0,0,0,0));
		FB_printf(20, 20, FB_makecol(255,255,255,0),"Terminado");
	}

	printf("Terminado!");
	getc(stdin);
	printf("\n");

	return 0;
}


/* -----APP END----------------------------------------*/


