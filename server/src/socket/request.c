/************************************************************************
*            Atrinik, a Multiplayer Online Role Playing Game            *
*                                                                       *
*    Copyright (C) 2009-2012 Alex Tokar and Atrinik Development Team    *
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
 * This file implements all of the goo on the server side for handling
 * clients.  It's got a bunch of global variables for keeping track of
 * each of the clients.
 *
 * @note All functions that are used to process data from the client
 * have the prototype of (char *data, int datalen, int client_num).  This
 * way, we can use one dispatch table. */

#include <global.h>

#define GET_CLIENT_FLAGS(_O_)	((_O_)->flags[0] & 0x7f)
#define NO_FACE_SEND (-1)

void socket_command_setup(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	packet_struct *packet;
	uint8 type;

	packet = packet_new(CLIENT_CMD_SETUP, 256, 256);

	while (pos < len)
	{
		type = packet_to_uint8(data, len, &pos);
		packet_append_uint8(packet, type);

		if (type == CMD_SETUP_SOUND)
		{
			ns->sound = packet_to_uint8(data, len, &pos);
			packet_append_uint8(packet, ns->sound);
		}
		else if (type == CMD_SETUP_MAPSIZE)
		{
			int x, y;

			x = packet_to_uint8(data, len, &pos);
			y = packet_to_uint8(data, len, &pos);

			if (x < 9 || y < 9 || x > MAP_CLIENT_X || y > MAP_CLIENT_Y)
			{
				x = MAP_CLIENT_X;
				y = MAP_CLIENT_Y;
			}

			ns->mapx = x;
			ns->mapy = y;
			ns->mapx_2 = x / 2;
			ns->mapy_2 = y / 2;

			packet_append_uint8(packet, x);
			packet_append_uint8(packet, y);
		}
		else if (type == CMD_SETUP_BOT)
		{
			ns->is_bot = packet_to_uint8(data, len, &pos);

			if (ns->is_bot != 0 && ns->is_bot != 1)
			{
				ns->is_bot = 0;
			}

			packet_append_uint8(packet, ns->is_bot);
		}
		else if (type == CMD_SETUP_SERVER_FILE)
		{
			uint8 file_type;
			uint64 len_ucomp, crc;

			file_type = packet_to_uint8(data, len, &pos);
			file_type = MAX(0, MIN(SERVER_FILES_MAX - 1, file_type));

			len_ucomp = packet_to_uint64(data, len, &pos);
			crc = packet_to_uint64(data, len, &pos);

			packet_append_uint8(packet, file_type);

			if (SrvClientFiles[file_type].len_ucomp != len_ucomp || SrvClientFiles[file_type].crc != crc)
			{
				packet_append_uint8(packet, 1);
			}
			else
			{
				packet_append_uint8(packet, 0);
			}
		}
	}

	socket_send_packet(ns, packet);
}

/**
 * The client has requested to be added to the game. This is what takes
 * care of it.
 *
 * We tell the client how things worked out. */
void socket_command_addme(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	if (ns->status != Ns_Add || add_player(ns))
	{
		ns->status = Ns_Dead;
	}
	else
	{
		/* Basically, the add_player copies the socket structure into
		 * the player structure, so this one (which is from init_sockets)
		 * is not needed anymore. */
		ns->addme = 1;
		/* Reset idle counter */
		ns->login_count = 0;
		ns->keepalive = 0;
		socket_info.nconns--;
		ns->status = Ns_Avail;
	}
}

void socket_command_player_cmd(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	char command[MAX_BUF];

	if (pl->state != ST_PLAYING)
	{
		return;
	}

	packet_to_string(data, len, &pos, command, sizeof(command));
	commands_handle(pl->ob, command);
}

/**
 * Receive a player name, and force the first letter to be uppercase.
 * @param op Object. */
void receive_player_name(object *op)
{
	player_cleanup_name(CONTR(op)->write_buf + 1);

	if (!check_name(CONTR(op), CONTR(op)->write_buf + 1))
	{
		get_name(op);
		return;
	}

	FREE_AND_COPY_HASH(op->name, CONTR(op)->write_buf + 1);

	get_password(op);
}

/**
 * Receive player password.
 * @param op Object. */
void receive_player_password(object *op)
{
	unsigned int pwd_len = strlen(CONTR(op)->write_buf + 1);

	if (pwd_len < PLAYER_PASSWORD_MIN || pwd_len > PLAYER_PASSWORD_MAX)
	{
		draw_info_send(0, COLOR_RED, &CONTR(op)->socket, "That password has an invalid length.");
		get_name(op);
		return;
	}

	if (CONTR(op)->state == ST_CONFIRM_PASSWORD)
	{
		packet_struct *packet;

		if (!check_password(CONTR(op)->write_buf + 1, CONTR(op)->password))
		{
			draw_info_send(0, COLOR_RED, &CONTR(op)->socket, "That password has an invalid length.");
			get_name(op);
			return;
		}

		packet = packet_new(CLIENT_CMD_NEW_CHAR, 0, 0);
		socket_send_packet(&CONTR(op)->socket, packet);
		CONTR(op)->state = ST_ROLL_STAT;

		return;
	}

	strcpy(CONTR(op)->password, crypt_string(CONTR(op)->write_buf + 1, NULL));
	CONTR(op)->state = ST_ROLL_STAT;
	check_login(op);
	return;
}

void socket_command_reply(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	char buf[MAX_BUF];

	packet_to_string(data, len, &pos, buf, sizeof(buf));

	strcpy(pl->write_buf, ":");
	strncat(pl->write_buf, buf, 250);
	pl->write_buf[250] = '\0';

	pl->socket.ext_title_flag = 1;

	switch (pl->state)
	{
		case ST_PLAYING:
			pl->socket.status = Ns_Dead;
			logger_print(LOG(BUG), "Got reply message with ST_PLAYING input state (player %s)", query_name(pl->ob, NULL));
			break;

		case ST_GET_NAME:
			receive_player_name(pl->ob);
			break;

		case ST_GET_PASSWORD:
		case ST_CONFIRM_PASSWORD:
			receive_player_password(pl->ob);
			break;

		default:
			pl->socket.status = Ns_Dead;
			logger_print(LOG(BUG), "Unknown input state: %d", pl->state);
			break;
	}
}

void socket_command_request_file(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	uint8 file_type;
	packet_struct *packet;

	if (ns->status != Ns_Add)
	{
		return;
	}

	file_type = packet_to_uint8(data, len, &pos);
	file_type = MIN(SERVER_FILES_MAX - 1, file_type);

	if (ns->requested_file[file_type])
	{
		return;
	}

	ns->requested_file[file_type] = 1;

	packet = packet_new(CLIENT_CMD_DATA, 1 + 4 + SrvClientFiles[file_type].len, 0);
	packet_append_uint8(packet, file_type);
	packet_append_uint32(packet, SrvClientFiles[file_type].len_ucomp);
	packet_append_data_len(packet, SrvClientFiles[file_type].file, SrvClientFiles[file_type].len);
	socket_send_packet(ns, packet);
}

void socket_command_version(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	uint32 ver;

	ver = packet_to_uint32(data, len, &pos);

	if (ver == 0 || ver == 991017 || ver == 1055)
	{
		draw_info_send(0, COLOR_RED, ns, "Your client is outdated!\nGo to http://www.atrinik.org/ and download the latest Atrinik client.");
		ns->status = Ns_Zombie;
		return;
	}

	ns->socket_version = ver;
}

void socket_command_item_move(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	tag_t to, tag;
	uint32 nrof;

	to = packet_to_uint32(data, len, &pos);
	tag = packet_to_uint32(data, len, &pos);
	nrof = packet_to_uint32(data, len, &pos);

	if (!tag)
	{
		return;
	}

	esrv_move_object(pl->ob, to, tag, nrof);
}

/**
 * Asks the client to query the user.
 *
 * This way, the client knows it needs to send something back (vs just
 * printing out a message). */
void send_query(socket_struct *ns, uint8 type)
{
	packet_struct *packet;

	packet = packet_new(CLIENT_CMD_QUERY, 4, 0);
	packet_append_uint8(packet, type);
	socket_send_packet(ns, packet);
}

#define AddIfInt(_old, _new, _type, _bitsize) \
	if ((_old) != (_new)) \
	{ \
		(_old) = (_new); \
		packet_append_uint8(packet, (_type)); \
		packet_append_##_bitsize(packet, (_new)); \
	}

#define AddIfFloat(_old, _new, _type) \
	if ((_old) != (_new)) \
	{ \
		(_old) = (_new); \
		packet_append_uint8(packet, (_type)); \
		packet_append_uint32(packet, (_new) * FLOAT_MULTI); \
	}

/**
 * Sends player statistics update.
 *
 * We look at the old values, and only send what has changed.
 *
 * Stat mapping values are in newclient.h */
