/*
 * RoadView.c
 *
 *  Created on: 18 de Fev de 2013
 *      Author: fabio32883
 */


#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include <math.h>
#include <stdbool.h>

#include "../libFB-0.2.4/FBlib.h"
#include "event/event.h"
#include "RoadView.h"

Road * road = 0;

static pthread_attr_t tattr;
static pthread_t tid;

static pthread_mutex_t mutex;

static int exitRoadView;

static double meters_per_pixel;
static Point table_location;
static Point table_length;

static Position my_pos;

static bool warning;
static BITMAP * warning_bmp;

bool verifyX(int x){
	if(x<table_location.x+ARROW_RAD || x>table_location.x + table_length.x - ARROW_RAD) return false;
	return true;
}

bool verifyY(int y){
	if(y>table_location.y - ARROW_RAD || y<table_location.y - table_length.y + ARROW_RAD) return false;
	return true;
}


void drawCar(Vehicle * v) {
	
	// meter a origem do plano a posicao do veiculo
	int xx = v->pos->x - my_pos.x;
	int yy = v->pos->y - my_pos.y;

	// rodar plano para ver `a frente do veiculo
	int x = xx*cos(my_pos.d) - yy*sin(my_pos.d);
	int y = xx*sin(my_pos.d) + yy*cos(my_pos.d);
	double angle_rad = v->pos->d - my_pos.d;

	// meter (x,y) em pixels
	x = table_location.x + table_length.x/2 + (x/100.0/meters_per_pixel);
	y = table_location.y - ((y+CAR_BACK_CM_VIEW)/100.0/meters_per_pixel);
	
	angle_rad = angle_rad - atan(1)*2; // angulo para o desenho

	// verificar se esta' dentro da zona de desenho
	if(!(x > table_location.x &&  x < table_location.x + table_length.x && y < table_location.y &&  y > table_location.y - table_length.y))
		return;

	// definir a cor em funcao da velocidade
	int color;
	if(v->pos->vel<30)
		color = FB_makecol(0,200,0,0);
	else if(v->pos->vel<90)
		color = FB_makecol(0,0,200,0);
	else
		color = FB_makecol(200,0,0,0);
	
	// actualizar ultima posicao para depois fazer o delete
	v->last_x = x;
	v->last_y = y;
	v->last_vel = v->pos->vel;
	v->last_angle = angle_rad;

	// definir tamanho do vector em funcao da velocidade
	int arrow_vel = (int)((ARROW_COMP*v->pos->vel)/100) + ARROW_COMP/2 ;
	
	// desenhar o carro
	double sig = atan(1)*2 - angle_rad - ARROW_ANG;
	double del = angle_rad - ARROW_ANG;
	int x1 = x+arrow_vel*cos(angle_rad);
	int y1 = y+arrow_vel*sin(angle_rad);
	int x2 = x1-ARROW_HEAD_COMP*sin(sig);
	int y2 = y1-ARROW_HEAD_COMP*cos(sig);
	int x3 = x1-ARROW_HEAD_COMP*cos(del);
	int y3 = y1-ARROW_HEAD_COMP*sin(del);

	if(verifyX(x) && verifyY(y) && verifyX(x1) && verifyY(y1) 
		&& verifyX(x2) && verifyY(y2) && verifyX(x3) && verifyY(y3)){ 

		FB_circle(x,y,ARROW_RAD,color);
		FB_line(x,y,x1,y1,color);
		FB_line(x1,y1,x2,y2,color);
		FB_line(x1,y1,x3,y3,color);
	}
}

