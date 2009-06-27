/************************************************************************
*            Atrinik, a Multiplayer Online Role Playing Game            *
*                                                                       *
*                     Copyright (C) 2009 Alex Tokar                     *
*                                                                       *
* Fork from Daimonin (Massive Multiplayer Online Role Playing Game)     *
* and Crossfire (Multiplayer game for X-windows).                       *
*                                                                       *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 3 of the License, or     *
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

#if !defined(__MENU_H)
#define __MENU_H

#define MENU_NO       1
#define MENU_KEYBIND  2
#define MENU_SPELL    4
#define MENU_SKILL    8
#define MENU_OPTION   16
#define MENU_CREATE   32
#define MENU_BOOK     64
#define MENU_PARTY    128

#define MENU_ALL (MENU_NO & MENU_KEYBIND & MENU_SPELL & MENU_OPTION & MENU_BOOK & MENU_PARTY)

#define MENU_SOUND_VOL 40
struct _skill_list skill_list[SKILL_LIST_MAX];
extern _dialog_list_set skill_list_set;

/* Skill list entries */
struct _spell_list spell_list[SPELL_LIST_MAX];
extern _dialog_list_set spell_list_set;

extern _dialog_list_set option_list_set;

struct _bindkey_list bindkey_list[BINDKEY_LIST_MAX];
extern _dialog_list_set bindkey_list_set;

extern _dialog_list_set create_list_set;
extern int keybind_status;

#define MAX_QUICK_SLOTS 8
typedef struct _quickslot
{
	/* Do we have an item or a spell in quickslot */
	int spell;

	int invSlot;
	int nr;

	/* What item/spellNr in quickslot */
	int tag;

	int spellNr;
	int groupNr;
	int classNr;
}_quickslot;

extern _quickslot quick_slots[MAX_QUICK_SLOTS];

typedef struct _media_file
{
	/* File name */
	char name[256];

	/* Data buffer */
	void *data;

	/* What is this? (What is loaded in buffer) */
	int type;

	/* Parameter 1 */
	int p1;

	/* Parameter 2 */
	int p2;
}_media_file;


typedef enum _media_type
{
	MEDIA_TYPE_NO,
	MEDIA_TYPE_PNG
}_media_type;

#define MEDIA_MAX 10
#define MEDIA_SHOW_NO -1

extern _media_file media_file[MEDIA_MAX];

/* Buffered media files */
extern int media_count;
extern int media_show;
extern int media_show_update;

extern void do_console();
extern void do_number();
extern void show_number(int x, int y);
extern void show_console(int x, int y);
extern void widget_show_resist(int x, int y);
extern void show_keybind(void);
extern void show_spelllist(void);
extern void show_skilllist(void);
extern void show_help(char *helpfile);

extern void show_menu(void);
extern void show_media(int x, int y);
extern void show_range(int x, int y);
extern int init_media_tag(char *tag);
extern void blt_inventory_face_from_tag(int tag, int x, int y);
extern int blt_window_slider(_Sprite *slider, int max_win, int winlen, int off, int len, int x, int y);
extern void do_keybind_input(void);

extern int read_anim_tmp(void);
extern int read_bmap_tmp(void);
extern void read_anims(void);
extern void read_bmaps_p0(void);
extern void delete_bmap_tmp(void);
extern void read_bmaps(void);
extern void delete_server_chars(void);
extern void load_settings(void);
extern void read_settings(void);
extern void read_spells(void);
extern void read_skills(void);
extern void read_help_files(void);
extern int blt_face_centered(int face, int x, int y);
extern int get_quickslot(int x, int y);
extern void show_quickslots(int x, int y);
extern void update_quickslots(int del_item);
extern void load_quickslots_entrys();
extern void save_quickslots_entrys();

extern void widget_quickslots_mouse_event(int x, int y, int MEvent);
extern void widget_range_event(int x, int y, SDL_Event event, int MEvent);
extern void widget_event_target(int x, int y);
extern void widget_number_event(int x, int y);
extern void widget_quickslots(int x, int y);
extern void widget_show_range(int x, int y);
extern void widget_show_target(int x, int y);
extern void widget_show_mapname(int x, int y);
extern void widget_show_console(int x, int y);
extern void widget_show_number(int x, int y);

extern int client_command_check(char *cmd);

#endif