void esrv_update_stats(player *pl)
{
	packet_struct *packet;
	int i;
	uint16 flags;

	packet = packet_new(CLIENT_CMD_STATS, 32, 256);

	if (pl->target_object && pl->target_object != pl->ob)
	{
		char hp;

		hp = MAX(1, (((float) pl->target_object->stats.hp / (float) pl->target_object->stats.maxhp) * 100.0f));

		AddIfInt(pl->target_hp, hp, CS_STAT_TARGET_HP, uint8);
	}

	AddIfInt(pl->last_gen_hp, pl->gen_client_hp, CS_STAT_REG_HP, uint16);
	AddIfInt(pl->last_gen_sp, pl->gen_client_sp, CS_STAT_REG_MANA, uint16);
	AddIfInt(pl->last_level, pl->ob->level, CS_STAT_LEVEL, uint8);
	AddIfFloat(pl->last_speed, pl->ob->speed, CS_STAT_SPEED);
	AddIfInt(pl->last_weight_limit, weight_limit[pl->ob->stats.Str], CS_STAT_WEIGHT_LIM, uint32);
	AddIfInt(pl->last_action_timer, pl->action_timer, CS_STAT_ACTION_TIME, uint32);

	if (pl->ob)
	{
		object *arrow;

		AddIfInt(pl->last_stats.hp, pl->ob->stats.hp, CS_STAT_HP, uint32);
		AddIfInt(pl->last_stats.maxhp, pl->ob->stats.maxhp, CS_STAT_MAXHP, uint32);
		AddIfInt(pl->last_stats.sp, pl->ob->stats.sp, CS_STAT_SP, uint16);
		AddIfInt(pl->last_stats.maxsp, pl->ob->stats.maxsp, CS_STAT_MAXSP, uint16);
		AddIfInt(pl->last_stats.Str, pl->ob->stats.Str, CS_STAT_STR, uint8);
		AddIfInt(pl->last_stats.Int, pl->ob->stats.Int, CS_STAT_INT, uint8);
		AddIfInt(pl->last_stats.Pow, pl->ob->stats.Pow, CS_STAT_POW, uint8);
		AddIfInt(pl->last_stats.Wis, pl->ob->stats.Wis, CS_STAT_WIS, uint8);
		AddIfInt(pl->last_stats.Dex, pl->ob->stats.Dex, CS_STAT_DEX, uint8);
		AddIfInt(pl->last_stats.Con, pl->ob->stats.Con, CS_STAT_CON, uint8);
		AddIfInt(pl->last_stats.Cha, pl->ob->stats.Cha, CS_STAT_CHA, uint8);
		AddIfInt(pl->last_stats.exp, pl->ob->stats.exp, CS_STAT_EXP, uint64);
		AddIfInt(pl->last_stats.wc, pl->ob->stats.wc, CS_STAT_WC, uint16);
		AddIfInt(pl->last_stats.ac, pl->ob->stats.ac, CS_STAT_AC, uint16);
		AddIfInt(pl->last_stats.dam, pl->ob->stats.dam, CS_STAT_DAM, uint16);
		AddIfInt(pl->last_stats.food, pl->ob->stats.food, CS_STAT_FOOD, uint16);
		AddIfInt(pl->last_path_attuned, pl->ob->path_attuned, CS_STAT_PATH_ATTUNED, uint32);
		AddIfInt(pl->last_path_repelled, pl->ob->path_repelled, CS_STAT_PATH_REPELLED, uint32);
		AddIfInt(pl->last_path_denied, pl->ob->path_denied, CS_STAT_PATH_DENIED, uint32);

		if (pl->equipment[PLAYER_EQUIP_WEAPON] && pl->equipment[PLAYER_EQUIP_WEAPON]->type == BOW && (arrow = arrow_find(pl->ob, pl->equipment[PLAYER_EQUIP_WEAPON]->race)))
		{
			AddIfInt(pl->last_ranged_dam, arrow_get_damage(pl->ob, pl->equipment[PLAYER_EQUIP_WEAPON], arrow), CS_STAT_RANGED_DAM, uint16);
			AddIfInt(pl->last_ranged_wc, arrow_get_wc(pl->ob, pl->equipment[PLAYER_EQUIP_WEAPON], arrow), CS_STAT_RANGED_WC, uint16);
			AddIfInt(pl->last_ranged_ws, bow_get_ws(pl->equipment[PLAYER_EQUIP_WEAPON], arrow), CS_STAT_RANGED_WS, uint32);
		}
		else
		{
			AddIfInt(pl->last_ranged_dam, 0, CS_STAT_RANGED_DAM, uint16);
			AddIfInt(pl->last_ranged_wc, 0, CS_STAT_RANGED_WC, uint16);
			AddIfInt(pl->last_ranged_ws, 0, CS_STAT_RANGED_WS, uint32);
		}
	}

	flags = 0;

	if (pl->run_on)
	{
		flags |= SF_RUNON;
	}

	/* we add additional player status flags - in old style, you got a msg
	 * in the text windows when you get xray of get blinded - we will skip
	 * this and add the info here, so the client can track it down and make
	 * it the user visible in it own, server independent way. */

	/* player is blind */
	if (QUERY_FLAG(pl->ob, FLAG_BLIND))
	{
		flags |= SF_BLIND;
	}

	/* player has xray */
	if (QUERY_FLAG(pl->ob, FLAG_XRAYS))
	{
		flags |= SF_XRAYS;
	}

	/* player has infravision */
	if (QUERY_FLAG(pl->ob, FLAG_SEE_IN_DARK))
	{
		flags |= SF_INFRAVISION;
	}

	AddIfInt(pl->last_flags, flags, CS_STAT_FLAGS, uint16);

	for (i = 0; i < NROFATTACKS; i++)
	{
		/* If there are more attacks, but we reached CS_STAT_PROT_END,
		 * we stop now. */
		if (CS_STAT_PROT_START + i > CS_STAT_PROT_END)
		{
			break;
		}

		AddIfInt(pl->last_protection[i], pl->ob->protection[i], CS_STAT_PROT_START + i, sint8);
	}

	if (pl->socket.ext_title_flag)
	{
		generate_ext_title(pl);
		packet_append_uint8(packet, CS_STAT_EXT_TITLE);
		packet_append_string_terminated(packet, pl->ext_title);
		pl->socket.ext_title_flag = 0;
	}

	AddIfInt(pl->last_gender, object_get_gender(pl->ob), CS_STAT_GENDER, uint8);

	if (packet->len >= 1)
	{
		socket_send_packet(&pl->socket, packet);
	}
	else
	{
		packet_free(packet);
	}
}

/**
 * Tells the client that here is a player it should start using. */
void esrv_new_player(player *pl, uint32 weight)
{
	packet_struct *packet;

	packet = packet_new(CLIENT_CMD_PLAYER, 12, 0);
	packet_append_uint32(packet, pl->ob->count);
	packet_append_uint32(packet, weight);
	packet_append_uint32(packet, pl->ob->face->number);
	socket_send_packet(&pl->socket, packet);
}

/**
 * Get ID of a tiled map by checking player::last_update.
 * @param pl Player.
 * @param map Tiled map.
 * @return ID of the tiled map, 0 if there is no match. */
static inline int get_tiled_map_id(player *pl, struct mapdef *map)
{
	int i;

	if (!pl->last_update)
	{
		return 0;
	}

	for (i = 0; i < TILED_NUM; i++)
	{
		if (pl->last_update->tile_path[i] == map->path)
		{
			return i + 1;
		}
	}

	return 0;
}

/**
 * Copy socket's last map according to new coordinates.
 * @param ns Socket.
 * @param dx X.
 * @param dy Y. */
static inline void copy_lastmap(socket_struct *ns, int dx, int dy)
{
	struct Map newmap;
	int x, y;

	for (x = 0; x < ns->mapx; x++)
	{
		for (y = 0; y < ns->mapy; y++)
		{
			if (x + dx < 0 || x + dx >= ns->mapx || y + dy < 0 || y + dy >= ns->mapy)
			{
				memset(&(newmap.cells[x][y]), 0, sizeof(MapCell));
				continue;
			}

			memcpy(&(newmap.cells[x][y]), &(ns->lastmap.cells[x + dx][y + dy]), sizeof(MapCell));
		}
	}

	memcpy(&(ns->lastmap), &newmap, sizeof(struct Map));
}

/**
 * Do some checks, map name and LOS and then draw the map.
 * @param pl Whom to draw map for. */
