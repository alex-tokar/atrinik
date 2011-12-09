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
 * Defines various structures and values that are used for the new client
 * server communication method.
 *
 * @see newclient.h */

#ifndef NEWSERVER_H
#define NEWSERVER_H

/** How many items to show in the below window. Used in esrv_draw_look(). */
#define NUM_LOOK_OBJECTS 15

/**
 * @defgroup MAP2_FLAG_xxx Map2 layer flags
 * Flags used to mark what kind of data there is on layers
 * in map2 protocol.
 *@{*/
/** Multi-arch object. */
#define MAP2_FLAG_MULTI      1
/** Player name. */
#define MAP2_FLAG_NAME       2
/** Target's HP bar. */
#define MAP2_FLAG_PROBE      4
/** Tile's Z position. */
#define MAP2_FLAG_HEIGHT     8
/** Zoom. */
#define MAP2_FLAG_ZOOM 16
/** X align. */
#define MAP2_FLAG_ALIGN 32
/** Draw the object twice. */
#define MAP2_FLAG_DOUBLE 64
/** More flags from @ref MAP2_FLAG2_xxx. */
#define MAP2_FLAG_MORE 128
/*@}*/

/**
 * @defgroup MAP2_FLAG2_xxx Extended map2 layer flags
 * Extended flags used to mark what kind of data there is on layers
 * in map2 protocol.
 *@{*/
/** Custom alpha value. */
#define MAP2_FLAG2_ALPHA 1
/** Custom rotate value in degrees. */
#define MAP2_FLAG2_ROTATE 2
/** The object should be highlighted in red. */
#define MAP2_FLAG2_INFRAVISION 4
/*@}*/

/**
 * @defgroup MAP2_FLAG_EXT_xxx Map2 tile flags
 * Flags used to mark what kind of data there is on different
 * tiles in map2 protocol.
 *@{*/
/** An animation. */
#define MAP2_FLAG_EXT_ANIM   1
/*@}*/

/**
 * @defgroup ANIM_xxx Animation types
 * Animation types.
 *@{*/
/** Damage animation. */
#define ANIM_DAMAGE     1
/** Kill animation. */
#define ANIM_KILL       2
/*@}*/

/**
 * @defgroup MAP2_MASK_xxx Map2 mask flags
 * Flags used for masks in map2 protocol.
 *@{*/
/** Clear cell, with all layers. */
#define MAP2_MASK_CLEAR      0x2
/** Add darkness. */
#define MAP2_MASK_DARKNESS   0x4
/*@}*/

/**
 * @defgroup MAP2_LAYER_xxx Map2 layer types
 *@{*/
/** Clear this layer. */
#define MAP2_LAYER_CLEAR    255
/*@}*/

/**
 * @defgroup CMD_MAPSTATS_xxx Mapstats command types
 * Mapstats command types.
 *@{*/
/** Change map name. */
#define CMD_MAPSTATS_NAME 1
/** Change map music. */
#define CMD_MAPSTATS_MUSIC 2
/** Change map weather. */
#define CMD_MAPSTATS_WEATHER 3
/*@}*/

/**
 * One map cell. Used to hold 'cache' of faces we already sent
 * to the client. */
typedef struct MapCell_struct
{
	/** Cache of last sent ambient sound. */
	tag_t sound_ambient_count;

	/* Everything below will be cleared by memset() in when the map
	 * cell is no longer visible. */

	/** Darkness cache. */
	int	count;

	/** Faces we sent. */
	sint16 faces[NUM_REAL_LAYERS];

	/** Multi-arch cache. */
	uint8 quick_pos[NUM_REAL_LAYERS];

	/** Flags cache. */
	uint8 flags[NUM_REAL_LAYERS];

	/**
	 * Probe cache. No need for an array, since this only appears
	 * for players or monsters, both on layer 6. */
	uint8 probe;
} MapCell;

/** One map for a player. */
struct Map
{
	/** The map cells. */
	struct MapCell_struct cells[MAP_CLIENT_X][MAP_CLIENT_Y];
};

/** Possible socket statuses. */
enum Sock_Status
{
	Ns_Avail,
	Ns_Wait,
	Ns_Add,
	Ns_Login,
	Ns_Dead,
	Ns_Zombie
};

