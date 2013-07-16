/*
 * GPS.h
 *
 *  Created on: 27 de Mai de 2013
 *      Author: fabio32883
 */

#include <stdint.h>
#include <stdbool.h>

#ifndef GPS_H_
#define GPS_H_


#define SERIAL_TIMEOUT 3000

typedef struct _coor{
	int lat_deg;
	double lat_min;
	char lat_dir;

	int lon_deg;
	double lon_min;
	char lon_dir;

	int hour;
	int min;
	double sec;

	double speed; // km/h
	double asimuth; //degree

	bool valid;
}GPSCoor;


void GPS_init(char * serial_name, void (*func)(GPSCoor*));

#endif /* GPS_H_ */