void draw_client_map(object *pl)
{
	int redraw_below = 0;

	if (pl->type != PLAYER)
	{
		logger_print(LOG(BUG), "Called with non-player: %s", pl->name);
		return;
	}

	/* IF player is just joining the game, he isn't on a map,
	 * If so, don't try to send them a map.  All will
	 * be OK once they really log in. */
	if (!pl->map || pl->map->in_memory != MAP_IN_MEMORY)
	{
		return;
	}

	CONTR(pl)->map_update_cmd = MAP_UPDATE_CMD_SAME;

	/* If we changed somewhere the map, prepare map data */
	if (CONTR(pl)->last_update != pl->map)
	{
		int tile_map = get_tiled_map_id(CONTR(pl), pl->map);

		/* Are we on a new map? */
		if (!CONTR(pl)->last_update || !tile_map)
		{
			CONTR(pl)->map_update_cmd = MAP_UPDATE_CMD_NEW;
			memset(&(CONTR(pl)->socket.lastmap), 0, sizeof(struct Map));
			CONTR(pl)->last_update = pl->map;
			redraw_below = 1;
		}
		else
		{
			CONTR(pl)->map_update_cmd = MAP_UPDATE_CMD_CONNECTED;
			CONTR(pl)->map_update_tile = tile_map;
			redraw_below = 1;

			/* We have moved to a tiled map. Let's calculate the offsets. */
			switch (tile_map - 1)
			{
				case 0:
					CONTR(pl)->map_off_x = pl->x - CONTR(pl)->map_tile_x;
					CONTR(pl)->map_off_y = -(CONTR(pl)->map_tile_y + (MAP_HEIGHT(pl->map) - pl->y));
					break;

				case 1:
					CONTR(pl)->map_off_y = pl->y - CONTR(pl)->map_tile_y;
					CONTR(pl)->map_off_x = (MAP_WIDTH(pl->map) - CONTR(pl)->map_tile_x) + pl->x;
					break;

				case 2:
					CONTR(pl)->map_off_x = pl->x - CONTR(pl)->map_tile_x;
					CONTR(pl)->map_off_y = (MAP_HEIGHT(pl->map) - CONTR(pl)->map_tile_y) + pl->y;
					break;

				case 3:
					CONTR(pl)->map_off_y = pl->y - CONTR(pl)->map_tile_y;
					CONTR(pl)->map_off_x = -(CONTR(pl)->map_tile_x + (MAP_WIDTH(pl->map) - pl->x));
					break;

				case 4:
					CONTR(pl)->map_off_y = -(CONTR(pl)->map_tile_y + (MAP_HEIGHT(pl->map) - pl->y));
					CONTR(pl)->map_off_x = (MAP_WIDTH(pl->map) - CONTR(pl)->map_tile_x) + pl->x;
					break;

				case 5:
					CONTR(pl)->map_off_x = (MAP_WIDTH(pl->map) - CONTR(pl)->map_tile_x) + pl->x;
					CONTR(pl)->map_off_y = (MAP_HEIGHT(pl->map) - CONTR(pl)->map_tile_y) + pl->y;
					break;

				case 6:
					CONTR(pl)->map_off_y = (MAP_HEIGHT(pl->map) - CONTR(pl)->map_tile_y) + pl->y;
					CONTR(pl)->map_off_x = -(CONTR(pl)->map_tile_x + (MAP_WIDTH(pl->map) - pl->x));
					break;

				case 7:
					CONTR(pl)->map_off_x = -(CONTR(pl)->map_tile_x + (MAP_WIDTH(pl->map) - pl->x));
					CONTR(pl)->map_off_y = -(CONTR(pl)->map_tile_y + (MAP_HEIGHT(pl->map) - pl->y));
					break;
			}

			copy_lastmap(&CONTR(pl)->socket, CONTR(pl)->map_off_x, CONTR(pl)->map_off_y);
			CONTR(pl)->last_update = pl->map;
		}
	}
	else
	{
		if (CONTR(pl)->map_tile_x != pl->x || CONTR(pl)->map_tile_y != pl->y)
		{
			copy_lastmap(&CONTR(pl)->socket, pl->x - CONTR(pl)->map_tile_x, pl->y - CONTR(pl)->map_tile_y);
			redraw_below = 1;
		}
	}

	/* Redraw below window and backbuffer new positions? */
	if (redraw_below)
	{
		/* Backbuffer position so we can determine whether we have moved or not */
		CONTR(pl)->map_tile_x = pl->x;
		CONTR(pl)->map_tile_y = pl->y;
		CONTR(pl)->socket.below_clear = 1;
		/* Redraw it */
		CONTR(pl)->socket.update_tile = 0;
		CONTR(pl)->socket.look_position = 0;
	}

	/* Do LOS after calls to update_position */
	if (CONTR(pl)->update_los)
	{
		update_los(pl);
		CONTR(pl)->update_los = 0;
	}

	draw_client_map2(pl);

	/* If we moved on the same map, check for map name/music to update. */
	if (redraw_below && CONTR(pl)->map_update_cmd == MAP_UPDATE_CMD_SAME)
	{
		MapSpace *msp;
		packet_struct *packet;
		object *map_info;

		msp = GET_MAP_SPACE_PTR(pl->map, pl->x, pl->y);
		map_info = msp->map_info && OBJECT_VALID(msp->map_info, msp->map_info_count) ? msp->map_info : NULL;

		packet = packet_new(CLIENT_CMD_MAPSTATS, 256, 256);

		if ((map_info && map_info->race && strcmp(map_info->race, CONTR(pl)->map_info_name) != 0) || (!map_info && CONTR(pl)->map_info_name[0] != '\0'))
		{
			packet_append_uint8(packet, CMD_MAPSTATS_NAME);
			packet_append_map_name(packet, pl, map_info);

			if (map_info)
			{
				strncpy(CONTR(pl)->map_info_name, map_info->race, sizeof(CONTR(pl)->map_info_name) - 1);
				CONTR(pl)->map_info_name[sizeof(CONTR(pl)->map_info_name) - 1] = '\0';
			}
			else
			{
				CONTR(pl)->map_info_name[0] = '\0';
			}
		}

		if ((map_info && map_info->slaying && strcmp(map_info->slaying, CONTR(pl)->map_info_music) != 0) || (!map_info && CONTR(pl)->map_info_music[0] != '\0'))
		{
			packet_append_uint8(packet, CMD_MAPSTATS_MUSIC);
			packet_append_map_music(packet, pl, map_info);

			if (map_info)
			{
				strncpy(CONTR(pl)->map_info_music, map_info->slaying, sizeof(CONTR(pl)->map_info_music) - 1);
				CONTR(pl)->map_info_music[sizeof(CONTR(pl)->map_info_music) - 1] = '\0';
			}
			else
			{
				CONTR(pl)->map_info_music[0] = '\0';
			}
		}

		if ((map_info && map_info->title && strcmp(map_info->title, CONTR(pl)->map_info_weather) != 0) || (!map_info && CONTR(pl)->map_info_weather[0] != '\0'))
		{
			packet_append_uint8(packet, CMD_MAPSTATS_WEATHER);
			packet_append_map_weather(packet, pl, map_info);

			if (map_info)
			{
				strncpy(CONTR(pl)->map_info_weather, map_info->title, sizeof(CONTR(pl)->map_info_weather) - 1);
				CONTR(pl)->map_info_weather[sizeof(CONTR(pl)->map_info_weather) - 1] = '\0';
			}
			else
			{
				CONTR(pl)->map_info_weather[0] = '\0';
			}
		}

		/* Anything to send? */
		if (packet->len >= 1)
		{
			socket_send_packet(&CONTR(pl)->socket, packet);
		}
		else
		{
			packet_free(packet);
		}
	}
}

/**
 * Figure out player name color for the client to show, in HTML notation.
 *
 * As you can see in this function, it is easy to add new player name
 * colors, just check for the match and make it return the correct color.
 * @param pl Player object that will get the map data sent to.
 * @param op Player object on the map, to get the name from.
 * @return The color. */
static const char *get_playername_color(object *pl, object *op)
{
	if (CONTR(pl)->party != NULL && CONTR(op)->party != NULL && CONTR(pl)->party == CONTR(op)->party)
	{
		return COLOR_GREEN;
	}
	else if (pl != op && pvp_area(pl, op))
	{
		return COLOR_RED;
	}

	return COLOR_WHITE;
}

void packet_append_map_name(packet_struct *packet, object *op, object *map_info)
{
	packet_append_string(packet, "<b><o=0,0,0>");
	packet_append_string(packet, map_info && map_info->race ? map_info->race : op->map->name);
	packet_append_string_terminated(packet, "</o></b>");
}

void packet_append_map_music(packet_struct *packet, object *op, object *map_info)
{
	packet_append_string_terminated(packet, map_info && map_info->slaying ? map_info->slaying : (op->map->bg_music ? op->map->bg_music : "no_music"));
}

void packet_append_map_weather(packet_struct *packet, object *op, object *map_info)
{
	packet_append_string_terminated(packet, map_info && map_info->title ? map_info->title : (op->map->weather ? op->map->weather : "none"));
}

/** Darkness table */
static int darkness_table[] = {0, 10, 30, 60, 120, 260, 480, 960};

/** Clear a map cell. */
#define map_clearcell(_cell_) \
{ \
	memset((void *) ((char *) (_cell_) + offsetof(MapCell, count)), 0, sizeof(MapCell) - offsetof(MapCell, count)); \
	(_cell_)->count = -1; \
}

/** Clear a map cell, but only if it has not been cleared before. */
#define map_if_clearcell() \
{ \
	if (CONTR(pl)->socket.lastmap.cells[ax][ay].count != -1) \
	{ \
		packet_append_uint16(packet, mask | MAP2_MASK_CLEAR); \
		map_clearcell(&CONTR(pl)->socket.lastmap.cells[ax][ay]); \
	} \
}

