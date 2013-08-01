/*
 * event.h
 *
 *  Created on: 1 de Out de 2012
 *      Author: fabio32883
 */

#include <stdbool.h>

#ifndef INPUT_H_
#define INPUT_H_

#ifndef _POINT_
#define _POINT_
typedef struct point_{
	int x;
	int y;
}Point;
#endif /* _POINT_ */



void input_init(char * input_dev); // inicia uma thread
void input_stop(); // parar a thread e fazer frees
void input_calibration();//pressupoe chamada ao VGA_init prévia, desenha janela de calibraçao
void input_getClickLock(Point * p);
int input_getClick(Point * p); //retorna o id do input
void input_print_values();

void input_flush();



#endif /* INPUT_H_ */
