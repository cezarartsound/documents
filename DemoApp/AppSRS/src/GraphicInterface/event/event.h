/*
 * event.h
 *
 *  Created on: 13 de Nov de 2012
 *      Author: fabio32883
 */

#ifndef EVENT_H_
#define EVENT_H_

#ifndef _BOOL_
#define _BOOL_
typedef enum bool {TRUE, FALSE} bool;
#endif /* _BOOL_ */

typedef struct event_{
	bool act;
	void * parameter;
	void (*func)(void*);
}Event;

typedef struct event_list_{
	Event event;
	int id;
	struct event_list_ * next;
}Event_List;


void event_init(); //inicia uma thread
void event_stop(); //parar a thread e fazer frees
Event * event_add(void (*func)(void*),void * parameter); //retorna id do evento
Event* event_once_add(void (*func)(void*),void * parameter, int ms); // activa o evento apos ms e dps remove-o



#endif /* EVENT_H_ */