/** Draw the client map. */
void draw_client_map2(object *pl)
{
	static uint32 map2_count = 0;
	MapCell *mp;
	MapSpace *msp;
	mapstruct *m;
	int x, y, ax, ay, d, nx, ny;
	int x_start, light_adjust;
	int special_vision;
	uint16 mask;
	int wdark;
	int layer, dark;
	int anim_value, anim_type, ext_flags;
	int num_layers;
	int outdoor;
	object *mirror = NULL;
	uint8 have_sound_ambient;
	packet_struct *packet, *packet_layer, *packet_sound;
	size_t oldpos;

	light_adjust = CONTR(pl)->tli ? global_darkness_table[MAX_DARKNESS] : 0;
	wdark = darkness_table[world_darkness];
	/* Any kind of special vision? */
	special_vision = (QUERY_FLAG(pl, FLAG_XRAYS) ? 1 : 0) | (QUERY_FLAG(pl, FLAG_SEE_IN_DARK) ? 2 : 0);
	map2_count++;

	packet = packet_new(CLIENT_CMD_MAP, 0, 512);
	packet_sound = packet_new(CLIENT_CMD_SOUND_AMBIENT, 0, 256);

	packet_enable_ndelay(packet);
	packet_append_uint8(packet, CONTR(pl)->map_update_cmd);

	if (CONTR(pl)->map_update_cmd != MAP_UPDATE_CMD_SAME)
	{
		object *map_info;

		msp = GET_MAP_SPACE_PTR(pl->map, pl->x, pl->y);
		map_info = msp->map_info && OBJECT_VALID(msp->map_info, msp->map_info_count) ? msp->map_info : NULL;

		packet_append_map_name(packet, pl, map_info);
		packet_append_map_music(packet, pl, map_info);
		packet_append_map_weather(packet, pl, map_info);

		if (map_info)
		{
			if (map_info->race)
			{
				strncpy(CONTR(pl)->map_info_name, map_info->race, sizeof(CONTR(pl)->map_info_name) - 1);
				CONTR(pl)->map_info_name[sizeof(CONTR(pl)->map_info_name) - 1] = '\0';
			}

			if (map_info->slaying)
			{
				strncpy(CONTR(pl)->map_info_music, map_info->slaying, sizeof(CONTR(pl)->map_info_music) - 1);
				CONTR(pl)->map_info_music[sizeof(CONTR(pl)->map_info_music) - 1] = '\0';
			}

			if (map_info->title)
			{
				strncpy(CONTR(pl)->map_info_weather, map_info->title, sizeof(CONTR(pl)->map_info_weather) - 1);
				CONTR(pl)->map_info_weather[sizeof(CONTR(pl)->map_info_weather) - 1] = '\0';
			}
		}

		if (CONTR(pl)->map_update_cmd == MAP_UPDATE_CMD_CONNECTED)
		{
			packet_append_uint8(packet, CONTR(pl)->map_update_tile);
			packet_append_sint8(packet, CONTR(pl)->map_off_x);
			packet_append_sint8(packet, CONTR(pl)->map_off_y);
		}
		else
		{
			packet_append_uint8(packet, pl->map->width);
			packet_append_uint8(packet, pl->map->height);
		}
	}

	packet_append_uint8(packet, pl->x);
	packet_append_uint8(packet, pl->y);

	x_start = (pl->x + (CONTR(pl)->socket.mapx + 1) / 2) - 1;

	for (ay = CONTR(pl)->socket.mapy - 1, y = (pl->y + (CONTR(pl)->socket.mapy + 1) / 2) - 1; y >= pl->y - CONTR(pl)->socket.mapy_2; y--, ay--)
	{
		ax = CONTR(pl)->socket.mapx - 1;

		for (x = x_start; x >= pl->x - CONTR(pl)->socket.mapx_2; x--, ax--)
		{
			d = CONTR(pl)->blocked_los[ax][ay];
			/* Form the data packet for x and y positions. */
			mask = (ax & 0x1f) << 11 | (ay & 0x1f) << 6;
			mp = &(CONTR(pl)->socket.lastmap.cells[ax][ay]);

			/* Space is out of map or blocked. Update space and clear values if needed. */
			if (d & BLOCKED_LOS_OUT_OF_MAP)
			{
				map_if_clearcell();
				continue;
			}

			nx = x;
			ny = y;

			if (!(m = get_map_from_coord(pl->map, &nx, &ny)))
			{
				map_if_clearcell();
				continue;
			}

			msp = GET_MAP_SPACE_PTR(m, nx, ny);
			/* Check whether there is ambient sound effect on this tile. */
			have_sound_ambient = msp->sound_ambient && OBJECT_VALID(msp->sound_ambient, msp->sound_ambient_count);

			/* If there is an ambient sound effect but it cannot be heard
			 * through walls due to its configuration, we will pretend
			 * there is no sound effect here. */
			if (have_sound_ambient && !QUERY_FLAG(msp->sound_ambient, FLAG_XRAYS) && d & BLOCKED_LOS_BLOCKED)
			{
				have_sound_ambient = 0;
			}

			/* If there is an ambient sound effect and we haven't sent it
			 * before, or there isn't one but it was sent before, send an
			 * update. */
			if ((have_sound_ambient && mp->sound_ambient_count != msp->sound_ambient->count) || (!have_sound_ambient && mp->sound_ambient_count))
			{
				packet_append_uint8(packet_sound, ax);
				packet_append_uint8(packet_sound, ay);
				packet_append_uint32(packet_sound, mp->sound_ambient_count);

				if (have_sound_ambient)
				{
					packet_append_uint32(packet_sound, msp->sound_ambient->count);
					packet_append_string_terminated(packet_sound, msp->sound_ambient->race);
					packet_append_uint8(packet_sound, msp->sound_ambient->item_condition);
					packet_append_uint8(packet_sound, msp->sound_ambient->item_level);

					mp->sound_ambient_count = msp->sound_ambient->count;
				}
				else
				{
					packet_append_uint32(packet_sound, 0);

					mp->sound_ambient_count = 0;
				}
			}

			if (d & BLOCKED_LOS_BLOCKED)
			{
				map_if_clearcell();
				continue;
			}

			/* Border tile, we can ignore every LOS change */
			if (!(d & BLOCKED_LOS_IGNORE))
			{
				/* Tile has blocksview set? */
				if (msp->flags & P_BLOCKSVIEW)
				{
					if (!d)
					{
						CONTR(pl)->update_los = 1;
					}
				}
				else
				{
					if (d & BLOCKED_LOS_BLOCKSVIEW)
					{
						CONTR(pl)->update_los = 1;
					}
				}
			}

			outdoor = MAP_OUTDOORS(m) || (msp->map_info && OBJECT_VALID(msp->map_info, msp->map_info_count) && msp->map_info->item_power == -2);

			/* Calculate the darkness/light value for this tile. */
			if (((outdoor && !(GET_MAP_FLAGS(m, nx, ny) & P_OUTDOOR)) || (!outdoor && GET_MAP_FLAGS(m, nx, ny) & P_OUTDOOR)) && (!msp->map_info || !OBJECT_VALID(msp->map_info, msp->map_info_count) || msp->map_info->item_power < 0))
			{
				d = msp->light_value + wdark + light_adjust;
			}
			else
			{
				/* Check if map info object bound to this tile has a darkness. */
				if (msp->map_info && OBJECT_VALID(msp->map_info, msp->map_info_count) && msp->map_info->item_power != -1)
				{
					int dark_value = msp->map_info->item_power;

					if (dark_value < 0 || dark_value > MAX_DARKNESS)
					{
						dark_value = MAX_DARKNESS;
					}

					d = global_darkness_table[dark_value] + msp->light_value + light_adjust;
				}
				else
				{
					d = m->light_value + msp->light_value + light_adjust;
				}
			}

			if (GET_MAP_FLAGS(m, nx, ny) & P_MAGIC_MIRROR)
			{
				object *mirror_tmp;
				magic_mirror_struct *m_data;
				mapstruct *mirror_map;

				/* Try to find the magic mirror, but only search on layer 0. */
				for (mirror_tmp = GET_MAP_OB(m, nx, ny); mirror_tmp && mirror_tmp->layer == LAYER_SYS; mirror_tmp = mirror_tmp->above)
				{
					if (mirror_tmp->type == MAGIC_MIRROR)
					{
						mirror = mirror_tmp;
						break;
					}
				}

				m_data = MMIRROR(mirror);

				if (m_data && (mirror_map = magic_mirror_get_map(mirror)) && !OUT_OF_MAP(mirror_map, m_data->x, m_data->y))
				{
					MapSpace *mirror_msp = GET_MAP_SPACE_PTR(mirror_map, m_data->x, m_data->y);

					if ((MAP_OUTDOORS(mirror_map) && !(GET_MAP_FLAGS(mirror_map, m_data->x, m_data->y) & P_OUTDOOR)) || (!MAP_OUTDOORS(mirror_map) && GET_MAP_FLAGS(mirror_map, m_data->x, m_data->y) & P_OUTDOOR))
					{
						d = mirror_msp->light_value + wdark + light_adjust;
					}
					else
					{
						d = mirror_map->light_value + mirror_msp->light_value + light_adjust;
					}
				}
			}

			/* Tile is not normally visible */
			if (d <= 0)
			{
				/* Xray or infravision? */
				if (special_vision & 1 || (special_vision & 2 && msp->flags & (P_IS_PLAYER | P_IS_MONSTER)))
				{
					d = 100;
				}
				else
				{
					map_if_clearcell();
					continue;
				}
			}

			if (d > 640)
			{
				d = 210;
			}
			else if (d > 320)
			{
				d = 180;
			}
			else if (d > 160)
			{
				d = 150;
			}
			else if (d > 80)
			{
				d = 120;
			}
			else if (d > 40)
			{
				d = 90;
			}
			else if (d > 20)
			{
				d = 60;
			}
			else
			{
				d = 30;
			}

			/* Initialize default values for some variables. */
			dark = NO_FACE_SEND;
			ext_flags = 0;
			oldpos = packet_get_pos(packet);
			anim_type = 0;
			anim_value = 0;

			/* Do we need to send the darkness? */
			if (mp->count != d)
			{
				mask |= MAP2_MASK_DARKNESS;
				dark = d;
				mp->count = d;
			}

			/* Add the mask. Any mask changes should go above this line. */
			packet_append_uint16(packet, mask);

			/* If we have darkness to send, send it. */
			if (dark != NO_FACE_SEND)
			{
				packet_append_uint8(packet, dark);
			}

			packet_layer = packet_new(0, 0, 128);
			num_layers = 0;

			/* Go through the visible layers. */
			for (layer = LAYER_FLOOR; layer <= NUM_LAYERS; layer++)
			{
				int sub_layer, socket_layer;

				for (sub_layer = 0; sub_layer < NUM_SUB_LAYERS; sub_layer++)
				{
					object *tmp = GET_MAP_SPACE_LAYER(msp, layer, sub_layer);

					/* Double check that we can actually see this object. */
					if (tmp && QUERY_FLAG(tmp, FLAG_HIDDEN))
					{
						tmp = NULL;
					}

					/* This is done so that the player image is always shown
					 * to the player, even if they are standing on top of another
					 * player or monster. */
					if (tmp && tmp->layer == LAYER_LIVING && pl->x == nx && pl->y == ny)
					{
						tmp = pl;
					}

					/* Still nothing, but there's a magic mirror on this tile? */
					if (!tmp && mirror)
					{
						magic_mirror_struct *m_data = MMIRROR(mirror);
						mapstruct *mirror_map;

						if (m_data && (mirror_map = magic_mirror_get_map(mirror)) && !OUT_OF_MAP(mirror_map, m_data->x, m_data->y))
						{
							tmp = GET_MAP_SPACE_LAYER(GET_MAP_SPACE_PTR(mirror_map, m_data->x, m_data->y), layer, sub_layer);
						}
					}

					/* Handle objects that are shown based on their direction
					 * and the player's position. */
					if (tmp && QUERY_FLAG(tmp, FLAG_DRAW_DIRECTION))
					{
						/* If the object is dir [0124568] and not in the top
						 * or right quadrant or on the central square, do not
						 * show it. */
						if ((!tmp->direction || tmp->direction == NORTH || tmp->direction == NORTHEAST || tmp->direction == SOUTHEAST || tmp->direction == SOUTH || tmp->direction == SOUTHWEST || tmp->direction == NORTHWEST) && !((ax <= CONTR(pl)->socket.mapx_2) && (ay <= CONTR(pl)->socket.mapy_2)) && !((ax > CONTR(pl)->socket.mapx_2) && (ay < CONTR(pl)->socket.mapy_2)))
						{
							tmp = NULL;
						}
						/* If the object is dir [0234768] and not in the top
						 * or left quadrant or on the central square, do not
						 * show it. */
						else if ((!tmp->direction || tmp->direction == NORTHEAST || tmp->direction == EAST || tmp->direction == SOUTHEAST || tmp->direction == SOUTHWEST || tmp->direction == WEST || tmp->direction == NORTHWEST) && !((ax <= CONTR(pl)->socket.mapx_2) && (ay <= CONTR(pl)->socket.mapy_2)) && !((ax < CONTR(pl)->socket.mapx_2) && (ay > CONTR(pl)->socket.mapy_2)))
						{
							tmp = NULL;
						}
					}

					socket_layer = NUM_LAYERS * sub_layer + layer - 1;

					/* Found something. */
					if (tmp)
					{
						sint16 face;
						uint8 quick_pos = tmp->quick_pos;
						uint8 flags = 0, probe_val = 0;
						uint32 flags2 = 0;
						object *head = tmp->head ? tmp->head : tmp;
						tag_t target_object_count = 0;

						/* If we have a multi-arch object. */
						if (quick_pos)
						{
							flags |= MAP2_FLAG_MULTI;

							/* Tail? */
							if (tmp->head)
							{
								/* If true, we have sent a part of this in this map
								 * update before, so skip it. */
								if (head->update_tag == map2_count)
								{
									face = 0;
								}
								else
								{
									/* Mark this object as sent. */
									head->update_tag = map2_count;
									face = head->face->number;
								}
							}
							/* Head. */
							else
							{
								if (tmp->update_tag == map2_count)
								{
									face = 0;
								}
								else
								{
									tmp->update_tag = map2_count;
									face = tmp->face->number;
								}
							}
						}
						else
						{
							face = tmp->face->number;
						}

						/* Player? So we want to send their name. */
						if (tmp->type == PLAYER)
						{
							flags |= MAP2_FLAG_NAME;
						}

						/* If our player has this object as their target, we want to
						 * know its HP percent. */
						if (head->count == CONTR(pl)->target_object_count)
						{
							flags |= MAP2_FLAG_PROBE;
							probe_val = MAX(1, ((double) head->stats.hp / ((double) head->stats.maxhp / 100.0)));
						}

						/* Z position set? */
						if (head->z)
						{
							flags |= MAP2_FLAG_HEIGHT;
						}

						/* Check if the object has zoom, or check if the magic mirror
						 * should affect the zoom value of this layer. */
						if ((head->zoom_x && head->zoom_x != 100) || (head->zoom_y && head->zoom_y != 100) || (mirror && mirror->last_heal && mirror->last_heal != 100 && mirror->path_attuned & (1U << (layer - 1))))
						{
							flags |= MAP2_FLAG_ZOOM;
						}

						if (head->align || (mirror && mirror->align))
						{
							flags |= MAP2_FLAG_ALIGN;
						}

						/* Draw the object twice if set, but only if it's not
						 * in the bottom quadrant of the map. */
						if ((QUERY_FLAG(tmp, FLAG_DRAW_DOUBLE) && (ax < CONTR(pl)->socket.mapx_2 || ay < CONTR(pl)->socket.mapy_2)) || QUERY_FLAG(tmp, FLAG_DRAW_DOUBLE_ALWAYS))
						{
							flags |= MAP2_FLAG_DOUBLE;
						}

						if (head->alpha)
						{
							flags2 |= MAP2_FLAG2_ALPHA;
						}

						if (head->rotate)
						{
							flags2 |= MAP2_FLAG2_ROTATE;
						}

						if (QUERY_FLAG(pl, FLAG_SEE_IN_DARK) && ((head->layer == LAYER_LIVING && d < 150) || (head->type == CONTAINER && (head->sub_type & 1) == ST1_CONTAINER_CORPSE && QUERY_FLAG(head, FLAG_IS_USED_UP) && (float) head->stats.food / head->last_eat >= CORPSE_INFRAVISION_PERCENT / 100.0)))
						{
							flags2 |= MAP2_FLAG2_INFRAVISION;
						}

						if (head != pl && layer == LAYER_LIVING && IS_LIVE(head))
						{
							flags2 |= MAP2_FLAG2_TARGET;
							target_object_count = head->count;
						}

						if (flags2)
						{
							flags |= MAP2_FLAG_MORE;
						}

						/* Damage animation? Store it for later. */
						if (tmp->last_damage && tmp->damage_round_tag == global_round_tag)
						{
							ext_flags |= MAP2_FLAG_EXT_ANIM;
							anim_type = ANIM_DAMAGE;
							anim_value = tmp->last_damage;
						}

						/* Now, check if we have cached this. */
						if (mp->faces[socket_layer] == face && mp->quick_pos[socket_layer] == quick_pos && mp->flags[socket_layer] == flags && (layer != LAYER_LIVING || (mp->probe == probe_val && mp->target_object_count == target_object_count)))
						{
							continue;
						}

						/* Different from cache, add it to the cache now. */
						mp->faces[socket_layer] = face;
						mp->quick_pos[socket_layer] = quick_pos;
						mp->flags[socket_layer] = flags;

						if (layer == LAYER_LIVING)
						{
							mp->probe = probe_val;
							mp->target_object_count = target_object_count;
						}

						if (OBJECT_IS_HIDDEN(pl, head))
						{
							/* Update target if applicable. */
							if (flags & MAP2_FLAG_PROBE)
							{
								CONTR(pl)->target_object = NULL;
								CONTR(pl)->target_object_count = 0;
								send_target_command(CONTR(pl));
							}

							if (mp->faces[socket_layer])
							{
								packet_append_uint8(packet_layer, MAP2_LAYER_CLEAR);
								packet_append_uint8(packet_layer, socket_layer);
								num_layers++;
							}

							continue;
						}

						num_layers++;

						packet_append_uint8(packet_layer, socket_layer);
						packet_append_uint16(packet_layer, face);
						packet_append_uint8(packet_layer, GET_CLIENT_FLAGS(head));
						packet_append_uint8(packet_layer, flags);

						/* Multi-arch? Add it's quick pos. */
						if (flags & MAP2_FLAG_MULTI)
						{
							packet_append_uint8(packet_layer, quick_pos);
						}

						/* Player name? Add the player's name, and their player name color. */
						if (flags & MAP2_FLAG_NAME)
						{
							packet_append_string_terminated(packet_layer, CONTR(tmp)->quick_name);
							packet_append_string_terminated(packet_layer, get_playername_color(pl, tmp));
						}

						/* Target's HP bar. */
						if (flags & MAP2_FLAG_PROBE)
						{
							packet_append_uint8(packet_layer, probe_val);
						}

						/* Z position. */
						if (flags & MAP2_FLAG_HEIGHT)
						{
							if (mirror && mirror->last_eat)
							{
								packet_append_sint16(packet_layer, head->z + mirror->last_eat);
							}
							else
							{
								packet_append_sint16(packet_layer, head->z);
							}
						}

						if (flags & MAP2_FLAG_ZOOM)
						{
							/* First check mirror, even if the object *does* have custom zoom. */
							if (mirror && mirror->last_heal)
							{
								packet_append_uint16(packet_layer, mirror->last_heal);
								packet_append_uint16(packet_layer, mirror->last_heal);
							}
							else
							{
								packet_append_uint16(packet_layer, head->zoom_x);
								packet_append_uint16(packet_layer, head->zoom_y);
							}
						}

						if (flags & MAP2_FLAG_ALIGN)
						{
							if (mirror && mirror->align)
							{
								packet_append_sint16(packet_layer, head->align + mirror->align);
							}
							else
							{
								packet_append_sint16(packet_layer, head->align);
							}
						}

						if (flags & MAP2_FLAG_MORE)
						{
							packet_append_uint32(packet_layer, flags2);

							if (flags2 & MAP2_FLAG2_ALPHA)
							{
								packet_append_uint8(packet_layer, head->alpha);
							}

							if (flags2 & MAP2_FLAG2_ROTATE)
							{
								packet_append_sint16(packet_layer, head->rotate);
							}

							if (flags2 & MAP2_FLAG2_TARGET)
							{
								packet_append_uint32(packet_layer, target_object_count);
								packet_append_uint8(packet_layer, is_friend_of(pl, head));
							}
						}
					}
					/* Didn't find anything. Now, if we have previously seen a face
					 * on this layer, we will want the client to clear it. */
					else if (mp->faces[socket_layer])
					{
						mp->faces[socket_layer] = 0;
						mp->quick_pos[socket_layer] = 0;
						packet_append_uint8(packet_layer, MAP2_LAYER_CLEAR);
						packet_append_uint8(packet_layer, socket_layer);
						num_layers++;
					}
				}
			}

			packet_append_uint8(packet, num_layers);

			packet_merge(packet_layer, packet);
			packet_free(packet_layer);

			/* Kill animation? */
			if (GET_MAP_RTAG(m, nx, ny) == global_round_tag)
			{
				ext_flags |= MAP2_FLAG_EXT_ANIM;
				anim_type = ANIM_KILL;
				anim_value = GET_MAP_DAMAGE(m, nx, ny);
			}

			/* Add flags for this tile. */
			packet_append_uint8(packet, ext_flags);

			/* Animation? Add its type and value. */
			if (ext_flags & MAP2_FLAG_EXT_ANIM)
			{
				packet_append_uint8(packet, anim_type);
				packet_append_uint16(packet, anim_value);
			}

			/* If nothing has really changed, go back to the old position
			 * in the packet. */
			if (!(mask & 0x3f) && !num_layers && !ext_flags)
			{
				packet_set_pos(packet, oldpos);
			}

			/* Set 'mirror' back to NULL, so we'll try to re-find it on another tile. */
			mirror = NULL;
		}
	}

	/* Verify that we in fact do need to send this. */
	if (packet->len >= 4)
	{
		socket_send_packet(&CONTR(pl)->socket, packet);
	}
	else
	{
		packet_free(packet);
	}

	if (packet_sound->len >= 1)
	{
		socket_send_packet(&CONTR(pl)->socket, packet_sound);
	}
	else
	{
		packet_free(packet_sound);
	}
}

