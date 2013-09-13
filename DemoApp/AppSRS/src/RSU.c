/*
 * IamHere.c
 *
 *  Created on: 12 de Jul de 2012
 *      Author: fabio32883
 */

#define _GNU_SOURCE 1
#define _THREAD_SAFE 1

#include "MAC_API/itdsrc_mac.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* sleep function, getopt */

void indication_cb(const struct ma_unitdata_indication *indication);
void status_indication_cb(const struct ma_unitdata_status_indication *indication);
void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication);

void print_usage(void);

#include "so_tcplib/so_tcplib.h"


#include <stdlib.h> /* atoi, ... */
#include <string.h>
#include <pthread.h>


#ifndef _BOOL_
#define _BOOL_
typedef enum bool {TRUE, FALSE} bool;
#endif /* _BOOL_ */


struct ma_dev *dev;
uint8_t sa[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
uint8_t da[6] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15};

bool exitServer;

void* appCicle(void *v){
	int sock = (int)v;
	int i, exit = 0;
	char data [15];
	char * string = data+1;
	int size;

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
		printf("rec: %s\r\n",string);

		data[0]=1; // indica que é um RSU
		int retval = ma_unitdatax_request(dev, sa, da, (uint8_t *)data, size+1, 0, 0, 0, 0, 0 ,0);
		if (retval != 0) {
			fprintf(stderr, "ERROR: ma_unitdatax_request failed with error %d.\n", retval);
		}
	}

	close(sock);

	printf("Servidor encerrado.");
	//exitServer = TRUE;
	sleep(3);


	return 0;
}

void waitConnection(int port){
	exitServer = FALSE;
	int socksrv;
	pthread_t t;

	socksrv = tcpCreateServerSocket(NULL, port );
	if(socksrv < 0){
		fprintf(stderr,"ERRO: Nao conseguiu criar o servidor.\n");
		exitServer = TRUE;
	}

	if(exitServer == FALSE){
		printf("\nServidor ligado.\n");

		int sock, i=0;

		printf("###waitConnection\n");

		while(exitServer==FALSE){
			sock = tcpAccpetSocket(socksrv);
			if(sock < 0){
				fprintf(stderr,"	ERRO: Nao conseguiu estabelecer uma ligacao.\n");
				continue;
			}

			appCicle((void*)sock);
		}
	}

	close(socksrv);

	printf("Servidor desligado.\n");
}


int default_port = 1234;

void RSU_init(int port_n){
	if(port_n == -1){
		fprintf(stderr,"WARNING: Nao introduziu o numero do porto. (DEFAULT:%d)\n", default_port);
		port_n = default_port;//exitServer = TRUE;
	}
	waitConnection(port_n);
}

/*

int main(int argc, char ** argv){
	if(argc < 2){
		fprintf(stderr,"WARNING: Nao introduziu o numero do porto. (DEFAULT:%d)\n", default_port);
		port = default_port;//exitServer = TRUE;
	}else
		port = atoi(argv[1]);

	waitConnection();

	return 0;
}

/**/

int main (int argc, char **argv)
{
	int retval;
	//const char *data = "abcdefghijklmnopqrstuvwxyz";
	//const char *datax = "ABCDEFGHIJLMNOPQRSTUVXZ";
	//uint8_t data_length = 26;
	//uint8_t datax_length = 23;
	int device_id;
	char optch;
	char *optstring = "hd:";
	int port_n = -1;

	device_id = -1;

	do {
	    optch = getopt(argc, argv, optstring);

	    switch (optch) {
	        case 'h':
	            print_usage();
	            return 0;
	            break;

	        case 'd':
	            sscanf(optarg, "%d", &device_id);
	            break;

	        case 'p':
	            sscanf(optarg, "%d", &port_n);
	            break;

	        case -1:
	            break;

	        default:
	            print_usage();
	            return -1;
	            break;
	    }
	} while (optch != -1);


	if ((device_id < 0) || (device_id > 1)) {
	    fprintf(stderr, "Device id must be 0 or 1\n");
	    print_usage();
		return -1;
	}

	retval = ma_init(device_id, &dev, &indication_cb, &status_indication_cb, &xstatus_indication_cb);
	if (retval != 0) {
		fprintf(stderr, "ERROR: failed to initialize mac layer: %s.\n", strerror(retval));
		return -1;
	}

	RSU_init(port_n);


/*
	retval = ma_unitdata_request(dev, sa, da, (uint8_t *)data, data_length, 0, 0);
	if (retval != 0) {
		fprintf(stderr, "ERROR: ma_unitdata_request failed with error %d.\n", retval);
	}
	sleep(2);

	retval = ma_unitdatax_request(dev, sa, da, (uint8_t *)datax, datax_length, 0, 0, 0, 0, 0 ,0);
	if (retval != 0) {
		fprintf(stderr, "ERROR: ma_unitdatax_request failed with error %d.\n", retval);
	}
	sleep(2);
*/
	ma_stop(dev);

	return 0;
}

void indication_cb(const struct ma_unitdata_indication *indication)
{
	int i;

	printf("Indication here.\n");
	printf("Received a frame from ");
	for (i=0; i<6; ++i) {
		printf("%02X", indication->source_address[i]);
	}
	printf(" to ");
	for (i=0; i<6; ++i) {
		printf("%02X", indication->destination_address[i]);
	}
	printf("\n");

	printf("Load: ");
	for (i=0; i<indication->data_length; ++i) {
		printf("%02X", indication->data[i]);
	}
	printf("\n");
}

void status_indication_cb(const struct ma_unitdata_status_indication *indication)
{
	printf("Status indication: %u.\n", indication->transmission_status);
}

void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication)
{
	printf("XStatus indication: %u.\n", indication->transmission_status);
}

void print_usage(void)
{
    printf("itdsrc-daemon-loopback -d <device_id> [-h]\n");
    printf("  -h : this help\n");
    printf("  -d <device_id> : id of the device to connect to (must be 0 or 1)\n");
    printf("  -p <port> : port to waiting SRS connection\n");
}

/**/
