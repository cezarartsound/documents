/*
 * event.c
 *
 *  Created on: 13 de Nov de 2012
 *      Author: fabio32883
 */

#define _GNU_SOURSE

#include "event.h"
#include <pthread.h>
#include <stdlib.h> //malloc
#include <stdio.h>
#include <sys/time.h>


static Event_List * event_list;

static pthread_mutex_t* event_mutex;
static pthread_attr_t tattr_event;
static pthread_t tid_event;

static unsigned char eventstop;

void * event_routine(void * v){

	Event_List * event_curr = event_list;

	while(!eventstop){
		pthread_mutex_lock(event_mutex);

		if(event_curr == 0){
			event_curr = event_list;
		}else{
			if(event_curr->event.act){
				pthread_mutex_unlock(event_mutex);
				event_curr->event.func(event_curr->event.parameter);
				pthread_mutex_lock(event_mutex);
			}
			event_curr = event_curr->next;
		}

		pthread_mutex_unlock(event_mutex);
//		pthread_yield();
	}
	return 0;
}

void event_init(){
	event_list = 0;

	event_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(event_mutex, NULL);

	eventstop = 0;

	pthread_create(&tid_event, &tattr_event, event_routine,NULL);
	printf("\nThread event created %d\n",(int)tid_event);

}

void event_stop(){
	eventstop = 1;
	//TODO free all
}

Event* event_add(void (*func)(void*),void * parameter){
	pthread_mutex_lock(event_mutex);

	Event_List * next = event_list;
	event_list = (Event_List*)malloc(sizeof(Event_List));

	event_list->event.func = func;
	event_list->event.parameter = parameter;
	event_list->event.act = 1;

	event_list->next = next;

	next = event_list;
	pthread_mutex_unlock(event_mutex);

	return (Event*)next;
}

void timedEvent(void * v){
	struct timeval curr_time;
	if ( gettimeofday( &curr_time,NULL ) < 0 ){
		return;
	}

	EventTimed * timed = (EventTimed * ) v;

	int actual_ms = (curr_time.tv_usec/1000 + curr_time.tv_sec*1000) ;
	if(actual_ms-timed->start_ms >= timed->goal_ms){
		timed->func(timed->parameter);

		Event * node = timed->node;
		free(timed);
		event_remove(node);
	}
}

Event* event_once_add(void (*func)(void*),void * parameter, int ms){
	struct timeval curr_time;
	if ( gettimeofday( &curr_time,NULL ) < 0 ){
		return 0;
	}

	pthread_mutex_lock(event_mutex);

	Event_List * next = event_list;
	event_list = (Event_List*)malloc(sizeof(Event_List));

	EventTimed * timed = (EventTimed*)malloc(sizeof(EventTimed));
	timed->func = func;
	timed->parameter = parameter;
	timed->goal_ms = ms;
	timed->start_ms = (curr_time.tv_usec/1000 + curr_time.tv_sec*1000) ;
	timed->node = (Event*)event_list;

	event_list->event.func = timedEvent;
	event_list->event.parameter = timed;
	event_list->event.act = 1;

	event_list->next = next;

	next = event_list;
	pthread_mutex_unlock(event_mutex);

	return (Event*)next;
}

void event_remove(Event * ev){
	if(ev == NULL) return;

	pthread_mutex_lock(event_mutex);

	Event_List * aux, * aux1;

	if(event_list != 0 && &event_list->event == ev){
		aux = event_list->next;
		free(event_list);
		event_list = aux;
	}else{
		aux = event_list;

		while(aux->next != 0){
			if(&aux->next->event == ev){
				aux1 = aux->next->next;
				free(aux->next);
				aux->next = aux1;
				break;
			}
			aux = aux->next;
		}
	}
	pthread_mutex_unlock(event_mutex);

}
