/*
 * Network_Layer.c
 *
 *  Created on: 20 de Jan de 2012
 *      Author: fabio32883
 */

#include "SerialPort/SerialPort.h"
#include <sys/types.h>
#include <stdbool.h> // bool
#include <stdio.h>	// printf
#include <pthread.h> // threads
#include <stdlib.h> // return macros
#include <unistd.h> // optarg
#include <string.h> //memcpy
#include <time.h>

#include "GPS.h"


static pthread_attr_t tattr_gps_rx;
static pthread_t tid_gps_rx;

static Serialport * serialport;

static bool exit_gps;

static bool csv_save;
static FILE *csv_file;
static bool tck_save;
static FILE *tck_file;

static void (*externFunc)(GPSCoor*);

void GPS_GPGGA(uint8_t * data){

	/*
	 * $GPGGA,hhmmss.ss,ddmm.mmmm,n,dddmm.mmmm,e,q,ss,y.y,a.a,z,g.g,z,t.t,iiii*CC<CR><LF>
	 * */

	GPSCoor coor;
	int i;

	memset(&coor,0,sizeof(GPSCoor));

	if(data[41]=='0') // Verify gps status
		coor.valid = false;
	else
		coor.valid = true;

	i=5;
	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Hour
		coor.hour = (data[i]-'0')*10 + (data[i+1]-'0');i+=2;
		coor.min = (data[i]-'0')*10 + (data[i+1]-'0');i+=2;
		if(data[i+2]!= '.') {printf("Incompatible data received\n");return;}
		coor.sec = (data[i]-'0')*10 + (data[i+1]-'0') + (data[i+3]-'0')/10.0 + (data[i+4]-'0')/100.0;
		i+=5;
	}

	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Latitude
		coor.lat_deg = (data[i]-'0')*10 + (data[i+1]-'0');i+=2;
		if(data[i+2]!= '.') {printf("Incompatible data received\n");return;}
		coor.lat_min = (data[i]-'0')*10 + (data[i+1]-'0') + (data[i+3]-'0')/10.0 + (data[i+4]-'0')/100.0 + (data[i+5]-'0')/1000.0 + (data[i+6]-'0')/10000.0;
		i+=7;
		if(data[i++]!= ',') {printf("Incompatible data received\n");return;}
		coor.lat_dir = data[i++];
	}

	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Longitude
		coor.lon_deg = (data[i]-'0')*100 + (data[i+1]-'0')*10 + (data[i+2]-'0');i+=3;
		if(data[i+2]!= '.') {printf("Incompatible data received\n");return;}
		coor.lon_min = (data[i]-'0')*10 + (data[i+1]-'0') + (data[i+3]-'0')/10.0 + (data[i+4]-'0')/100.0 + (data[i+5]-'0')/1000.0 + (data[i+6]-'0')/10000.0;
		i+=7;
		if(data[i++]!= ',') {printf("Incompatible data received\n");return;}
		coor.lon_dir = data[i++];
	}


	printf("%sGPS location: %d°%2.4f'%c ; %d°%2.4f'%c ; time: %2d:%2d %2.2f\n",(coor.valid?"":"(invalid) "), coor.lat_deg,coor.lat_min,coor.lat_dir, coor.lon_deg,coor.lon_min,coor.lon_dir,coor.hour,coor.min,coor.sec);

}

