/************************************************************************
*            Atrinik, a Multiplayer Online Role Playing Game            *
*                                                                       *
*    Copyright (C) 2009-2011 Alex Tokar and Atrinik Development Team    *
*                                                                       *
* Fork from Daimonin (Massive Multiplayer Online Role Playing Game)     *
* and Crossfire (Multiplayer game for X-windows).                       *
*                                                                       *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 2 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program; if not, write to the Free Software           *
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.             *
*                                                                       *
* The author can be reached at admin@atrinik.org                        *
************************************************************************/

/**
 * @file
 * Event related header file. */

#ifndef EVENT_H
#define EVENT_H

enum
{
	DRAG_GET_STATUS = -1,
	DRAG_NONE,
	DRAG_IWIN_BELOW,
	DRAG_IWIN_INV,
	DRAG_QUICKSLOT,
	DRAG_QUICKSLOT_SPELL,
	DRAG_PDOLL
};

/**
 * Key information. */
typedef struct key_struct
{
	/** If 1, the key is pressed. */
	int pressed;

	/** Last repeat time. */
	uint32 time;
} key_struct;

int old_mouse_y;
key_struct keys[SDLK_LAST];

#endif