void eraseCar(Vehicle * v ){
	
	int color = BACKGROUND_COLOR;
	int x = v->last_x;
	int y = v->last_y;
	double angle_rad = v->last_angle;

	int arrow_vel = (int)((ARROW_COMP*v->last_vel)/100) + ARROW_COMP/2 ;	
	
	double sig = atan(1)*2 - angle_rad - ARROW_ANG;
	double del = angle_rad - ARROW_ANG;
	int x1 = x+arrow_vel*cos(angle_rad);
	int y1 = y+arrow_vel*sin(angle_rad);
	int x2 = x1-ARROW_HEAD_COMP*sin(sig);
	int y2 = y1-ARROW_HEAD_COMP*cos(sig);
	int x3 = x1-ARROW_HEAD_COMP*cos(del);
	int y3 = y1-ARROW_HEAD_COMP*sin(del);

	if(verifyX(x) && verifyY(y) && verifyX(x1) && verifyY(y1) 
		&& verifyX(x2) && verifyY(y2) && verifyX(x3) && verifyY(y3)){ 

		FB_circle(x,y,ARROW_RAD,color);
		FB_line(x,y,x1,y1,color);
		FB_line(x1,y1,x2,y2,color);
		FB_line(x1,y1,x3,y3,color);
	}
}

void * thread_routine(void *v){
	Road * r;
	while(!exitRoadView){
		pthread_mutex_lock(&mutex);

		r = road;
		while(r != 0){
			if(r->v->count++ == 150){ // eliminar por deixar de receber informações
				eraseCar( r->v);
				pthread_mutex_unlock(&mutex);
				RoadView_delete(r->v->id);
				pthread_mutex_lock(&mutex);
			}
			r = r->next;
		}

		pthread_mutex_unlock(&mutex);

		usleep(30000);
	}
	pthread_exit(0);
	return 0;
}

void free_snake(Vehicle * v){
	if(v->pos == NULL) return;
	if(v->pos->next == NULL) goto END;

	Position * p = v->pos->next, * a;
	do{
		a = p->next;
		free(p);		
		p = a;
	}while(p != v->pos);
END:
	free(v->pos);
	v->pos = NULL;
	return;
}

void RoadView_delete(int vehicle_id){
	pthread_mutex_lock(&mutex);

	Road * r = road;
	Road * last = NULL;

	while(r!=0){
		if(r->v->id == vehicle_id){
			if(last == NULL){
				free_snake(r->v);
				free(r->v);
				road = road->next;
				free(r);
			}else{
				free_snake(r->v);
				free(r->v);
				last->next = r->next;
				free(r);
			}
		}
		last = r;
		r = r->next;
	}

	pthread_mutex_unlock(&mutex);
}

void RoadView_update_my(int x_cm,int y_cm, int vel, unsigned int angle){
	my_pos.x = x_cm;
	my_pos.y = y_cm;
	my_pos.vel = vel;
//	if(vel>3) // asimuth errada via gps, TODO melhorar
		my_pos.d = (double)angle * atan(1)*4 /180.0;
	
	// redesenhar os outros veiculos
	pthread_mutex_lock(&mutex);
	Road * r = road;
	while(r != 0){
		eraseCar( r->v);
		drawCar( r->v);
		r = r->next;
	}
	pthread_mutex_unlock(&mutex);

	// actualizar o rectangulo do veiculo, pq pode ser destruido se outro veiculo for desenhado sobre ele	
	int car_width_s2 = 3/meters_per_pixel/2;
	int car_lenght_s2 = 5/meters_per_pixel/2;
	int car_back = CAR_BACK_CM_VIEW/100.0/meters_per_pixel;
	
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2, table_location.y-1-car_back-car_lenght_s2,
			table_location.x + table_length.x/2+car_width_s2, table_location.y - 1-car_back+car_lenght_s2, FB_makecol(0,0,255,0));
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2+1, table_location.y-1-car_back-car_lenght_s2-1,
			table_location.x + table_length.x/2+car_width_s2-1, table_location.y - 1-car_back+car_lenght_s2+1, FB_makecol(0,0,255,0));
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2-1, table_location.y-1-car_back-car_lenght_s2+1,
			table_location.x + table_length.x/2+car_width_s2+1, table_location.y - 1-car_back+car_lenght_s2-1, FB_makecol(0,0,255,0));
}

