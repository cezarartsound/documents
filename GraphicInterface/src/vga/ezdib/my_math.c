// Cproject.cpp : Defines the entry point for the console application.
//

#include "my_math.h"


double sin(double x)
{
	x = x - ((int)(x/(2*PI)))*2*PI;

	if(x>PI)
		x = x-2*PI;
	else if(x<-PI)
		x = x+2*PI;

    double x2 = x*x;
    double x4 = x2*x2;

    double result = x * (1.0 - x2 / (2*3));
    double xp = x * x4; //x5
    result += xp * (1.0 - x2 / (6*7)) / (1.0* 2*3*4*5);
    xp = xp * x4; //x9
    result += xp * (1.0 - x2 / (10*11)) / (1.0* 2*3*4*5*6*7*8*9);
    xp = xp * x4; //x13
    result += xp * (1.0 - x2 / (14*15)) / (1.0* 2*3*4*5*6*7*8*9*10*11*12*13);

    return result;
}

double cos(double x)
{
	x = x - ((int)(x/(2*PI)))*2*PI;

	if(x>PI)
		x = x-2*PI;
	else if(x<-PI)
		x = x+2*PI;

    double x2 = x*x;

    double result = 1.0 - x2 / 2;
    double xp = x2 * x2; //x4
    result += xp * (1.0) / (1.0* 2*3*4);
    xp = xp * x2; //x6
    result -= xp * (1.0) / (1.0* 2*3*4*5*6);
    xp = xp * x2; //x8
    result += xp * (1.0) / (1.0* 2*3*4*5*6*7*8);
    xp = xp * x2; //x10
    result -= xp * (1.0) / (1.0* 2*3*4*5*6*7*8*9*10);
    xp = xp * x2; //x12
    result += xp * (1.0) / (1.0* 2*3*4*5*6*7*8*9*10*11*12);

    return result;
}

double tan(double x)
{
	double result;

	x = x - ((int)(x/PI))*PI;

	if(x>PI/2)
		x = x-PI;
	else if(x<-PI/2)
		x = x+PI;
	else if(x==PI/2)
		return 1000;
	else if(x==-PI/2)
		return -1000;

    double x2 = x*x;

	result = x*(1.0 + x2/3.0);
	double xp = x2 * x2 * x; //x5
	result += xp*(2.0/15.0);
	xp = xp *x2; //x7
	result += xp*(17.0/315.0);
	xp = xp*x2; //x9
	result += xp*(62.0/2835.0);
	xp = xp*x2; //x11
	result += xp*(1382.0/155925.0);
	xp = xp*x2; //x13
	result += xp*(21844.0/6081075.0);
	
    return result;
}
