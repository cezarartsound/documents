/*
 * Network_Layer.c
 *
 *  Created on: 20 de Jan de 2012
 *      Author: fabio32883
 */

#include "SerialPort/SerialPort.h"
#include <stdbool.h> // bool
#include <stdio.h>	// printf
#include <pthread.h> // threads
#include <stdlib.h> // return macros
#include <unistd.h> // optarg
#include <string.h> //memcpy

#include "GPS_motorola.h"


static pthread_attr_t tattr_rx;
static pthread_t tid_rx;

static Serialport * serialport;

static bool EXIT;


char bin[8] = "00000000";
char * byte2bin(char n){
	int i;
	for(i=0;i<8;i++)
		if((n>>i)&0x1)
			bin[7-i]='1';
		else
			bin[7-i]='0';
	return bin;
}
void * rx_routine(void * arg){
	unsigned char data[1000];
	int i = 0;

	SHORT_POSITION_MESSAGE * short_position_msg;

	while(EXIT == false){
		if(Serialport_ReadByte(serialport,SERIAL_TIMEOUT)!='@')continue;
		if(Serialport_ReadByte(serialport,SERIAL_TIMEOUT)!='@')continue;

		i=0;
		while(1){
			if((data[i++]=Serialport_ReadByte(serialport,SERIAL_TIMEOUT))==255) break;
			if((i>2 && data[i-1]=='\n' && data[i-2]=='\r') || i>=1000) break;
		}

		if(data[i-1]==255){
			printf("Read command error!\r\n");
			continue; // capturar erro, por timeout ou outro
		}

		int size = i;

		/*printf("size: %d\r\n",size);
		for(i=0;i<size;i++)
			printf("(%X)\n",data[i]);*/

		/* CONFIRMAÇÃO DO CHECKSUM (falhava 50% das tramas) */
	/*	i=1;
		checksum = data[0];
		while(i<size-3) // calcular o checksum
			checksum =  checksum ^ data[i++];

		if(checksum != data[size-3]){ // detectar erro de checksum
			printf("Checksum failed!\r\n");
			continue;
		}*/

		printf("Command received: @@%c%c\r\n",data[0],data[1]);

		switch(data[0]){
		case 'C':
			switch(data[1]){
			case 'f':
				printf("Set to default successful!\r\n");
				break;

			default:
				printf("Command not implemented.\r\n");
				break;
			}
			break;
		case 'I':
			switch(data[1]){
			case 'a':
				printf("Result of seft test: %s %s %s\r\n",byte2bin(data[2]),byte2bin(data[3]),byte2bin(data[4]));
				break;

			default:
				printf("Command not implemented.\r\n");
				break;
			}
			break;
		case 'H':
			switch(data[1]){
			case 'b':
				short_position_msg = (SHORT_POSITION_MESSAGE*)malloc(sizeof(SHORT_POSITION_MESSAGE));
				memcpy(short_position_msg,data+2,47);

				//printf("%f , %f , %f \r\n",short_position_msg->lat_d,short_position_msg->lat_d*90.0,(short_position_msg->lat_d*90.0)/90.0);

				short_position_msg->lat = ((double)short_position_msg->lat_d)*90.0/324000000;
				short_position_msg->lon = ((double)short_position_msg->lon_d)*180.0/648000000;
				short_position_msg->GPS_height = short_position_msg->GPS_height_d/100.0;
				short_position_msg->MLS_height = short_position_msg->MLS_height_d/100.0;
				short_position_msg->speed_3D = short_position_msg->speed_3D_d/100.0;
				short_position_msg->speed_2D = short_position_msg->speed_2D_d/100.0;
				short_position_msg->heading = short_position_msg->heading_d/10.0;
				short_position_msg->DOP = short_position_msg->DOP_d/10.0;
				short_position_msg->converted = true;

				double z = 0;
				for(i=0;i<size-3;i++)if(data[i]==0)z++; // contar os dados a zero
				/*if(size/z > 0.9)
					printf("SHORT POSITION MESSAGE: zeros\r\n");
				else*/{
					printf("SHORT POSITION MESSAGE\n");
					printf("	Date: 			%02i/%02i/%02i\n",short_position_msg->day, short_position_msg->month, short_position_msg->year);
					printf("	Hour: 			%02i:%02i:%02i (%04i)\n",short_position_msg->hour, short_position_msg->minutes, short_position_msg->seconds, short_position_msg->fraq_seconds);
					printf("	Latitude: 		%g°  (%d)\n",short_position_msg->lat,short_position_msg->lat_d);
					printf("	Longitude: 		%g°  (%d)\n",short_position_msg->lon,short_position_msg->lon_d);
					printf("	GPS height: 		%g m\n",short_position_msg->GPS_height);
					printf("	MLS height: 		%g m\n",short_position_msg->MLS_height);
					printf("	3D speed: 		%g m/s\n",short_position_msg->speed_3D);
					printf("	2D speed: 		%g m/s\n",short_position_msg->speed_2D);
					printf("	2D heading: 		%g°\n",short_position_msg->heading);
					printf("	Visible satellite: 	%d\n",short_position_msg->visible_satellite);
					printf("	Tracked satellite: 	%d\n",short_position_msg->tracked_satellite);
					printf("	Status: 		0x%x\n",short_position_msg->receiver_status);
				}
				break;
			default:
				printf("Command not implemented.\r\n");
				break;
			}
			break;

		default:
			printf("Command not implemented.\r\n");
			break;
		}

		usleep(10000) ;
	}

	Serialport_Close(serialport);
	Serialport_Destroy(serialport);
	return 0;
}