void RoadView_update_myCoor(Coor * c){
	FB_rectfill(table_location.x+table_length.x+6,table_location.y,table_location.x+table_length.x+150,table_location.y - 6*20,FB_makecol(0,0,0,0));
	FB_printf(table_location.x+table_length.x+15,table_location.y-2*20,FB_makecol(255,255,255,0),"ANG: %d deg",(int)(c->asimuth));
	FB_printf(table_location.x+table_length.x+15,table_location.y-20,FB_makecol(255,255,255,0),"VEL: %d km/h",(int)(c->vel));
	Coor_utmCalc(c);
	RoadView_update_my( c->x, c->y, c->vel, c->asimuth);
}

void RoadView_updateCoor(int vehicle_id,Coor * c){
	Coor_utmCalc(c);
	RoadView_update(vehicle_id, c->x, c->y, c->vel, c->asimuth);
}

void RoadView_update(int vehicle_id, int x_cm, int y_cm, int vel, unsigned int angle){
	pthread_mutex_lock(&mutex);
	
	Road * r = road;

	while(r != 0){
		if(r->v->id == vehicle_id){
			eraseCar( r->v);
			
			Position * p = r->v->pos;
			r->v->pos = (Position*)malloc(sizeof(Position));
			if(p->next == NULL || p->prev == NULL){
				p->next = p->prev = r->v->pos;
				r->v->pos->next = r->v->pos->prev = p;			
			}else{
				r->v->pos->next = p->next;
				p->next = r->v->pos;
				r->v->pos->prev = p;
			}
			r->v->pos->x = x_cm;
			r->v->pos->y = y_cm;
			r->v->pos->vel = vel;
			r->v->pos->d = (double)angle * atan(1)*4 /180.0;
			if(r->v->pos->vel < 5) // velocidades reduzidas resultam azimuths erradas
				r->v->pos->d = r->v->pos->prev->d;

			r->v->count = 0;
			
			if(r->v->snake_count > SNAKE_MAX_LEN){
				p = r->v->pos->next; // eliminar a ultima
				r->v->pos->next = p->next;
				free(p);
			}else
				r->v->snake_count++;

			drawCar( r->v);
			
			RoadView_caution(r->v);
			break;
		}
		r = r->next;
	}
	if(r == 0){
		r = road;
		road = (Road*)malloc(sizeof(Road));
		road->next = r;
		road->v = (Vehicle*)malloc(sizeof(Vehicle));
		road->v->pos = (Position *)malloc(sizeof(Position));
		road->v->id = vehicle_id;
		road->v->pos->x = x_cm;
		road->v->pos->y = y_cm;
		road->v->pos->vel = vel;
		road->v->pos->d = (double)angle * atan(1)*4 /180.0;
		road->v->pos->prev = NULL;
		road->v->pos->next = NULL;
		road->v->snake_count = 1;
		road->v->count = 0;
		r = road;

		drawCar( r->v);
		RoadView_caution(r->v);
	}

	pthread_mutex_unlock(&mutex);
}

void RoadView_stop(){
	exitRoadView = 1;
	event_stop();

	pthread_mutex_lock(&mutex);
	
	Road * r = road;

	while(r!=0){
		free_snake(r->v);
		free(r->v);
		road = r->next;
		free(r);
		r = road;
	}
	
	pthread_mutex_unlock(&mutex);
}

