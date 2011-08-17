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
 * Handles client settings. */

#include <global.h>

/** Text representations of the setting types. */
static const char *const opt_types[OPT_TYPE_NUM] =
{
	"bool", "input_num", "input_text", "range", "select", "int"
};

/** List of setting categories. */
setting_category **setting_categories = NULL;
/** Number of ::setting_categories. */
size_t setting_categories_num = 0;
/**
 * Whether we need to send a setup command to server to request mapsize
 * change. */
static uint8 setting_update_mapsize = 0;

/**
 * Load a setting value from file.
 * @param setting The setting to load the value into.
 * @param str The value to load. */
static void setting_load_value(setting_struct *setting, const char *str)
{
	switch (setting->type)
	{
		case OPT_TYPE_BOOL:
			if (KEYWORD_IS_TRUE(str))
			{
				setting->val.i = 1;
			}
			else if (KEYWORD_IS_FALSE(str))
			{
				setting->val.i = 0;
			}
			else
			{
				setting->val.i = atoi(str);
			}

			break;

		case OPT_TYPE_INPUT_NUM:
		case OPT_TYPE_RANGE:
		case OPT_TYPE_INT:
		case OPT_TYPE_SELECT:
			setting->val.i = atoi(str);
			break;

		case OPT_TYPE_INPUT_TEXT:
			if (setting->val.str)
			{
				free(setting->val.str);
			}

			setting->val.str = strdup(str);
			break;
	}
}

/**
 * Initialize the setting defaults. */
void settings_init()
{
	FILE *fp;
	char buf[HUGE_BUF], *cp;
	setting_category *category;
	setting_struct *setting;

	fp = fopen_wrapper(FILE_SETTINGS_TXT, "r");

	if (!fp)
	{
		LOG(llevError, "settings_init(): Missing "FILE_SETTINGS_TXT", cannot continue.\n");
	}

	category = NULL;
	setting = NULL;
	setting_categories = NULL;
	setting_categories_num = 0;

	while (fgets(buf, sizeof(buf) - 1, fp))
	{
		cp = strchr(buf, '\n');

		if (cp)
		{
			*cp = '\0';
		}

		cp = buf;

		while (*cp != '\0')
		{
			if (isspace(*cp))
			{
				cp++;
			}
			else
			{
				break;
			}
		}

		if (*cp == '#' || *cp == '\0')
		{
			continue;
		}

		if (!strcmp(cp, "end"))
		{
			if (setting)
			{
				category->settings = realloc(category->settings, sizeof(category->settings) * (category->settings_num + 1));
				category->settings[category->settings_num] = setting;
				category->settings_num++;
				setting = NULL;
			}
			else if (category)
			{
				setting_categories = realloc(setting_categories, sizeof(*setting_categories) * (setting_categories_num + 1));
				setting_categories[setting_categories_num] = category;
				setting_categories_num++;
				category = NULL;
			}
		}
		else if (setting)
		{
			if (!strncmp(cp, "type ", 5))
			{
				size_t type_id;

				for (type_id = 0; type_id < OPT_TYPE_NUM; type_id++)
				{
					if (!strcmp(cp + 5, opt_types[type_id]))
					{
						setting->type = type_id;
						break;
					}
				}

				if (type_id == OPT_TYPE_NUM)
				{
					LOG(llevError, "Invalid type: %s\n", cp + 5);
				}
				else if (type_id == OPT_TYPE_SELECT)
				{
					setting->custom_attrset = calloc(1, sizeof(setting_select));
				}
				else if (type_id == OPT_TYPE_RANGE)
				{
					setting->custom_attrset = calloc(1, sizeof(setting_range));
				}
			}
			else if (!strncmp(cp, "default ", 8))
			{
				setting_load_value(setting, cp + 8);
			}
			else if (!strncmp(cp, "desc ", 5))
			{
				setting->desc = strdup(cp + 5);
				convert_newline(setting->desc);
			}
			else if (!strncmp(cp, "internal ", 9))
			{
				setting->internal = KEYWORD_IS_TRUE(cp + 9) ? 1 : 0;
			}
			else if (setting->type == OPT_TYPE_SELECT && !strncmp(cp, "option ", 7))
			{
				setting_select *s_select = SETTING_SELECT(setting);

				s_select->options = realloc(s_select->options, sizeof(*s_select->options) * (s_select->options_len + 1));
				s_select->options[s_select->options_len] = strdup(cp + 7);
				s_select->options_len++;
			}
			else if (setting->type == OPT_TYPE_RANGE && !strncmp(cp, "range ", 6))
			{
				setting_range *range = SETTING_RANGE(setting);
				sint64 min, max;

				if (sscanf(cp + 6, "%"FMT64" - %"FMT64, &min, &max) == 2)
				{
					range->min = min;
					range->max = max;
				}
				else
				{
					LOG(llevBug, "settings_init(): Invalid line: %s\n", cp);
				}
			}
			else if (setting->type == OPT_TYPE_RANGE && !strncmp(cp, "advance ", 8))
			{
				SETTING_RANGE(setting)->advance = atoi(cp + 8);
			}
			else
			{
				LOG(llevBug, "settings_init(): Invalid line: %s\n", cp);
			}
		}
		else if (category)
		{
			if (!strncmp(cp, "setting ", 8))
			{
				setting = calloc(1, sizeof(*setting));
				setting->name = strdup(cp + 8);
			}
			else
			{
				LOG(llevBug, "settings_init(): Invalid line: %s\n", cp);
			}
		}
		else if (!strncmp(cp, "category ", 9))
		{
			category = calloc(1, sizeof(*category));
			category->name = strdup(cp + 9);
		}
	}

	fclose(fp);

	/* Now load the user's settings (if any). */
	settings_load();
}

