/*
 * IamHere.c
 *
 *  Created on: 12 de Jul de 2012
 *      Author: fabio32883
 */

#define _GNU_SOURCE 1
#define _THREAD_SAFE 1

#include "itdsrc_mac.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* sleep function, getopt */
#include <sys/ioctl.h>
#include <stdbool.h>
#include <errno.h>

#define TIME_PER_FRAME 100000

void indication_cb(const struct ma_unitdata_indication *indication);
void status_indication_cb(const struct ma_unitdata_status_indication *indication);
void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication);

void print_usage(void);

#include "so_tcplib/so_tcplib.h"


#include <stdlib.h> /* atoi, ... */
#include <string.h>
#include <pthread.h>

struct ma_dev *dev;
uint8_t mac_addr[6];
uint8_t destination_mac_addr[6];
int power;
int modulation;
int count;
bool tip;
bool exitServer;

int sock;

void appCicle(){
	int i;
	char string[20];
	unsigned char size;
	int data2read = 0;
	int retval;
	bool recycle;

	printf("Ligacao estabelecida. \r\n");

	char buff;
	do{ // testar a ligacao
		read(sock,&buff,1);
		//printf("%d\r\n",buff);
		buff++;
		write(sock,&buff,1);
		read(sock,&buff,1);
	}while(buff!='s');



	while(exitServer == false){
		//if(ioctl(sock, FIONREAD, &data2read) == -1) break;
		//if (data2read > 0){
		//}else
		//	continue;
		retval = read(sock,&size,1);
		if(retval == -1) {
			if (errno == EAGAIN) {
				usleep(100000); // 100 ms
				continue;
			} else {
				break;
			}
		}

		printf("size: %d \r\n",size);

		if(size>20)size=20;
		if(size==0) continue;

		ioctl(sock, FIONREAD, &data2read);

		recycle = true;
		do {
			retval = read(sock,string,size);
			if(retval == -1) {
				if (errno == EAGAIN) {
					usleep(100000); // 100 ms
					continue;
				} else {
					break;
				}
			} else {
				recycle = false;
			}
		} while (recycle);

		string[size-1] = 0;
		printf("rec: %s (%zd)\r\n",string, strlen(string));

		// send frames to the other obu
		for (i=0; i < count; ++i) {
			tip = true;
			retval = ma_unitdatax_request(dev, mac_addr, destination_mac_addr, (uint8_t *)string, (uint16_t) size, 0, 0, 0, (uint8_t)modulation, (uint8_t)power, 0);
			if (retval != 0) {
				printf("ma_unitdatax_request failed: %s\n", strerror(retval));
			}

			while(tip) {
				usleep(1000);
				if (exitServer) goto cleanup;
			}
		}
	}

cleanup:
	close(sock);

	printf("Ligacao terminada.\r\n");
	sleep(3);
}


void waitConnection(int port){
	exitServer = false;
	int socksrv;

	socksrv = tcpCreateServerSocket(NULL, port );
	if(socksrv < 0){
		fprintf(stderr,"ERRO: Nao conseguiu criar o servidor.\n\r");
		exitServer = true;
	}

	if(exitServer == false){
		printf("\nServidor ligado.\n\r");

		printf("###waitConnection\n\r");

		while(exitServer==false){
			sock = tcpAccpetSocket(socksrv);
			if(sock < 0 && exitServer==false){
				usleep(100000); // sleep for 100ms
				if (errno == EAGAIN) {
					continue;
				} else {
					fprintf(stderr,"	ERRO: Nao conseguiu estabelecer uma ligacao.\r\n");
					continue;
				}
			}

			appCicle(sock);

		}
	}

	close(socksrv);
	sleep(3);
	printf("Servidor desligado.\n\r");
}

void * kbd_routine(void * v){
	while(exitServer == false){
		switch(getchar()){
		case 'e':
			exitServer = true;
			printf("Exit command...\r\n");
			break;
		default: 
			break;
		}
	}
	return 0;
}

static pthread_attr_t tattr_kbd;
static pthread_t tid_kbd;

int default_port = 1234;

void RSU_run(int port_n){

	printf("Port: %d\r\n", port_n);

	pthread_create(&tid_kbd, &tattr_kbd, kbd_routine,(void*)port_n);

	if(port_n == -1){
		fprintf(stderr,"WARNING: Nao introduziu o numero do porto. (DEFAULT:%d)\n", default_port);
		port_n = default_port;//exitServer = true;
	}
	waitConnection(port_n);
}

