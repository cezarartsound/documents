/*
 * RoadView.h
 *
 *  Created on: 18 de Fev de 2013
 *      Author: fabio32883
 */

#ifndef ROADVIEW_H_
#define ROADVIEW_H_


#include <stdbool.h>
#include "../Coor.h"

#define BACKGROUND_COLOR FB_makecol(255,255,255,0)

#define ARROW_ANG (atan(1))
#define ARROW_COMP 20		// comprimento medio do vetor de velocidade, pixels
#define ARROW_HEAD_COMP 7	// comprimento das barras da cabeça do vetor, pixels
#define ARROW_RAD 4		// raio do circulo do vetor, pixels

#define CAR_BACK_CM_VIEW 1000 // metros visiveis da traseira do veiculo
#define METERS_VISIBLE 100 // metros visiveis em toda a altura da janela

#define WARNING_BMP_MS 2000  // tempo em ms que a imagem de aviso permanece
#define WARNING_BMP_NAME "images/warning.png"  // imagem a aparecer em situaçao de aviso

#define SNAKE_MAX_LEN 20 // numero de coordenadas da cobra

#define POSITION_RATE 1.1 // segundos
#define TIME_REACTION 1.3  // tempo de reaçao em segundos
#define CAR_L 700  // largura do carro em cm
#define CAR_W 600 //comprimento do carro em cm

typedef struct _point{int x; int y;} Point;

typedef struct _position{
	int x; //cm
	int y; //cm
	int vel; //km/h
	double d; // rad
	
	struct _position * next;
	struct _position * prev;
}Position;

typedef struct _vehicle{
	int id;

	int snake_count;
	Position * pos;

	int count; // para eliminar veiculo por timeout

	int last_x; // pixels
	int last_y; // pixels
	int last_vel; // km/h
	double last_angle; // rad
}Vehicle;

typedef struct _road{
	Vehicle * v;
	struct _road * next;
}Road;





void RoadView_start();
void RoadView_stop();

void RoadView_delete(int vehicle_id);

void RoadView_updateCoor(int vehicle_id,Coor * c);
void RoadView_update_myCoor(Coor * c);
void RoadView_update(int vehicle_id, int x_cm, int y_cm, int vel, unsigned int angle);
void RoadView_update_my(int x_cm, int y_cm, int vel, unsigned int angle);

bool RoadView_caution(Vehicle * v);

inline void RoadView_add_meters_visible(int meters);
void RoadView_redraw();

#endif /* ROADVIEW_H_ */