/**
 * Load user's settings (if any). */
void settings_load()
{
	FILE *fp;
	char buf[HUGE_BUF], *cp;
	sint64 cat = 0, setting = 0;
	uint8 is_setting_name = 1;

	fp = fopen_wrapper(FILE_SETTINGS_DAT, "r");

	/* No user settings yet. */
	if (!fp)
	{
		return;
	}

	while (fgets(buf, sizeof(buf) - 1, fp))
	{
		cp = strchr(buf, '\n');

		if (cp)
		{
			*cp = '\0';
		}

		cp = buf;

		while (*cp != '\0')
		{
			if (isspace(*cp))
			{
				cp++;
			}
			else
			{
				break;
			}
		}

		if (*cp == '#' || *cp == '\0')
		{
			continue;
		}

		if (!strncmp(cp, "category ", 9))
		{
			cat = category_from_name(cp + 9);
		}
		else
		{
			if (is_setting_name)
			{
				setting = setting_from_name(cp);
			}
			else
			{
				if (cat != -1 && setting != -1)
				{
					setting_load_value(setting_categories[cat]->settings[setting], cp);
				}
			}

			is_setting_name = !is_setting_name;
		}
	}

	fclose(fp);
}

/**
 * Save the user's settings to file. */
void settings_save()
{
	FILE *fp;
	size_t cat, set;
	setting_struct *setting;

	fp = fopen_wrapper(FILE_SETTINGS_DAT, "w");

	if (!fp)
	{
		LOG(llevBug, "Could not open settings file ("FILE_SETTINGS_DAT").\n");
		return;
	}

	for (cat = 0; cat < setting_categories_num; cat++)
	{
		fprintf(fp, "category %s\n", setting_categories[cat]->name);

		for (set = 0; set < setting_categories[cat]->settings_num; set++)
		{
			setting = setting_categories[cat]->settings[set];

			fprintf(fp, "%s\n", setting->name);

			if (setting_is_text(setting))
			{
				fprintf(fp, "%s\n", setting->val.str);
			}
			else
			{
				fprintf(fp, "%"FMT64"\n", setting->val.i);
			}
		}
	}

	fclose(fp);
}

/**
 * Deinitialize the settings.
 *
 * User's settings are also saved to file using settings_save(). */
