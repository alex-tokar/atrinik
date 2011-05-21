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
 * Handles music player widget code. */

#include <include.h>

/**
 * Handle mouse events for the music player widget.
 * @param widget The widget.
 * @param event The event. */
void widget_mplayer_mevent(widgetdata *widget, SDL_Event *event)
{
	list_struct *list = list_exists(LIST_MPLAYER);

	/* If the list has handled the mouse event, we need to redraw the
	 * widget. */
	if (list && list_handle_mouse(list, event->motion.x - widget->x1, event->motion.y - widget->y1, event))
	{
		widget->redraw = 1;
	}
}

/**
 * Handle music list double-click and "Play" button.
 * @param list The music list. */
static void list_handle_enter(list_struct *list)
{
	sound_start_bg_music(list->text[list->row_selected - 1][0], options.music_volume, -1);
	sound_map_background(1);
}

/**
 * Render the music player widget.
 * @param widget The widget. */
void widget_show_mplayer(widgetdata *widget)
{
	SDL_Rect box, box2;
	list_struct *list;
	const char *bg_music, *cp;

	if (!widget->widgetSF)
	{
		widget->widgetSF = SDL_ConvertSurface(Bitmaps[BITMAP_CONTENT]->bitmap, Bitmaps[BITMAP_CONTENT]->bitmap->format, Bitmaps[BITMAP_CONTENT]->bitmap->flags);
	}

	list = list_exists(LIST_MPLAYER);

	/* The list doesn't exist yet, create it. */
	if (!list)
	{
		DIR *dir;
		struct dirent *currentfile;
		size_t i = 0;

		/* Create the list and set up settings. */
		list = list_create(LIST_MPLAYER, 10, 2, 12, 1, 8);
		list->handle_enter_func = list_handle_enter;
		list->surface = widget->widgetSF;
		list_scrollbar_enable(list);
		list_set_column(list, 0, 130, 7, NULL, -1);
		list_set_font(list, FONT_ARIAL10);

		/* Read the default media directory and add the file names to the list. */
		dir = opendir(DIRECTORY_MEDIA);

		while ((currentfile = readdir(dir)))
		{
			/* Ignore hidden files and files without extension. */
			if (currentfile->d_name[0] == '.' || !strchr(currentfile->d_name, '.'))
			{
				continue;
			}

			list_add(list, i, 0, currentfile->d_name);
			i++;
		}

		closedir(dir);

		/* If we added any, sort the list alphabetically and add an entry
		 * to disable background music. */
		if (i)
		{
			list_sort(list, LIST_SORT_ALPHA);
			list_add(list, i++, 0, "Disable music");
		}
	}

	if (widget->redraw)
	{
		_BLTFX bltfx;

		bltfx.surface = widget->widgetSF;
		bltfx.flags = 0;
		bltfx.alpha = 0;
		sprite_blt(Bitmaps[BITMAP_CONTENT], 0, 0, NULL, &bltfx);

		widget->redraw = 0;

		box.h = 0;
		box.w = widget->wd;
		string_blt(widget->widgetSF, FONT_SERIF12, "Music Player", 0, 3, COLOR_SIMPLE(COLOR_HGOLD), TEXT_ALIGN_CENTER, &box);
		list_show(list);
		box.w /= 2;
		string_blt(widget->widgetSF, FONT_SANS10, "Currently playing:", widget->wd / 2, 22, COLOR_SIMPLE(COLOR_WHITE), TEXT_ALIGN_CENTER, &box);
	}

	box2.x = widget->x1;
	box2.y = widget->y1;
	SDL_BlitSurface(widget->widgetSF, NULL, ScreenSurface, &box2);

	bg_music = sound_get_bg_music();
	box.h = 0;
	box.w = widget->wd / 2;

	/* Strip off the directory path, if any. */
	if (bg_music && (cp = strrchr(bg_music, '/')))
	{
		bg_music = cp + 1;
	}

	/* Show the music that is being played. */
	string_blt(ScreenSurface, FONT_SANS11, bg_music ? bg_music : "No music", widget->x1 + widget->wd / 2, widget->y1 + 34, COLOR_SIMPLE(COLOR_HGOLD), TEXT_ALIGN_CENTER, &box);

	/* Show Play/Stop button. */
	if (button_show(BITMAP_BUTTON, -1, BITMAP_BUTTON_DOWN, widget->x1 + widget->wd / 4 - Bitmaps[BITMAP_BUTTON]->bitmap->w / 2, widget->y1 + widget->ht - Bitmaps[BITMAP_BUTTON]->bitmap->h - 4, sound_map_background(-1) ? "Stop" : "Play", FONT_ARIAL10, COLOR_SIMPLE(COLOR_WHITE), COLOR_SIMPLE(COLOR_BLACK), COLOR_SIMPLE(COLOR_HGOLD), COLOR_SIMPLE(COLOR_BLACK)))
	{
		if (sound_map_background(-1))
		{
			sound_start_bg_music("no_music", 0, 0);
			sound_map_background(0);
		}
		else
		{
			list_handle_enter(list);
		}
	}

	/* Show close button. */
	if (button_show(BITMAP_BUTTON_ROUND, -1, BITMAP_BUTTON_ROUND_DOWN, widget->x1 + widget->wd - Bitmaps[BITMAP_BUTTON_ROUND]->bitmap->w - 4, widget->y1 + 4, "X", FONT_ARIAL10, COLOR_SIMPLE(COLOR_WHITE), COLOR_SIMPLE(COLOR_BLACK), COLOR_SIMPLE(COLOR_HGOLD), COLOR_SIMPLE(COLOR_BLACK)))
	{
		widget->show = 0;
	}
}