void RoadView_start() {
	
	event_init();
	warning = false;

	my_pos.d = atan(1)*2;
	my_pos.x = 0;
	my_pos.y = 0;


	int xm, ym;
	FB_getres(&xm, &ym);
	
	warning_bmp = FB_bitmapLoad(WARNING_BMP_NAME);

	table_location.x = 20;
	table_location.y = ym-50;
	table_length.x = xm-60-warning_bmp->width;
	table_length.y = ym-70;
	meters_per_pixel = METERS_VISIBLE*1.0/table_length.y;


	FB_clear_screen(FB_makecol(0,0,0,0));
	
	FB_rectfill(table_location.x-4, table_location.y+4,table_location.x + table_length.x+4,
				 table_location.y - table_length.y - 4, FB_makecol(190,190,255,0));
	FB_rectfill(table_location.x, table_location.y,table_location.x + table_length.x,
				 table_location.y - table_length.y, BACKGROUND_COLOR);


	int car_width_s2 = 3/meters_per_pixel/2;
	int car_lenght_s2 = 5/meters_per_pixel/2;
	int car_back = CAR_BACK_CM_VIEW/100.0/meters_per_pixel;
	
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2, table_location.y-1-car_back-car_lenght_s2,
			table_location.x + table_length.x/2+car_width_s2, table_location.y - 1-car_back+car_lenght_s2, FB_makecol(0,0,255,0));
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2+1, table_location.y-1-car_back-car_lenght_s2-1,
			table_location.x + table_length.x/2+car_width_s2-1, table_location.y - 1-car_back+car_lenght_s2+1, FB_makecol(0,0,255,0));
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2-1, table_location.y-1-car_back-car_lenght_s2+1,
			table_location.x + table_length.x/2+car_width_s2+1, table_location.y - 1-car_back+car_lenght_s2-1, FB_makecol(0,0,255,0));
	
	
	
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&tid, &tattr, thread_routine,NULL);
	
}

inline void RoadView_add_meters_visible(int meters){
	meters_per_pixel = (METERS_VISIBLE+meters)*1.0/table_length.y;
}

void RoadView_redraw(){
	FB_clear_screen(FB_makecol(0,0,0,0));

	FB_rectfill(table_location.x-4, table_location.y+4,table_location.x + table_length.x+4,
				 table_location.y - table_length.y - 4, FB_makecol(190,190,255,0));
	FB_rectfill(table_location.x, table_location.y,table_location.x + table_length.x,
				 table_location.y - table_length.y, BACKGROUND_COLOR);


	int car_width_s2 = 3/meters_per_pixel/2;
	int car_lenght_s2 = 5/meters_per_pixel/2;
	int car_back = CAR_BACK_CM_VIEW/100.0/meters_per_pixel;
	
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2, table_location.y-1-car_back-car_lenght_s2,
			table_location.x + table_length.x/2+car_width_s2, table_location.y - 1-car_back+car_lenght_s2, FB_makecol(0,0,255,0));
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2+1, table_location.y-1-car_back-car_lenght_s2-1,
			table_location.x + table_length.x/2+car_width_s2-1, table_location.y - 1-car_back+car_lenght_s2+1, FB_makecol(0,0,255,0));
	FB_rectfill(table_location.x+table_length.x/2-car_width_s2-1, table_location.y-1-car_back-car_lenght_s2+1,
			table_location.x + table_length.x/2+car_width_s2+1, table_location.y - 1-car_back+car_lenght_s2-1, FB_makecol(0,0,255,0));
	Road * r = road;

	while(r != 0){
		drawCar( r->v);
		r = r->next;
	}

}















void getVertices(int center_x, int center_y, double phi, double L, double W, double Z[4][2]){ // double [4][2] Z

	double A[2][2] = { { cos(phi), -sin(phi) }, { sin(phi), cos(phi) } };
	double K[4][2] = { { L, -W }, { L, W }, { -L, W }, { -L, -W } };

	int m,l,c;
	for (m = 0; m < 4; m++)
	{
		for (l = 0; l < 2; l++)
		{
			Z[m][l] = 0;
			for (c = 0; c < 2; c++)
			{
				Z[m][l] = Z[m][l] + 0.5 * A[l][c] * K[m][c];
			}
		}
		Z[m][0] = Z[m][0] + center_x;
		Z[m][1] = Z[m][1] + center_y;
	}
}

int left(int p_x, int p_y, int p1_x, int p1_y, int p2_x, int p2_y){
	if( (p2_x-p1_x)*(p_y-p1_y) - (p2_y-p1_y)*(p_x-p1_x) > 0)
		return 1;
	else
		return 0;
}