void settings_deinit()
{
	size_t cat, setting;

	/* Save the user's settings first. */
	settings_save();

	/* Start deinitializing. */
	for (cat = 0; cat < setting_categories_num; cat++)
	{
		for (setting = 0; setting < setting_categories[cat]->settings_num; setting++)
		{
			if (setting_is_text(setting_categories[cat]->settings[setting]))
			{
				free(setting_categories[cat]->settings[setting]->val.str);
			}

			free(setting_categories[cat]->settings[setting]->name);

			if (setting_categories[cat]->settings[setting]->desc)
			{
				free(setting_categories[cat]->settings[setting]->desc);
			}

			if (setting_categories[cat]->settings[setting]->type == OPT_TYPE_SELECT)
			{
				setting_select *s_select = SETTING_SELECT(setting_categories[cat]->settings[setting]);
				size_t option;

				for (option = 0; option < s_select->options_len; option++)
				{
					free(s_select->options[option]);
				}

				if (s_select->options)
				{
					free(s_select->options);
				}
			}

			if (setting_categories[cat]->settings[setting]->custom_attrset)
			{
				free(setting_categories[cat]->settings[setting]->custom_attrset);
			}
		}

		if (setting_categories[cat]->settings)
		{
			free(setting_categories[cat]->settings);
		}

		free(setting_categories[cat]->name);
		free(setting_categories[cat]);
	}

	if (setting_categories)
	{
		free(setting_categories);
		setting_categories = NULL;
	}

	setting_categories_num = 0;
}

/**
 * Get pointer to a setting's value.
 * @param setting The setting.
 * @return Pointer to the setting's value. */
void *setting_get(setting_struct *setting)
{
	switch (setting->type)
	{
		case OPT_TYPE_BOOL:
		case OPT_TYPE_INPUT_NUM:
		case OPT_TYPE_RANGE:
		case OPT_TYPE_INT:
		case OPT_TYPE_SELECT:
			return &setting->val.i;

		case OPT_TYPE_INPUT_TEXT:
			return setting->val.str;
	}

	return NULL;
}

/**
 * Get setting's string value.
 * @param cat ID of the category the setting is in.
 * @param setting Setting ID inside the category.
 * @return The setting's string value. */
const char *setting_get_str(int cat, int setting)
{
	return (const char *) setting_get(setting_categories[cat]->settings[setting]);
}

/**
 * Get setting's integer value.
 * @param cat ID of the category the setting is in.
 * @param setting Setting ID inside the category.
 * @return The setting's integer value. */
sint64 setting_get_int(int cat, int setting)
{
	return *(sint64 *) setting_get(setting_categories[cat]->settings[setting]);
}

/**
 * Apply a setting change, which needs to be handled regardless of
 * whether it's changed at runtime or at startup.
 * @param cat ID of the category the setting is in.
 * @param setting Setting ID inside the category.
 * @return 1 if the change was handled, 0 otherwise. */
static int setting_apply_always(int cat, int setting)
{
	switch (cat)
	{
		case OPT_CAT_GENERAL:
			switch (setting)
			{
				/* Need to hide/show the playerdoll widget. */
				case OPT_PLAYERDOLL:
					cur_widget[PDOLL_ID]->show = setting_get_int(cat, setting);
					return 1;
			}

			break;

		case OPT_CAT_DEVEL:
			switch (setting)
			{
				/* Need to hide/show the fps widget. */
				case OPT_SHOW_FPS:
					cur_widget[FPS_ID]->show = setting_get_int(cat, setting);
					return 1;
			}
	}

	return 0;
}

/**
 * Apply a setting change at run-time.
 * @param cat ID of the category the setting is in.
 * @param setting Setting ID inside the category. */