void GPS_GPRMC(uint8_t * data){

	/*
	 * $GPRMC,hhmmss.ss,a,ddmm.mmmm,n,dddmm.mmmm,w,z.z,y.y,ddmmyy,d.d,v*CC<CR><LF>
	 * */

	GPSCoor coor;
	int i;

	memset(&coor,0,sizeof(GPSCoor));

	i=5;
	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Hour
		coor.hour = (data[i]-'0')*10 + (data[i+1]-'0');i+=2;
		coor.min = (data[i]-'0')*10 + (data[i+1]-'0');i+=2;
		if(data[i+2]!= '.') {printf("Incompatible data received\n");return;}
		coor.sec = (data[i]-'0')*10 + (data[i+1]-'0') + (data[i+3]-'0')/10.0 + (data[i+4]-'0')/100.0;
		i+=5;
	}


	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){
		switch(data[i++]){
		case 'A': coor.valid = true; break;
		case 'V': coor.valid = false; break;
		default:
			printf("Incompatible data received\n");
			return;
		}
	}

	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Latitude
		coor.lat_deg = (data[i]-'0')*10 + (data[i+1]-'0');i+=2;
		if(data[i+2]!= '.') {printf("Incompatible data received\n");return;}
		coor.lat_min = (data[i]-'0')*10 + (data[i+1]-'0') + (data[i+3]-'0')/10.0 + (data[i+4]-'0')/100.0 + (data[i+5]-'0')/1000.0 + (data[i+6]-'0')/10000.0;
		i+=7;
		if(data[i++]!= ',') {printf("Incompatible data received\n");return;}
		coor.lat_dir = data[i++];
	}

	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Longitude
		coor.lon_deg = (data[i]-'0')*100 + (data[i+1]-'0')*10 + (data[i+2]-'0');i+=3;
		if(data[i+2]!= '.') {printf("Incompatible data received\n");return;}
		coor.lon_min = (data[i]-'0')*10 + (data[i+1]-'0') + (data[i+3]-'0')/10.0 + (data[i+4]-'0')/100.0 + (data[i+5]-'0')/1000.0 + (data[i+6]-'0')/10000.0;
		i+=7;
		if(data[i++]!= ',') {printf("Incompatible data received\n");return;}
		coor.lon_dir = data[i++];
	}

	int aux = 0;
	coor.speed = 0;
	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Speed
		aux = 0;
		do{
			coor.speed = coor.speed*10+(data[i++]-'0');
			if(aux++>3){printf("Incompatible data received\n");return;}
		}while(data[i]!='.');
		i++;

		coor.speed = coor.speed+(data[i++]-'0')*0.1;
		coor.speed *= 1.852;
	}

	coor.asimuth = 0;
	if(data[i++]!=','){printf("Incompatible data received\n");return;}
	if(data[i]!=','){ // Azimuth
		aux = 0;
		do{
			coor.asimuth = coor.asimuth*10+(data[i++]-'0');
			if(aux++>3){printf("Incompatible data received\n");return;}
		}while(data[i]!='.');
		i++;

		coor.asimuth = coor.asimuth+(data[i++]-'0')*0.1;
	}

	printf("%sGPS location: %d°%2.4f'%c ; %d°%2.4f'%c ; %3.1f km/h ; %3.1f° ; time:%02d:%02d %02.2f\n",(coor.valid?"":"(invalid) "), coor.lat_deg,coor.lat_min,coor.lat_dir, coor.lon_deg,coor.lon_min,coor.lon_dir,coor.speed,coor.asimuth,coor.hour,coor.min,coor.sec);

	if(csv_save==true && csv_file != NULL)
		fprintf(csv_file,"%d,%2.4f,%c,%d,%2.4f,%c,%3.1f,%3.1f,%02d,%02d,%02.2f\n",coor.lat_deg,coor.lat_min,coor.lat_dir, coor.lon_deg,coor.lon_min,coor.lon_dir,coor.asimuth,coor.speed,coor.hour,coor.min,coor.sec);
	if(tck_save==true && tck_file != NULL)
		fprintf(tck_file,"%do%2.4f'%c\t%do%2.4f'%c\t%3.1fo\t%3.1fkmh\t%02d:%02d:%02.2f\n",coor.lat_deg,coor.lat_min,coor.lat_dir, coor.lon_deg,coor.lon_min,coor.lon_dir,coor.asimuth,coor.speed,coor.hour,coor.min,coor.sec);
	if(externFunc!=NULL)
		externFunc(&coor);
}

void * gps_rx_routine(void * arg){
	uint8_t data[1000];
	int i = 0;
	//char checksum = 0;

	while(exit_gps == false){
		if(Serialport_ReadByte(serialport,SERIAL_TIMEOUT)!='$')continue;

		i=0;
		while(1){
			if((data[i++]=Serialport_ReadByte(serialport,SERIAL_TIMEOUT))==255) break;
			if((i>2 && data[i-1]=='\n' && data[i-2]=='\r') || i>=1000) break;
		}

		if(data[i-1]==255){
			printf("Read command error!\n");
			continue; // capturar erro, por timeout ou outro
		}

		//int size = i;

		//data[size] = 0;
		//printf("Command received: $%s\n",data);

		// TODO verify checksum (optional)

		if(memcmp(data,"GPGGA",5)==0){
			GPS_GPGGA(data);
		}else if(memcmp(data,"GPRMC",5)==0){
			GPS_GPRMC(data);
		}else{
			printf("Command received not implemented!\n");
		}

		usleep(10000) ;
	}

	Serialport_Close(serialport);
	Serialport_Destroy(serialport);
	return 0;
}

