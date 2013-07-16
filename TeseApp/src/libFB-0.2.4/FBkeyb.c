/*! \file FBkeyb.c
 * \brief Keyboard handling through ncurses
 *
 * ncurses is used to get over all the complexity of the Linux console layer,
 * this way we don't need keymaps for different keyboard layouts or low level
 * ioctls.\n
 * begin                : Fri Feb 8 2002\n
 * copyright            : (C) 2001 by Daniele Venzano\n
 * email                : venza@users.sf.net */

/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License. */

#include <ncurses.h>

#include "FBlib.h"
#include "FBpriv.h"

int FB_kb_init()
{
	uses_keyboard = 1;
	initscr();
	cbreak();
	noecho();
	nonl();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	return OK;
}

int FB_get_key()
{
	int key;

	key = getch();
	if(key == ERR)
		return NO_KEY;
	return key;	
}

void FB_kb_end()
{
	endwin();
	return;
}
