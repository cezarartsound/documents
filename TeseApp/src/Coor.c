/*
* Coor.c
*
*  Created on: 20 de Jun de 2013
*      Author: fabio
*/

#include <malloc.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h> //strtod
#include "Coor.h"

static double a = 6378137;
static double sf = 298.257223563; // 1/f
static double k0 = 0.9996;

static int lon_positive_cm [] = { 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 96, 102, 108, 114, 120, 126, 132, 138, 144 , 150,156,162,168,174,180
												,186,192,198,204,210,216,222,228,234,240,246,252,259,264,270,276,282,288,294,300,306,312,318,324,330,336,342,348,354,360};
static int cm_degree [] = { 3,9,15,21,27,33,39,45,51,57,63,69,75,81,87,93,99,105,111,117,123,129,135,141,147,153,159,165,171,177
									 ,183,189,195,201,207,213,219,225,231,237,243,249,255,261,267,273,279,285,291,297,303,309,315,321,327,333,339,345,351,357};


Coor * Coor_new0(){
	Coor * c = (Coor*)malloc(sizeof(Coor));
	c->calc = NONE;
	return c;
}

Coor * Coor_new1(double lon, double lat, double asimuth, double vel){
	Coor * c = (Coor*)malloc(sizeof(Coor));

	c->lon = lon;
	c->lat = lat;
	c->asimuth = asimuth;
	c->vel = vel;
	c->origin = NULL;
	c->calc = GEO;

	return c;
}

Coor * Coor_new2(int lon_degree, double lon_min, double lon_sec, int lat_degree, double lat_min, double lat_sec, double asimuth, double vel){
	Coor * c = (Coor*)malloc(sizeof(Coor));

	c->lon = lon_degree + lon_min * 1.0 / 60 + lon_sec * 1.0 / 3600;
	c->lat = lat_degree + lat_min * 1.0 / 60 + lat_sec * 1.0 / 3600;

	c->asimuth = asimuth;
	c->vel = vel;
	c->calc = GEO;

	c->origin = NULL;

	return c;
}

void Coor_destroy(Coor * c){
	free(c);
}

void Coor_utmCalc(Coor * c){
	if(c->calc & UTM) return; // already calculated
	if(!(c->calc & GEO)) return; // dont have geo coor to calculate UTM

	int PI = 4*atan(1);

	int i=0;
	while(lon_positive_cm[i]>=c->lon);

	double delta0 = cm_degree[i]*PI/180; //lon0
	double delta = - c->lon * PI / 180;
	double phi = c->lat * PI / 180;

	double e2 = 2 * 1 / sf - 1 / (sf*sf);
	double e2p = e2 / (1 - e2);

	//double RM = a*(1-e2)/pow(1-e2*pow(sin(phi),2),3.0/2);
	double RN = a / sqrt(1 - e2 * pow(sin(phi), 2));

	double T = pow(tan(phi), 2);
	double C = e2p * pow(cos(phi), 2);
	double A = -(-delta0 - delta) * cos(phi);

	double M1 = (1 - e2 / 4 - 3 * e2 * e2 / 64 - 5 * e2 * e2 * e2 / 256) * phi;
	double M2 = (3 * e2 / 8 + 3 * e2 * e2 / 32 + 45 * e2 * e2 * e2 / 1024) * sin(2 * phi);
	double M3 = (15 * e2 * e2 / 256 + 45 * e2 * e2 * e2 / 1024) * sin(4 * phi);
	double M4 = (35 * e2 * e2 * e2 / 3072) * sin(6 * phi);
	double M = a*(M1 - M2 + M3 - M4);

	c->x = (int)(100 * k0 * RN * (A + (1 - T + C) * A * A * A / 6 + (5 - 18 * T + T * T + 72 * C - 58 * e2p) * pow(A, 5) / 120)) + 50000000;
	c->y = (int)(100 * k0 * (M + RN * tan(phi) * (A * A / 2 + (5 - T + 9 * C + 4 * C * C) * pow(A, 4) / 24 + (61 - 58 * T + T * T + 600 * C - 330 * e2p) * pow(A, 6) / 720)));
	c->calc = BOTH;

	if(c->origin != NULL){
		Coor_utmCalc(c->origin);
		c->x = (int)((c->x - c->origin->x));
		c->y = (int)((c->y - c->origin->y));
	}
}