static void setting_apply_runtime(int cat, int setting)
{
	/* Try both run-time and startup-time changes first. */
	if (setting_apply_always(cat, setting))
	{
		return;
	}

	switch (cat)
	{
		case OPT_CAT_GENERAL:
			switch (setting)
			{
				/* Changed how exp display shows its data, redraw the
				 * widget. */
				case OPT_EXP_DISPLAY:
					WIDGET_REDRAW_ALL(SKILL_EXP_ID);
					break;
			}

			break;

		case OPT_CAT_CLIENT:
			switch (setting)
			{
				/* Resolution change. */
				case OPT_RESOLUTION:
				{
					int w, h;

					if (sscanf(SETTING_SELECT(setting_categories[cat]->settings[setting])->options[setting_get_int(cat, setting)], "%dx%d", &w, &h) == 2 && (ScreenSurface->w != w || ScreenSurface->h != h))
					{
						resize_window(w, h);
						video_set_size();
					}

					break;
				}

				/* Fullscreen change. */
				case OPT_FULLSCREEN:
					if ((setting_get_int(cat, setting) && !(ScreenSurface->flags & SDL_FULLSCREEN)) || (!setting_get_int(cat, setting) && ScreenSurface->flags & SDL_FULLSCREEN))
					{
						video_fullscreen_toggle(&ScreenSurface, NULL);
					}

					break;
			}

			break;

		case OPT_CAT_MAP:
			switch (setting)
			{
				/* Map width/height change. */
				case OPT_MAP_WIDTH:
				case OPT_MAP_HEIGHT:
					if (setting_update_mapsize)
					{
						char buf[MAX_BUF];

						snprintf(buf, sizeof(buf), "setup mapsize %"FMT64"x%"FMT64, setting_get_int(cat, OPT_MAP_WIDTH), setting_get_int(cat, OPT_MAP_HEIGHT));
						cs_write_string(buf, strlen(buf));
						setting_update_mapsize = 0;
					}

					break;
			}

			break;

		case OPT_CAT_SOUND:
			switch (setting)
			{
				/* Music volume change. */
				case OPT_VOLUME_MUSIC:
					sound_update_volume();
					break;
			}

			break;
	}
}

/**
 * Apply all settings that need to be applied at start-time (after
 * everything has been initialized successfully). */
void settings_apply()
{
	size_t i, j;

	for (i = 0; i < setting_categories_num; i++)
	{
		for (j = 0; j < setting_categories[i]->settings_num; j++)
		{
			setting_apply_always(i, j);
		}
	}
}

/**
 * Apply a change of settings at run-time (through the settings GUI, for
 * example). */
void settings_apply_change()
{
	size_t cat, setting;

	for (cat = 0; cat < setting_categories_num; cat++)
	{
		for (setting = 0; setting < setting_categories[cat]->settings_num; setting++)
		{
			setting_apply_runtime(cat, setting);
		}
	}
}

/**
 * Set setting's integer value.
 * @param cat ID of the category the setting is in.
 * @param setting Setting ID inside the category.
 * @param val Value to set. */
void setting_set_int(int cat, int setting, sint64 val)
{
	void *dst = setting_get(setting_categories[cat]->settings[setting]);

	(*(sint64 *) dst) = val;

	/* Map width/height, mark for update. */
	if (cat == OPT_CAT_MAP && (setting == OPT_MAP_WIDTH || setting == OPT_MAP_HEIGHT))
	{
		setting_update_mapsize = 1;
	}
}

/**
 * Check if the specified setting has a string value.
 * @param setting The setting.
 * @return 1 if it has a string value, 0 otherwise. */
int setting_is_text(setting_struct *setting)
{
	switch (setting->type)
	{
		case OPT_TYPE_INPUT_TEXT:
			return 1;
	}

	return 0;
}

/**
 * Find a category ID by its name.
 * @param name The name.
 * @return Category ID if found, -1 otherwise. */
sint64 category_from_name(const char *name)
{
	size_t cat;

	for (cat = 0; cat < setting_categories_num; cat++)
	{
		if (!strcmp(setting_categories[cat]->name, name))
		{
			return cat;
		}
	}

	return -1;
}

/**
 * Find a setting ID by its name.
 *
 * @note All categories are checked.
 * @param name The name.
 * @return Setting ID if found, -1 otherwise. */
sint64 setting_from_name(const char *name)
{
	size_t cat, setting;

	for (cat = 0; cat < setting_categories_num; cat++)
	{
		for (setting = 0; setting < setting_categories[cat]->settings_num; setting++)
		{
			if (!strcmp(setting_categories[cat]->settings[setting]->name, name))
			{
				return setting;
			}
		}
	}

	return -1;
}
