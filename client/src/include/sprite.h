/*************************************************************************
 *           Atrinik, a Multiplayer Online Role Playing Game             *
 *                                                                       *
 *   Copyright (C) 2009-2014 Alex Tokar and Atrinik Development Team     *
 *                                                                       *
 * Fork from Crossfire (Multiplayer game for X-windows).                 *
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
 * Sprite header file. */

#ifndef SPRITE_H
#define SPRITE_H

/**
 * @defgroup SPRITE_FLAG_xxx Sprite drawing flags
 * Sprite drawing flags.
 *@{*/
/** Use darkness. */
#define SPRITE_FLAG_DARK 1
/** Fog of war. */
#define SPRITE_FLAG_FOW 2
/** Red. */
#define SPRITE_FLAG_RED 4
/** Gray. */
#define SPRITE_FLAG_GRAY 8
/*@}*/

/** Sprite structure. */
typedef struct sprite_struct
{
    /** Rows of blank pixels before first color information. */
    int border_up;

    /** Border down. */
    int border_down;

    /** Border left. */
    int border_left;

    /** Border right. */
    int border_right;

    /** The sprite's bitmap. */
    SDL_Surface *bitmap;

    /** Red (infravision). */
    SDL_Surface *red;

    /** Gray (xray). */
    SDL_Surface *grey;

    /** Fog of war. */
    SDL_Surface *fog_of_war;

    /** Overlay effect. */
    SDL_Surface *effect;

    /** Dark levels. */
    SDL_Surface *dark_level[DARK_LEVELS];
} sprite_struct;

/**
 * @defgroup ANIM_xxx Animation types
 * Animation types.
 *@{*/
/** Damage animation. */
#define ANIM_DAMAGE     1
/** Kill animation. */
#define ANIM_KILL       2
/*@}*/

/** Animation structure. */
typedef struct _anim
{
    /** Pointer to next anim in queue. */
    struct _anim *next;

    /** Pointer to anim before. */
    struct _anim *before;

    /** Type of the animation, one of @ref ANIM_xxx. */
    int type;

    /** The time we started this anim. */
    uint32 start_tick;

    /** This is the end-tick. */
    uint32 last_tick;

    /** This is the number to display. */
    int value;

    /** X position. */
    int x;

    /** Y position. */
    int y;

    /** Movement in X per tick. */
    int xoff;

    /** Movement in Y per tick. */
    float yoff;

    /** Map position X. */
    int mapx;

    /** Map position Y. */
    int mapy;
}_anim;

#define BORDER_CREATE_TOP(_surface, _x, _y, _w, _h, _color, _thickness) border_create_line((_surface), (_x), (_y), (_w), (_thickness), (_color))
#define BORDER_CREATE_BOTTOM(_surface, _x, _y, _w, _h, _color, _thickness) border_create_line((_surface), (_x), (_y) + (_h) - (_thickness), (_w), (_thickness), (_color))
#define BORDER_CREATE_LEFT(_surface, _x, _y, _w, _h, _color, _thickness) border_create_line((_surface), (_x), (_y), (_thickness), (_h), (_color))
#define BORDER_CREATE_RIGHT(_surface, _x, _y, _w, _h, _color, _thickness) border_create_line((_surface), (_x) + (_w) - (_thickness), (_y), (_thickness), (_h), (_color))

#endif
