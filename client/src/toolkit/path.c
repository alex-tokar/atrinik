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
 * OS path API. */

#include <global.h>

/**
 * Initialize the path API.
 * @internal */
void toolkit_path_init(void)
{
	TOOLKIT_INIT_FUNC_START(path)
	{
		toolkit_import(stringbuffer);
	}
	TOOLKIT_INIT_FUNC_END()
}

/**
 * Deinitialize the path API.
 * @internal */
void toolkit_path_deinit(void)
{
}

/**
 * Joins two path components, eg, '/usr' and 'bin' -> '/usr/bin'.
 * @param path First path component.
 * @param path2 Second path component.
 * @return The joined path; should be freed when no longer needed. */
char *path_join(const char *path, const char *path2)
{
	StringBuffer *sb;
	size_t len;
	char *cp;

	sb = stringbuffer_new();
	stringbuffer_append_string(sb, path);

	len = strlen(path);

	if (len && path[len - 1] != '/')
	{
		stringbuffer_append_string(sb, "/");
	}

	stringbuffer_append_string(sb, path2);
	cp = stringbuffer_finish(sb);

	return cp;
}

/**
 * Extracts the directory component of a path.
 *
 * Example:
 * @code
 * path_dirname("/usr/local/foobar"); --> "/usr/local"
 * @endcode
 * @param path A path.
 * @return A directory name. This string should be freed when no longer
 * needed.
 * @author Hongli Lai (public domain) */
char *path_dirname(const char *path)
{
	const char *end;
	char *result;

	if (!path)
	{
		return NULL;
	}

	end = strrchr(path, '/');

	if (!end)
	{
		return strdup(".");
	}

	while (end > path && *end == '/')
	{
		end--;
	}

	result = strndup(path, end - path + 1);

	if (result[0] == '\0')
	{
		free(result);
		return strdup("/");
	}

	return result;
}

/**
 * Extracts the basename from path.
 *
 * Example:
 * @code
 * path_basename("/usr/bin/kate"); --> "kate"
 * @endcode
 * @param path A path.
 * @return The basename of the path. Should be freed when no longer
 * needed. */
char *path_basename(const char *path)
{
	const char *slash;

	if (!path)
	{
		return NULL;
	}

	while ((slash = strrchr(path, '/')))
	{
		if (*(slash + 1) != '\0')
		{
			return strdup(slash + 1);
		}
	}

	return strdup(path);
}

/**
 * Checks whether any directories in the given path don't exist, and
 * creates them if necessary.
 * @param path The path to check. */
void path_ensure_directories(const char *path)
{
	char buf[MAXPATHLEN], *cp;
	struct stat statbuf;

	if (!path || *path == '\0')
	{
		return;
	}

	strncpy(buf, path, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	cp = buf;

	while ((cp = strchr(cp + 1, '/')))
	{
		*cp = '\0';

		if (stat(buf, &statbuf) || !S_ISDIR(statbuf.st_mode))
		{
			if (mkdir(buf, 0777))
			{
				logger_print(LOG(BUG), "Cannot mkdir %s: %s", buf, strerror(errno));
				return;
			}
		}

		*cp = '/';
	}
}