bool safetyZones(Vehicle * v){
	double angle1 = -my_pos.d + atan(1)*2;
	double angle2 = -v->pos->d + atan(1)*2;

	double v1 = my_pos.vel * 10.0/36.0;
	double v2 = v->pos->vel * 10.0 / 36.0;

	// dw do algoritmo da ieee
	double dw1 = TIME_REACTION * v1 + 0.1 * v1 + 0.006 * v1 * v1;
	double dw2 = TIME_REACTION * v2 + 0.1 * v2 + 0.006 * v2 * v2;
	
	// adicionar a compensacao da frequencia de amostragem em metros

	dw1 = dw1 + v1*POSITION_RATE;
	dw2 = dw2 + v2*POSITION_RATE;

	dw1 = dw1 * 100; //cm
	dw2 = dw2 * 100; //cm

	int center1_X = (int)(my_pos.x + cos(angle1) * dw1 / 2.0);
	int center1_Y = (int)(my_pos.y + sin(angle1) * dw1 / 2.0); 

	int center2_X = (int)(v->pos->x + cos(angle2) * dw2 / 2.0);
	int center2_Y = (int)(v->pos->y + sin(angle2) * dw2 / 2.0); 


	double Z1 [4][2], Z2 [4][2];

	getVertices(center1_X,center1_Y, angle1, CAR_L+dw1, CAR_W, Z1);
	getVertices(center2_X,center2_Y, angle2, CAR_L+dw2, CAR_W, Z2);

	int i,j,ii,jj;

	for(i=0;i<4;i++){
		ii = (i+1)%4;
		for(j=0;j<4;j++){
			jj = (j+1)%4;
			if( (left(Z1[i][0],Z1[i][1],Z2[j][0],Z2[j][1],Z2[jj][0],Z2[jj][1])
			     ^ left(Z1[ii][0],Z1[ii][1],Z2[j][0],Z2[j][1],Z2[jj][0],Z2[jj][1]))
			     & (left(Z2[j][0],Z2[j][1],Z1[i][0],Z1[i][1],Z1[ii][0],Z1[ii][1])
			     ^ left(Z2[jj][0],Z2[jj][1],Z1[i][0],Z1[i][1],Z1[ii][0],Z1[ii][1])) )
			return true;
		}
	}


/*	int count, i, j;
	// deteta ponto no interior de uma zona, nao e' bom
	for (i = 0; i < 4; i++) // percorrer os vertices de Z1 para verificar se estao no interior de Z2
	{
		count = 0;
		for (j = 0; j < 4; j++) // percorrer todas as arestas de Z2
		{
			if ((Z2[(j + 1)%4][0] - Z2[j][0]) * (Z1[i][1] - Z2[j][1]) - (Z2[(j + 1)%4][1] - Z2[j][1]) * (Z1[i][0] - Z2[j][0]) > 0)
				count++;
		}
		if (count == 4) return true;
	}
*/
	return false;
}


void warning_clean(void * v){
	warning = false;
	FB_rectfill(table_location.x+table_length.x+20,table_location.y-table_length.y,
			table_location.x+table_length.x+20+warning_bmp->width,table_location.y-table_length.y+warning_bmp->height,
			FB_makecol(0,0,0,0));
}

void warning_trow(){
	if(warning == false){
		warning = true;
		FB_bitmapDraw(warning_bmp,table_location.x+table_length.x+20,table_location.y-table_length.y);
		event_once_add(warning_clean,0, WARNING_BMP_MS);
	}
}

bool RoadView_caution(Vehicle * v){
	bool attention = false;

	double angle_dif = abs(v->pos->d - my_pos.d);
	if(angle_dif > atan(1)*4) angle_dif = 2*4*atan(1)-angle_dif;

	if(angle_dif < 2.79 ){ // >160º: sentido oposto
		attention = safetyZones(v);
	}
	
	if(attention){ 
		warning_trow();
		printf("WARNING\n");
	}
	return attention;
}
		