void socket_command_quest_list(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	object *quest_container, *tmp;
	StringBuffer *sb;
	packet_struct *packet;
	char *cp;
	size_t cp_len;

	quest_container = pl->quest_container;

	if (!quest_container || !quest_container->inv)
	{
		packet = packet_new(CLIENT_CMD_BOOK, 0, 0);
		packet_append_string_terminated(packet, "<title>No quests to speak of.</title>");
		socket_send_packet(&pl->socket, packet);
		return;
	}

	sb = stringbuffer_new();
	stringbuffer_append_string(sb, "<book=Quest List><title>Incomplete quests:</title>\n");

	/* First show incomplete quests */
	for (tmp = quest_container->inv; tmp; tmp = tmp->below)
	{
		if (tmp->type != QUEST_CONTAINER || tmp->magic == QUEST_STATUS_COMPLETED)
		{
			continue;
		}

		stringbuffer_append_printf(sb, "\n<title>%s</title>\n%s%s", tmp->name, tmp->msg ? tmp->msg : "", tmp->msg ? "\n" : "");

		if (tmp->sub_type == QUEST_TYPE_MULTI)
		{
			object *tmp2, *last;

			/* Find the last entry. */
			for (last = tmp->inv; last && last->below; last = last->below)
			{
			}

			/* Show the quest parts. */
			for (tmp2 = last; tmp2; tmp2 = tmp2->above)
			{
				if (tmp2->msg)
				{
					stringbuffer_append_printf(sb, "\n- %s", tmp2->msg);

					if (tmp2->magic == QUEST_STATUS_COMPLETED)
					{
						stringbuffer_append_string(sb, " [done]");
					}
				}

				switch (tmp2->sub_type)
				{
					case QUEST_TYPE_KILL:
						stringbuffer_append_printf(sb, "\n<x=10>Status: %d/%d", MIN(tmp2->last_sp, tmp2->last_grace), tmp2->last_grace);
						break;
				}
			}

			stringbuffer_append_string(sb, "\n");
		}
		else
		{
			switch (tmp->sub_type)
			{
				case QUEST_TYPE_KILL:
					stringbuffer_append_printf(sb, "Status: %d/%d\n", MIN(tmp->last_sp, tmp->last_grace), tmp->last_grace);
					break;
			}
		}
	}

	stringbuffer_append_string(sb, "<p>\n<title>Completed quests:</title>\n");

	/* Now show completed quests */
	for (tmp = quest_container->inv; tmp; tmp = tmp->below)
	{
		if (tmp->type != QUEST_CONTAINER || tmp->magic != QUEST_STATUS_COMPLETED)
		{
			continue;
		}

		stringbuffer_append_printf(sb, "\n<title>%s</title>\n%s%s", tmp->name, tmp->msg ? tmp->msg : "", tmp->msg ? "\n" : "");
	}

	cp_len = stringbuffer_length(sb);
	cp = stringbuffer_finish(sb);

	packet = packet_new(CLIENT_CMD_BOOK, 0, 0);
	packet_append_data_len(packet, (uint8 *) cp, cp_len);
	socket_send_packet(&pl->socket, packet);
	free(cp);
}