Coor * Coor_parce_wO(char* s, Coor * dest, Coor * origin){
	Coor * ret = Coor_parce_str(s,dest);
	ret->origin = origin;
	return ret;
}

double get_nr(char * s, int * i){
	double nr = 0;
	double dec = 10;
	int c = *i;
	bool coma = false;
	
	while((s[c]<'0' || s[c] > '9') && s[c]!='\0')c++;
	
	while((s[c] >= '0' && s[c] <= '9') || s[c] == '.'){	
		if(s[c] == '.'){
 			if(coma == true) break;
			else coma = true;
		}else{
			if(!coma)
				nr = nr*10 + s[c] - '0';
			else{
				nr = nr + (s[c]-'0')/dec;
				dec*=10;
			}
		}
		c++;
	}
	*i = c;
	return nr;
}


Coor * Coor_parce_str(char* s, Coor* dest){

	int i = 0, j;
	double degree, min, sec;

	for(j = 0;j<2; j++){
		
		degree = get_nr(s,&i);
		if(s[i] != 'o')
			goto ERROR;
		
		min = get_nr(s,&i);
		if (s[i] != '\'')
			goto ERROR;
		i++;

		if(s[i]>='0' && s[i]<='9'){ // pode n ter os segundos
			sec = get_nr(s,&i); 
			if (s[i] != '"' && sec != 0)
				goto ERROR;
			i++;
		}

		switch (s[i])
		{
			case 'N':
				dest->lat = degree + min*1.0/60 + sec*1.0/3600;
				break;
			case 'O':
			case 'W':
				dest->lon = degree + min * 1.0 / 60 + sec * 1.0 / 3600;
				break;
			default:
				goto ERROR;
		}
	}

	dest->asimuth = get_nr(s,&i);
	if (s[i++] != 'o' && dest->asimuth!=0) goto ERROR;
	i++;

	dest->vel = get_nr(s,&i);
	if (s[i++] != 'k' && dest->vel !=0 ) goto ERROR;
	i++;

	dest->calc = GEO;

	return dest;

ERROR:
	printf("Error parcing string to Coor.\n");
	return NULL;

}

/*
Coor * Coor_parce_str(char* s, Coor* dest){

	int i = 0, j;
	int degree, min;
	double sec;

	for(j = 0;j<2; j++){
		degree = s[i++]-'0';

		if (s[i] != 'o')
		{
			degree = degree * 10 + s[i++]-'0';
			if(s[i] != 'o')
				goto ERROR;
		}
		i++;
		
		min = s[i++] - '0';

		if (s[i] != '\'')
		{
			min = min * 10 + s[i++] - '0';
			if (s[i] != '\'')
				goto ERROR;
		}
		i++;

		sec = s[i++] - '0';

		if (s[i] != '.' && s[i] != ','){
			sec = sec * 10 + s[i++] - '0';
			if (s[i] != '.' && s[i] != ',' && s[i] != '"')
				goto ERROR;
		}

		if (s[i] == '.' || s[i] == ','){
			i++;
			sec = sec + (s[i++] - '0') * 0.1;
			if (s[i] != '"')
			{
				sec = sec + (s[i++] - '0') * 0.01;
				if (s[i] != '"')
					goto ERROR;
			}
		}
		i++;

		switch (s[i++])
		{
			case 'N':
				dest->lat = degree + min*1.0/60 + sec*1.0/3600;
				break;
			case 'O':
			case 'W':
				dest->lon = degree + min * 1.0 / 60 + sec * 1.0 / 3600;
				break;
			default:
				goto ERROR;
		}
		i++;
	}

	char * end;
	dest->asimuth = strtod (s+i, & end);//coor->asimuth = Convert.ToDouble(s.Substring(i, h-i-1));
	if (end == s+i) goto ERROR;

	while (s[i++] != 'o') if ((size_t)i == strlen(s)) goto ERROR;
	i++;

	dest->vel = strtod (s+i, & end);
	if (end == s+i) dest->vel = -1;

	while (s[i++] != 'k')
		if ((size_t)i == strlen(s)) {
			dest->vel = -1;
			break;
		}
	i++;

	dest->calc = GEO;

	return dest;

ERROR:
	printf("Error parcing string to Coor.\n");
	return NULL;

}*/