/*

int main(int argc, char ** argv){
	if(argc < 2){
		fprintf(stderr,"WARNING: Nao introduziu o numero do porto. (DEFAULT:%d)\n", default_port);
		port = default_port;//exitServer = true;
	}else
		port = atoi(argv[1]);

	waitConnection();

	return 0;
}

*/

int main (int argc, char **argv)
{
	int retval;
	int optch;
	char *optstring = "hb:c:d:m:p:P:";
	int port_n = -1;
	int backoff;
	int destination_id;

	backoff = 0;
	count = 10;
	power = 0;
	destination_id = -1;

	do {
	    optch = getopt(argc, argv, optstring);

	    switch (optch) {
	        case 'h':
	            print_usage();
	            return 0;
	            break;
	
	        case 'b':
	            sscanf(optarg, "%d", &backoff);
	            break;
	
	        case 'c':
	            sscanf(optarg, "%d", &count);
	            break;
	
	        case 'd':
	            sscanf(optarg, "%d", &destination_id);
	            break;
	
	        case 'm':
	            sscanf(optarg, "%d", &modulation);
	            break;
	
	        case 'p':
	            sscanf(optarg, "%d", &power);
	            break;

	        case 'P':
	            port_n = atoi(optarg);
	            break;

	        case -1:
	            break;

	        default:
				printf("%c not found\r\n",optch);
	            print_usage();
	            return -1;
	            break;
	    }
	} while (optch != -1);

	if ((backoff > 7) || (backoff < 0)) {
		fprintf(stderr, "Backoff must be positive and not exceed %d\n", 7);
		print_usage();
		return -1;
	}

	if ((count > 99) || (count < 0)) {
		fprintf(stderr, "Count must be positive and not exceed %d\n", 99);
		print_usage();
		return -1;
	}
	
	if ((destination_id < 0) || (destination_id > 1)) {
	    fprintf(stderr, "Destination id must be specified and be 0 or 1\n");
	    print_usage();
		return -1;
	}

	if ((modulation > 7) || (modulation < 0)) {
		fprintf(stderr, "Modulation must be positive and not exceed %d\n", 7);
		print_usage();
		return -1;
	}

	if ((power > 63) || (power < 0)) {
		fprintf(stderr, "Power must be positive and not exceed %d\n", 7);
		print_usage();
		return -1;
	}
	
	destination_mac_addr[0] = 0;
	destination_mac_addr[1] = 0;
	destination_mac_addr[2] = 0;
	destination_mac_addr[3] = 0;
	destination_mac_addr[4] = 0;
	destination_mac_addr[5] = (uint8_t) destination_id;

	mac_addr[0] = 0;
	mac_addr[1] = 0;
	mac_addr[2] = 0;
	mac_addr[3] = 0;
	mac_addr[4] = 0;
	mac_addr[5] = 11;

	retval = ma_init(0, &dev, &indication_cb, &status_indication_cb, &xstatus_indication_cb);
	if (retval != 0) {
		fprintf(stderr, "ERROR: failed to initialize mac layer: %s.\n", strerror(retval));
		return -1;
	}

	if(port_n <= 100){
		fprintf(stderr, "Invalid port number\n");
	    	print_usage();
		return -1;
	}

	printf("Start\r\n");

	// this blocks until application is done
	RSU_run(port_n);

	ma_stop(dev);

	return 0;
}

void indication_cb(const struct ma_unitdata_indication *indication)
{
	//int i;

	//printf("Indication here.\n");
	//printf("Received a frame from ");
	//for (i=0; i<6; ++i) {
	//	printf("%02X", indication->source_address[i]);
	//}
	//printf(" to ");
	//for (i=0; i<6; ++i) {
	//	printf("%02X", indication->destination_address[i]);
	//}
	//printf("\n");

	//printf("Load: ");
	//for (i=0; i<indication->data_length; ++i) {
	//	printf("%02X", indication->data[i]);
	//}
	//printf("\n");
}

void status_indication_cb(const struct ma_unitdata_status_indication *indication)
{
	//printf("Status indication: %u.\n", indication->transmission_status);
}

void xstatus_indication_cb(const struct ma_unitdatax_status_indication *indication)
{
	tip = false;
	//printf("XStatus indication: %u.\n", indication->transmission_status);
}

void print_usage(void)
{
    printf("itdsrc-daemon-loopback -d <device_id> [-h]\n");
    printf("  -h : this help\n");
	printf("  -b : backoff slots (0)\n");
	printf("  -c : frame count (10)\n");
    printf("  -d <device_id> : id of the destination device (must be 0 or 1)\n");
	printf("  -m : modulation (3)\n");
	printf("  -p : transmission power (0)\n");
    printf("  -P <port> : port to waiting SRS connection\n");
}

/**/