void socket_command_clear(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	ns->packet_recv_cmd->len = 0;
}

void socket_command_move_path(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	uint8 x, y;
	mapstruct *m;
	int xt, yt;
	path_node *node, *tmp;

	x = packet_to_uint8(data, len, &pos);
	y = packet_to_uint8(data, len, &pos);

	/* Validate the passed x/y. */
	if (x >= pl->socket.mapx || y >= pl->socket.mapy)
	{
		return;
	}

	/* If this is the middle of the screen where the player is already,
	 * there isn't much to do. */
	if (x == pl->socket.mapx_2 && y == pl->socket.mapy_2)
	{
		return;
	}

	/* The x/y we got above is from the client's map, so 0,0 is
	 * actually topmost (northwest) corner of the map in the client,
	 * and not 0,0 of the actual map, so we need to transform it to
	 * actual map coordinates. */
	xt = pl->ob->x + (x - pl->socket.mapx / 2);
	yt = pl->ob->y + (y - pl->socket.mapy / 2);
	m = get_map_from_coord(pl->ob->map, &xt, &yt);

	/* Invalid x/y. */
	if (!m)
	{
		return;
	}

	/* Find and compress the path to the destination. */
	node = compress_path(find_path(pl->ob, pl->ob->map, pl->ob->x, pl->ob->y, m, xt, yt));

	/* No path available. */
	if (!node)
	{
		return;
	}

	/* Clear any previously queued paths. */
	player_path_clear(pl);

	/* 'node' now actually points to where the player is standing, so
	 * skip that. */
	if (node->next)
	{
		for (tmp = node->next; tmp; tmp = tmp->next)
		{
			player_path_add(pl, tmp->map, tmp->x, tmp->y);
		}
	}

	/* The last x,y where we wanted to move is not included in the
	 * above paths finding, so we have to add it manually. */
	player_path_add(pl, m, xt, yt);
}

void socket_command_item_ready(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	tag_t tag;
	object *tmp;

	tag = packet_to_uint32(data, len, &pos);

	if (!tag)
	{
		return;
	}

	/* Try to look for objects to unready first. */
	for (tmp = pl->ob->inv; tmp; tmp = tmp->below)
	{
		if (QUERY_FLAG(tmp, FLAG_IS_READY))
		{
			CLEAR_FLAG(tmp, FLAG_IS_READY);
			esrv_update_item(UPD_FLAGS, tmp);
			fix_player(pl->ob);
			draw_info_format(COLOR_WHITE, pl->ob, "Unready %s.", query_base_name(tmp, pl->ob));

			/* If we wanted to unready the object, no point in going on. */
			if (tmp->count == tag)
			{
				return;
			}

			break;
		}
	}

	/* If we are here, we want to ready an object. */
	for (tmp = pl->ob->inv; tmp; tmp = tmp->below)
	{
		if (tmp->count == tag)
		{
			SET_FLAG(tmp, FLAG_IS_READY);
			esrv_update_item(UPD_FLAGS, tmp);
			fix_player(pl->ob);
			draw_info_format(COLOR_WHITE, pl->ob, "Ready %s as ammunition.", query_base_name(tmp, pl->ob));
			break;
		}
	}
}

void socket_command_fire(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	int dir;
	double skill_time, delay;

	dir = packet_to_uint8(data, len, &pos);
	dir = MAX(0, MIN(dir, 8));

	if (!pl->equipment[PLAYER_EQUIP_WEAPON])
	{
		return;
	}

	if (!check_skill_to_fire(pl->ob, pl->equipment[PLAYER_EQUIP_WEAPON]))
	{
		return;
	}

	if (pl->action_attack > global_round_tag)
	{
		return;
	}

	skill_time = skills[pl->ob->chosen_skill->stats.sp].time;
	delay = 0;

	object_ranged_fire(pl->equipment[PLAYER_EQUIP_WEAPON], pl->ob, dir, &delay);

	if (skill_time > 1.0f)
	{
		skill_time -= (SK_level(pl->ob) / 10 / 3) * 0.1f;

		if (skill_time < 1.0f)
		{
			skill_time = 1.0f;
		}
	}

	pl->action_attack = global_round_tag + skill_time + delay;

	pl->action_timer = (float) (pl->action_attack - global_round_tag) / (1000000 / MAX_TIME) * 1000.0;
	pl->last_action_timer = 0;
}

void socket_command_keepalive(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	ns->keepalive = 0;
}