void GPS_init(char * serial_name, void (*func)(GPSCoor*)){

	if(func != NULL)
		externFunc = func;
	else
		externFunc = NULL;

	exit_gps = false;
	csv_save = false;
	tck_save = false;

	serialport = Serialport_Create();

	Serialport_Open(serialport,serial_name,BAUD_4800, CHAR_SIZE_DEFAULT,PARITY_DEFAULT, STOP_BITS_DEFAULT,FLOW_CONTROL_DEFAULT);

	// Start reception
	Serialport_Write(serialport,"$PMOTG,RMC,0001\r\n",17);

	pthread_create(&tid_gps_rx, &tattr_gps_rx, gps_rx_routine,NULL);
	printf("\nThread GPS-rx created %d\n",(int)tid_gps_rx);
}

void csv_save_command(){
	time_t rawtime;
	struct tm * timeinfo;
	char times [80];

	if(csv_save==false){
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime (times,80,"Coors_%Y-%m-%d_%H:%M:%S.csv",timeinfo);

		csv_file = fopen(times,"w");
		if(csv_file == NULL){
			printf("Error trying create a file \"%s\"\n",times);
		}else{
			csv_save = true;
			fprintf(csv_file,"Lat_degrees,Lat_minutes,Lat_dir,Lon_degrees,Lon_minutes,Lon_dir,Azimuth,Speed,Hour,Minute,Second\n");
			printf("Started saving in CSV file...\n");
		}
	}else{
		if(csv_file != NULL)
			fclose(csv_file);
		csv_save = false;
		printf("CSV file saved...\n");
	}
}

void tck_save_command(){
	time_t rawtime;
	struct tm * timeinfo;
	char times [80];

	if(tck_save==false){
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		strftime (times,80,"Coors_%Y-%m-%d_%H:%M:%S.tck",timeinfo);

		tck_file = fopen(times,"w");
		if(tck_file == NULL){
			printf("Error trying create a file \"%s\"\n",times);
		}else{
			tck_save = true;
			printf("Started saving in TCK file...\n");
		}
	}else{
		if(tck_file != NULL)
			fclose(tck_file);
		tck_save = false;
		printf("TCK file saved...\n");
	}
}

void print_commands(){
	printf("Commands avaiable:\n");
	printf("	e - exit\n");
	printf("	h - help\n");
	printf("	l - enable/disable serial port log\n");
	printf("	g - get GPS data once time (GGA)\n");
	printf("	G - get GPS data every 2 seconds (GGA)\n");
	printf("	t - get GPS data once time (RMC)\n");
	printf("	T - get GPS data every 2 seconds (RMC)\n");
	printf("	f - start/stop saving in a TCK csv_file\n");
	printf("	F - start/stop saving in a CSV csv_file\n");
}

void GPS_test(){
	print_commands();
	while(exit_gps == false){
		switch(getchar()){
		case 'e':
			printf("Exit.\n");
			exit_gps = true;
			break;
		case 'l':
			if(serial_log)
				printf("Disable serial log.\n");
			else
				printf("Enable serial log.\n");
			serial_log = !serial_log;
			break;
		case 'g':
			printf("Get data...\n");
			Serialport_Write(serialport,"$PMOTG,GGA,0000\r\n",17);
			break;
		case 'G':
			printf("Get data...\n");
			Serialport_Write(serialport,"$PMOTG,GGA,0002\r\n",17);
			break;
		case 't':
			printf("Get data...\n");
			Serialport_Write(serialport,"$PMOTG,RMC,0000\r\n",17);
			break;
		case 'T':
			printf("Get data...\n");
			Serialport_Write(serialport,"$PMOTG,RMC,0002\r\n",17);
			break;
		case 'f':
			tck_save_command();
			break;
		case 'F':
			csv_save_command();
			break;
		case 'h':
			print_commands();
			break;
		}
	}
}

