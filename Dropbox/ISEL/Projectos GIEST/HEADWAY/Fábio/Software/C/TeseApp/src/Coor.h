/*
 * Coor.h
 *
 *  Created on: 20 de Jun de 2013
 *      Author: fabio
 */

#ifndef COOR_H_
#define COOR_H_


typedef enum _Coor_calc {NONE = 0x0, UTM = 0x1, GEO = 0x2 , BOTH = 0x3} Coor_calc;


typedef struct _Coor{

	double lon;
        double lat;
        double asimuth;
        double vel;

        int x; //cm em funcao de origin
        int y; //cm

        Coor_calc calc;

        struct _Coor * origin;
}Coor;


Coor * Coor_new0();
Coor * Coor_new1(double lon, double lat, double asimuth, double vel);
Coor * Coor_new2(int lon_degree, double lon_min, double lon_sec, int lat_degree, double lat_min, double lat_sec, double asimuth, double vel);
void Coor_destroy(Coor * c);
void Coor_utmCalc(Coor * c);
Coor * Coor_parce_wO(char* s, Coor* dest, Coor * origin);
Coor * Coor_parce_str(char* s, Coor* dest);









#endif /* COOR_H_ */