void socket_command_password_change(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	char pswd_current[MAX_BUF], pswd_new[MAX_BUF];
	size_t pswd_len;

	/* Get the current and new password. */
	packet_to_string(data, len, &pos, pswd_current, sizeof(pswd_current));
	packet_to_string(data, len, &pos, pswd_new, sizeof(pswd_new));

	/* Make sure there are no untypeable characters... */
	string_replace_unprintable_chars(pswd_current);
	string_replace_unprintable_chars(pswd_new);

	/* Make sure there is current and new password. */
	if (*pswd_current == '\0' || *pswd_new == '\0')
	{
		return;
	}

	pswd_len = strlen(pswd_new);

	/* Make sure the new password has a valid length. */
	if (pswd_len < PLAYER_PASSWORD_MIN || pswd_len > PLAYER_PASSWORD_MAX)
	{
		draw_info_format(COLOR_RED, pl->ob, "That password has an invalid length (must be %d-%d).", PLAYER_PASSWORD_MIN, PLAYER_PASSWORD_MAX);
		return;
	}

	/* Ensure the current password matches, but silently ignore if it
	 * doesn't as the client should handle this. */
	if (!strcmp(crypt_string(pswd_current, NULL), pl->password))
	{
		return;
	}

	/* Update the player's password. */
	strcpy(pl->password, crypt_string(pswd_new, NULL));
	draw_info(COLOR_GREEN, pl->ob, "Your password has been changed successfully.");
}

void socket_command_move(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	uint8 dir, run_on;

	dir = packet_to_uint8(data, len, &pos);
	dir = MAX(0, MIN(dir, 8));
	run_on = packet_to_uint8(data, len, &pos);

	pl->run_on = MAX(0, MIN(1, run_on));

	if (dir != 0)
	{
		pl->ob->speed_left -= 1.0;
		move_object(pl->ob, dir);
	}
}

/**
 * Send target command, calculate the target's color level, etc.
 * @param pl Player requesting this. */
void send_target_command(player *pl)
{
	packet_struct *packet;

	if (!pl->ob->map)
	{
		return;
	}

	packet = packet_new(CLIENT_CMD_TARGET, 64, 64);
	packet_append_uint8(packet, pl->combat_mode);

	pl->ob->enemy = NULL;
	pl->ob->enemy_count = 0;

	if (!pl->target_object || pl->target_object == pl->ob || !OBJECT_VALID(pl->target_object, pl->target_object_count) || IS_INVISIBLE(pl->target_object, pl->ob))
	{
		packet_append_uint8(packet, CMD_TARGET_SELF);
		packet_append_string_terminated(packet, COLOR_YELLOW);
		packet_append_string_terminated(packet, pl->ob->name);

		pl->target_object = pl->ob;
		pl->target_object_count = 0;
	}
	else
	{
		if (is_friend_of(pl->ob, pl->target_object))
		{
			packet_append_uint8(packet, CMD_TARGET_FRIEND);
		}
		else
		{
			packet_append_uint8(packet, CMD_TARGET_ENEMY);

			pl->ob->enemy = pl->target_object;
			pl->ob->enemy_count = pl->target_object_count;
		}

		if (pl->target_object->level < level_color[pl->ob->level].yellow)
		{
			if (pl->target_object->level < level_color[pl->ob->level].green)
			{
				packet_append_string_terminated(packet, COLOR_GRAY);
			}
			else
			{
				if (pl->target_object->level < level_color[pl->ob->level].blue)
				{
					packet_append_string_terminated(packet, COLOR_GREEN);
				}
				else
				{
					packet_append_string_terminated(packet, COLOR_BLUE);
				}
			}
		}
		else
		{
			if (pl->target_object->level >= level_color[pl->ob->level].purple)
			{
				packet_append_string_terminated(packet, COLOR_PURPLE);
			}
			else if (pl->target_object->level >= level_color[pl->ob->level].red)
			{
				packet_append_string_terminated(packet, COLOR_RED);
			}
			else if (pl->target_object->level >= level_color[pl->ob->level].orange)
			{
				packet_append_string_terminated(packet, COLOR_ORANGE);
			}
			else
			{
				packet_append_string_terminated(packet, COLOR_YELLOW);
			}
		}

		if (pl->tgm)
		{
			char buf[MAX_BUF];

			snprintf(buf, sizeof(buf), "%s (lvl %d)", pl->target_object->name, pl->target_object->level);
			packet_append_string_terminated(packet, buf);
		}
		else
		{
			packet_append_string_terminated(packet, pl->target_object->name);
		}
	}

	socket_send_packet(&pl->socket, packet);
}

/**
 * This loads the first map an puts the player on it.
 * @param op The player object. */
static void set_first_map(object *op)
{
	object *current;

	strcpy(CONTR(op)->maplevel, first_map_path);
	op->x = -1;
	op->y = -1;

	if (!strcmp(first_map_path, "/tutorial"))
	{
		current = get_object();
		FREE_AND_COPY_HASH(EXIT_PATH(current), first_map_path);
		EXIT_X(current) = 1;
		EXIT_Y(current) = 1;
		current->last_eat = MAP_PLAYER_MAP;
		enter_exit(op, current);
		/* Update save bed position, so if we die, we don't end up in
		 * the public version of the map. */
		strncpy(CONTR(op)->savebed_map, CONTR(op)->maplevel, sizeof(CONTR(op)->savebed_map) - 1);
	}
	else
	{
		enter_exit(op, NULL);
	}

	/* Update save bed X/Y in any case. */
	CONTR(op)->bed_x = op->x;
	CONTR(op)->bed_y = op->y;
}

/**
 * Information about a character the player may choose. */
typedef struct new_char_struct
{
	/** Archetype of the player. */
	char arch[MAX_BUF];

	/**
	 * Maximum number of points the player can allocate to their
	 * character's stats. */
	int points_max;

	/** Base values of stats for this character. */
	int stats_base[NUM_STATS];

	/** Minimum values of stats for this character. */
	int stats_min[NUM_STATS];

	/** Maximum values of stats for this character. */
	int stats_max[NUM_STATS];
} new_char_struct;

/** All the loaded characters. */
static new_char_struct *new_chars = NULL;
/** Number of ::new_chars. */
static size_t num_new_chars = 0;

/**
 * Deinitialize ::new_chars. */
void new_chars_deinit(void)
{
	if (new_chars)
	{
		free(new_chars);
		new_chars = NULL;
	}

	num_new_chars = 0;
}

/**
 * Initialize ::new_chars by reading server_settings file. */
void new_chars_init(void)
{
	char filename[HUGE_BUF], buf[HUGE_BUF];
	FILE *fp;
	size_t added = 0, i;

	/* Open the server_settings file. */
	snprintf(filename, sizeof(filename), "%s/server_settings", settings.datapath);
	fp = fopen(filename, "r");

	while (fgets(buf, sizeof(buf) - 1, fp))
	{
		/* New race; added keeps track of how many archetypes have been
		 * added since the last new. */
		if (!strncmp(buf, "char ", 5))
		{
			added = 0;
		}
		/* Add new archetype for this race. */
		else if (!strncmp(buf, "gender ", 7))
		{
			char gender[MAX_BUF], arch[MAX_BUF], face[MAX_BUF];

			/* Parse the line. */
			if (sscanf(buf + 7, "%s %s %s", gender, arch, face) != 3)
			{
				logger_print(LOG(ERROR), "Bogus line in %s: %s", filename, buf);
				exit(1);
			}

			new_chars = realloc(new_chars, sizeof(*new_chars) * (num_new_chars + 1));
			strncpy(new_chars[num_new_chars].arch, arch, sizeof(new_chars[num_new_chars].arch) - 1);
			new_chars[num_new_chars].arch[sizeof(new_chars[num_new_chars].arch) - 1] = '\0';
			num_new_chars++;
			added++;
		}
		/* Data that applies to any gender archetype of this race. */
		else if (!strncmp(buf, "points_max ", 11) || !strncmp(buf, "stats_", 6))
		{
			/* Start from the end of the array. */
			for (i = num_new_chars - 1; ; i--)
			{
				if (!strncmp(buf, "points_max ", 11))
				{
					new_chars[i].points_max = atoi(buf + 11);
				}
				else if (!strncmp(buf, "stats_base ", 11) && sscanf(buf + 11, "%d %d %d %d %d %d %d", &new_chars[i].stats_base[STR], &new_chars[i].stats_base[DEX], &new_chars[i].stats_base[CON], &new_chars[i].stats_base[INT], &new_chars[i].stats_base[WIS], &new_chars[i].stats_base[POW], &new_chars[i].stats_base[CHA]) != NUM_STATS)
				{
					logger_print(LOG(ERROR), "Bogus line in %s: %s", filename, buf);
					exit(1);
				}
				else if (!strncmp(buf, "stats_min ", 10) && sscanf(buf + 10, "%d %d %d %d %d %d %d", &new_chars[i].stats_min[STR], &new_chars[i].stats_min[DEX], &new_chars[i].stats_min[CON], &new_chars[i].stats_min[INT], &new_chars[i].stats_min[WIS], &new_chars[i].stats_min[POW], &new_chars[i].stats_min[CHA]) != NUM_STATS)
				{
					logger_print(LOG(ERROR), "Bogus line in %s: %s", filename, buf);
					exit(1);
				}
				else if (!strncmp(buf, "stats_max ", 10) && sscanf(buf + 10, "%d %d %d %d %d %d %d", &new_chars[i].stats_max[STR], &new_chars[i].stats_max[DEX], &new_chars[i].stats_max[CON], &new_chars[i].stats_max[INT], &new_chars[i].stats_max[WIS], &new_chars[i].stats_max[POW], &new_chars[i].stats_max[CHA]) != NUM_STATS)
				{
					logger_print(LOG(ERROR), "Bogus line in %s: %s", filename, buf);
					exit(1);
				}

				/* Check if we have reached the total number of gender
				 * archetypes added for this race. */
				if (i == num_new_chars - added)
				{
					break;
				}
			}
		}
	}

	fclose(fp);
}