/** This contains basic information on the socket structure. */
typedef struct socket_struct
{
	/** The actual file descriptor we are using. */
	int fd;

	/**
	 * If someone is too long idle in the login, he will get
	 * disconnected. */
	int login_count;

	/** X size of the map the client wants. */
	int mapx;

	/** Y size of the map the client wants. */
	int mapy;

	/** X size of the map the client wants / 2. */
	int mapx_2;

	/** Y size of the map the client wants / 2. */
	int mapy_2;

	/** Which host it is connected from (ip address). */
	char *host;

	/** Version of the client. */
	uint32 socket_version;

	/** Marker to see we must update the below windows of the tile the player is. */
	uint32 update_tile;

	/** Marker to map draw/draw below. */
	uint32 below_clear:1;

	/** When set, a "connect" was initialized as "player". */
	uint32 addme:1;

	/** Does the client want sound? */
	uint32 sound:1;

	/** Has the client sent version command? */
	uint32 version:1;

	/** Has the client requested settings file? */
	uint32 rf_settings:1;

	/** Has the client requested skills file? */
	uint32 rf_skills:1;

	/** Has the client requested spells file? */
	uint32 rf_spells:1;

	/** Has the client requested animations file? */
	uint32 rf_anims:1;

	/** Has the client requested bitmaps file? */
	uint32 rf_bmaps:1;

	/** Has the client requested hfiles file? */
	uint32 rf_hfiles:1;

	/** Is the client a bot? */
	uint32 is_bot:1;

	/** Start of drawing of look window. */
	sint16 look_position;

	/** Faceset the client is using, default 0. */
	uint8 faceset;

	/**
	 * How many times the player has failed to provide the right
	 * password. */
	uint8 password_fails;

	/** Send ext title to client. */
	uint8 ext_title_flag;

	/** Current state of the socket. */
	enum Sock_Status status;

	/** Last map. */
	struct Map lastmap;

	/** Holds one command to handle. */
	SockList inbuf;

	/** Raw data read in from the socket. */
	SockList readbuf;

	/** Buffer for player commands. */
	SockList cmdbuf;

	struct packet_struct *packet_head;
	struct packet_struct *packet_tail;
	pthread_mutex_t packet_mutex;

	/**
	 * Buffer for how many ticks have passed since the last keep alive
	 * command. When this reaches @ref SOCKET_KEEPALIVE_TIMEOUT, the
	 * socket is disconnected. */
	uint32 keepalive;
} socket_struct;

/**
 * How many seconds must pass since the last keep alive command for the
 * socket to be disconnected. */
#define SOCKET_KEEPALIVE_TIMEOUT (60 * 10)

/** Holds some system related information. */
typedef struct Socket_Info_struct
{
	/** Timeout for select. */
	struct timeval timeout;

	/** Max filedescriptor on the system. */
	int	max_filedescriptor;

	/** Number of connections. */
	int	nconns;

	/** Number of allocated in init_sockets. */
	int allocated_sockets;
} Socket_Info;

/**
 * A single file loaded from the updates directory that the client can
 * request. */
typedef struct update_file_struct
{
	/** File's CRC32. */
	unsigned long checksum;

	/** Length of the file. */
	size_t len;

	/** Uncompressed length of the file. */
	size_t ucomp_len;

	/** Name of the file. */
	char *filename;

	/** Compressed contents of the file. */
	char *contents;

	/** Socklist instance for sending the data about the file. */
	SockList sl;
} update_file_struct;

/**
 * A single data packet. */
typedef struct packet_struct
{
	/**
	 * Next packet to send. */
	struct packet_struct *next;

	/**
	 * Previous packet. */
	struct packet_struct *prev;

	/**
	 * The data. */
	uint8 *data;

	/**
	 * Length of 'data'. */
	size_t len;

	/**
	 * Current size of 'data'. */
	size_t size;

	/**
	 * Expand size. */
	size_t expand;

	/**
	 * Position in 'data'. */
	size_t pos;

	/**
	 * Whether to enable NDELAY on this packet. */
	uint8 ndelay;

	/**
	 * The packet's command type. */
    uint8 type;
} packet_struct;

/** Filename used to store information about the updated files. */
#define UPDATES_FILE_NAME "updates"
/**
 * Directory to recursively traverse, looking for files that the client
 * can request for an update. */
#define UPDATES_DIR_NAME "updates"

/**
 * Maximum password failures allowed before the server kills the
 * socket. */
#define MAX_PASSWORD_FAILURES 3

/**
 * How many packet structures to allocate when expanding the available
 * packet structures. */
#define SOCKET_PACKET_EXPAND 10

#endif
