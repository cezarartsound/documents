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
	double lat;
	double lon;
}Coor;


typedef struct _SHORT_POSITION_MESSAGE{
	uint8_t month;
	uint8_t day;
	uint16_t year;
	uint8_t hour;
	uint8_t minutes;
	uint8_t seconds;
	uint32_t fraq_seconds;

	int32_t lat_d;
	int32_t lon_d;
	int32_t GPS_height_d;
	int32_t MLS_height_d;

	uint16_t speed_3D_d;
	uint16_t speed_2D_d;
	uint16_t heading_d;

	uint16_t DOP_d;
	uint8_t visible_satellite;
	uint8_t tracked_satellite;
	uint16_t receiver_status;

	uint16_t reserved;

	uint8_t ID[6];

	bool converted;

	double lat;
	double lon;
	double GPS_height;
	double MLS_height;

	double speed_3D;
	double speed_2D;
	double heading;

	double DOP;
}SHORT_POSITION_MESSAGE;

void GPS_init();

#endif /* GPS_H_ */