/**
 * Client sent us a new char creation.
 *
 * At this point we know the player's name and the password but nothing
 * about his (player char) base arch.
 *
 * This command tells us which the player has selected and how he has
 * setup the stats.
 *
 * If <b>anything</b> is not correct here, we kill this socket.
 * @param params Parameters.
 * @param len Length.
 * @param pl Player structure. */
void socket_command_new_char(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	archetype *player_arch;
	const char *name_tmp = NULL;
	object *op = pl->ob;
	int x = pl->ob->x, y = pl->ob->y;
	int stats[NUM_STATS];
	size_t i, j;
	char archname[MAX_BUF];

	/* Ignore the command if the player is already playing. */
	if (pl->state == ST_PLAYING)
	{
		return;
	}

	/* Incorrect state... */
	if (pl->state != ST_ROLL_STAT)
	{
		pl->socket.status = Ns_Dead;
		return;
	}

	packet_to_string(data, len, &pos, archname, sizeof(archname));
	stats[STR] = packet_to_uint8(data, len, &pos);
	stats[DEX] = packet_to_uint8(data, len, &pos);
	stats[CON] = packet_to_uint8(data, len, &pos);
	stats[INT] = packet_to_uint8(data, len, &pos);
	stats[WIS] = packet_to_uint8(data, len, &pos);
	stats[POW] = packet_to_uint8(data, len, &pos);
	stats[CHA] = packet_to_uint8(data, len, &pos);

	player_arch = find_archetype(archname);

	/* Invalid player arch? */
	if (!player_arch || player_arch->clone.type != PLAYER)
	{
		pl->socket.status = Ns_Dead;
		return;
	}

	for (i = 0; i < num_new_chars; i++)
	{
		if (!strcmp(archname, new_chars[i].arch))
		{
			break;
		}
	}

	if (i == num_new_chars)
	{
		pl->socket.status = Ns_Dead;
		return;
	}

	/* Ensure all stat points have been allocated. */
	if (stats[STR] + stats[DEX] + stats[CON] + stats[INT] + stats[WIS] + stats[POW] + stats[CHA] != new_chars[i].stats_min[STR] + new_chars[i].stats_min[DEX] + new_chars[i].stats_min[CON] + new_chars[i].stats_min[INT] + new_chars[i].stats_min[WIS] + new_chars[i].stats_min[POW] + new_chars[i].stats_min[CHA] + new_chars[i].points_max)
	{
		pl->socket.status = Ns_Dead;
		return;
	}

	/* Make sure all the stats are in a valid range. */
	for (j = 0; j < NUM_STATS; j++)
	{
		if (stats[j] < new_chars[i].stats_min[j])
		{
			pl->socket.status = Ns_Dead;
			return;
		}
		else if (stats[j] > new_chars[i].stats_max[j])
		{
			pl->socket.status = Ns_Dead;
			return;
		}
	}

	FREE_AND_ADD_REF_HASH(name_tmp, op->name);
	copy_object(&player_arch->clone, op, 0);
	op->custom_attrset = pl;
	pl->ob = op;
	FREE_AND_CLEAR_HASH2(op->name);
	op->name = name_tmp;
	op->x = x;
	op->y = y;
	/* So the player faces east. */
	op->direction = op->anim_last_facing = op->anim_last_facing_last = op->facing = 3;
	/* We assume that players always have a valid animation. */
	SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);

	pl->orig_stats.Str = stats[STR];
	pl->orig_stats.Dex = stats[DEX];
	pl->orig_stats.Con = stats[CON];
	pl->orig_stats.Int = stats[INT];
	pl->orig_stats.Wis = stats[WIS];
	pl->orig_stats.Pow = stats[POW];
	pl->orig_stats.Cha = stats[CHA];

	SET_FLAG(op, FLAG_NO_FIX_PLAYER);
	/* This must before then initial items are given. */
	esrv_new_player(CONTR(op), op->weight + op->carrying);

	/* Trigger the global BORN event */
	trigger_global_event(GEVENT_BORN, op, NULL);

	/* Trigger the global LOGIN event */
	trigger_global_event(GEVENT_LOGIN, CONTR(op), CONTR(op)->socket.host);

	FREE_AND_CLEAR_HASH2(op->msg);

#ifdef AUTOSAVE
	CONTR(op)->last_save_tick = pticks;
#endif

	display_motd(op);

	draw_info_flags_format(NDI_ALL, COLOR_DK_ORANGE, op, "%s entered the game.", op->name);
	give_initial_items(op, op->randomitems);
	CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
	/* Force sending of skill exp data to client */
	CONTR(op)->last_stats.exp = 1;
	fix_player(op);
	link_player_skills(op);
	CONTR(op)->state = ST_PLAYING;
	esrv_new_player(CONTR(op), op->weight + op->carrying);
	esrv_update_item(UPD_FACE, op);
	esrv_send_inventory(op, op);

	set_first_map(op);
	SET_FLAG(op, FLAG_FRIENDLY);

	CONTR(op)->socket.update_tile = 0;
	CONTR(op)->socket.look_position = 0;
	CONTR(op)->socket.ext_title_flag = 1;
}

/**
 * Generate player's extended name from race, gender, guild, etc.
 * @param pl The player. */
void generate_ext_title(player *pl)
{
	char name[MAX_BUF], race[MAX_BUF];
	int i;

	strncpy(pl->quick_name, pl->ob->name, sizeof(pl->quick_name) - 1);
	pl->quick_name[sizeof(pl->quick_name) - 1] = '\0';

	for (i = 0; i < pl->num_cmd_permissions; i++)
	{
		if (pl->cmd_permissions[i] && string_startswith(pl->cmd_permissions[i], "[") && string_endswith(pl->cmd_permissions[i], "]"))
		{
			strncat(pl->quick_name, " ", sizeof(pl->quick_name) - strlen(pl->quick_name) - 1);
			strncat(pl->quick_name, pl->cmd_permissions[i], sizeof(pl->quick_name) - strlen(pl->quick_name) - 1);
		}
	}

	snprintf(name, sizeof(name), "%s", pl->quick_name);

	if (pl->afk)
	{
		strncat(name, " [AFK]", sizeof(name) - strlen(name) - 1);
	}

	snprintf(pl->ext_title, sizeof(pl->ext_title), "%s\n%s %s", name, gender_noun[object_get_gender(pl->ob)], player_get_race_class(pl->ob, race, sizeof(race)));
}

void socket_command_target(socket_struct *ns, player *pl, uint8 *data, size_t len, size_t pos)
{
	uint8 type;

	type = packet_to_uint8(data, len, &pos);

	if (type == CMD_TARGET_TCOMBAT)
	{
		if (pl->combat_mode)
		{
			pl->combat_mode = 0;
		}
		else
		{
			pl->combat_mode = 1;
		}

		send_target_command(pl);
	}
	else if (type == CMD_TARGET_MAPXY)
	{
		uint8 x, y;
		uint32 count, target_object_count;
		int i, xt, yt;
		mapstruct *m;
		object *tmp;

		x = packet_to_uint8(data, len, &pos);
		y = packet_to_uint8(data, len, &pos);
		count = packet_to_uint32(data, len, &pos);

		/* Validate the passed x/y. */
		if (x >= pl->socket.mapx || y >= pl->socket.mapy)
		{
			return;
		}

		target_object_count = pl->target_object_count;
		pl->target_object = NULL;
		pl->target_object_count = 0;

		(void) count;

		for (i = 0; i <= SIZEOFFREE1 && !pl->target_object_count; i++)
		{
			/* Check whether we are still in range of the player's
			 * viewport, and whether the player can see the square. */
			if (x + freearr_x[i] < 0 || x + freearr_x[i] >= pl->socket.mapx || y + freearr_y[i] < 0 || y + freearr_y[i] >= pl->socket.mapy || pl->blocked_los[x + freearr_x[i]][y + freearr_y[i]] > BLOCKED_LOS_BLOCKSVIEW)
			{
				continue;
			}

			/* The x/y we got above is from the client's map, so 0,0 is
			 * actually topmost (northwest) corner of the map in the client,
			 * and not 0,0 of the actual map, so we need to transform it to
			 * actual map coordinates. */
			xt = pl->ob->x + (x - pl->socket.mapx_2) + freearr_x[i];
			yt = pl->ob->y + (y - pl->socket.mapy_2) + freearr_y[i];
			m = get_map_from_coord(pl->ob->map, &xt, &yt);

			/* Invalid x/y. */
			if (!m)
			{
				continue;
			}

			/* Nothing alive on this spot. */
			if (!(GET_MAP_FLAGS(m, xt, yt) & (P_IS_MONSTER | P_IS_PLAYER)))
			{
				continue;
			}

			FOR_MAP_LAYER_BEGIN(m, xt, yt, LAYER_LIVING, tmp)
			{
				tmp = HEAD(tmp);

				if ((!count || tmp->count == count) && IS_LIVE(tmp) && tmp != pl->ob && !IS_INVISIBLE(tmp, pl->ob) && !OBJECT_IS_HIDDEN(pl->ob, tmp))
				{
					pl->target_object = tmp;
					pl->target_object_count = tmp->count;
					FOR_MAP_LAYER_BREAK;
				}
			}
			FOR_MAP_LAYER_END
		}

		if (pl->target_object_count != target_object_count)
		{
			send_target_command(pl);
		}
	}
	else if (type == CMD_TARGET_CLEAR)
	{
		if (pl->target_object_count)
		{
			pl->target_object = NULL;
			pl->target_object_count = 0;
			send_target_command(pl);
		}
	}
}