void GPS_init(char * serial_name){
	EXIT = false;

	serialport = Serialport_Create();

	Serialport_Open(serialport,serial_name,BAUD_9600, CHAR_SIZE_DEFAULT,PARITY_DEFAULT, STOP_BITS_DEFAULT,FLOW_CONTROL_DEFAULT);

	pthread_create(&tid_rx, &tattr_rx, rx_routine,NULL);
	printf("\nThread LLC-rx created %d\n",(int)tid_rx);
}

void GPS_sendCommand(char * data, int size){
	int i;

	char checksum = data[0];

	for(i=1;i<size;i++)
		checksum = checksum ^ data[i];

	char * trm = (char*)malloc(size+5);
	trm[0]='@';
	trm[1]='@';
	memcpy(trm+2,data,size);
	trm[2+size]=checksum;
	trm[3+size]='\r';
	trm[4+size]='\n';

	Serialport_Write(serialport,trm,size+5);

	free(trm);
}


void print_commands(){
	printf("Commands avaiable:\r\n");
	printf("	e - exit\r\n");
	printf("	h - help\r\n");
	printf("	l - enable/disable serial port log\r\n");
	printf("	d - set GPS to default configuration\r\n");
	printf("	g - get GPS data once time\r\n");
	printf("	G - get GPS data every 5 seconds\r\n");
	printf("	t - GPS self test command\r\n");
	printf("	s - Switch to NMEA (4800)\r\n");
}

void GPS_test(){
	print_commands();
	while(EXIT == false){
		switch(getchar()){
		case 'e':
			printf("Exit.\r\n");
			EXIT = true;
			break;
		case 'd':
			printf("Set to default configuration...\r\n");
			GPS_sendCommand("Cf",2);
			break;
		case 'l':
			if(serial_log)
				printf("Disable serial log.\r\n");
			else
				printf("Enable serial log.\r\n");
			serial_log = !serial_log;
			break;
		case 'g':
			printf("Get data...\r\n");
			GPS_sendCommand("Hb\x00",3);
			break;
		case 'G':
			printf("Get data...\r\n");
			GPS_sendCommand("Hb\x05",3);
			break;
		case 't':
			printf("Seft test...\r\n");
			GPS_sendCommand("Ia",2);
			break;
		case 's':
			printf("Switch to NMEA...\r\n");
			GPS_sendCommand("Ci\x01",3);
			break;
		case 'h':
			print_commands();
			break;
		}
	}
}










void print_usage(){
	printf("GPS -n <serial_name> [-h] \n");
	printf("  -h : this help\n");
	printf("  -n : serial port name\n");
}

int main(int argc, char **argv){

	char optch;

	char serial_name[50];
	serial_name[0] = -1;


	do {
		optch = getopt(argc, argv, "hn:");

		switch (optch) {
			case 'h':
				print_usage();
				return EXIT_SUCCESS;
				break;

			case 'n':
				sscanf(optarg, "%s", serial_name);
				break;

			case 255:
				break;

			default:
				print_usage();
				return EXIT_FAILURE;
				break;
		}
	} while (optch != 255);

	if(serial_name[0]<0){
		fprintf(stderr, "Serial_name must be defined\n");
		print_usage();
		return EXIT_FAILURE;
	}

	printf("START!\r\n");

	GPS_init(serial_name);

	GPS_test();

	printf("END!\n\r");

	return 0;
}

