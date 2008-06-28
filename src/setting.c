/*
 *      Copyright 2008 Fred Chien <cfsghost@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "setting.h"

void setting_save_to_file(const char *path, const char *data)
{
	FILE *fp;

	if (!g_file_test(path, G_FILE_TEST_EXISTS))
		g_creat(path, 0700);

	/* open config file */
	fp = fopen(path, "w");
	fputs(data, fp);
	fclose(fp);
}

void setting_save(Setting *setting)
{
	gchar *path;
	gchar *file_data;

	/* build config path */
	path = g_build_filename(g_get_user_config_dir(), "lxterminal/lxterminal.conf", NULL);

	/* push settings to GKeyFile */
	g_key_file_set_string(setting->keyfile, "general", "fontname", setting->fontname);
	g_key_file_set_string(setting->keyfile, "general", "selchars", setting->selchars);

	/* generate config data */
	file_data = g_key_file_to_data(setting->keyfile, NULL, NULL);

	/* save to config file */
	setting_save_to_file(path, file_data);

	/* release */
	g_free(file_data);
}

Setting *load_setting_from_file(const char *filename)
{
	GKeyFileFlags flags;
	GError *error = NULL;
	Setting *setting;

	setting = (Setting *)malloc(sizeof(Setting));

	/* initiate key_file */
	setting->keyfile = g_key_file_new();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

	/* Load config */
	if (g_file_test(filename, G_FILE_TEST_EXISTS)) {
		if (!g_key_file_load_from_file(setting->keyfile, filename, flags, &error))
			goto setting_default;

		/* general setting */
		setting->fontname = g_key_file_get_string(setting->keyfile, "general", "fontname", NULL);
		setting->selchars = g_key_file_get_string(setting->keyfile, "general", "selchars", NULL);
	}

setting_default:

	if (!setting->fontname)
		setting->fontname = g_strdup("monospace 10");

	if (!setting->selchars)
		setting->selchars = g_strdup("-A-Za-z0-9,./?%&#:_");

	return setting;
}
