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

Setting *load_setting_from_file(const char *filename)
{
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;
	Setting *setting;

	/* initiate key_file */
	keyfile = g_key_file_new();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

	/* Load config */
	if (g_file_test(filename, G_FILE_TEST_EXISTS))
		return NULL;

	if (!g_key_file_load_from_file (keyfile, filename, flags, &error))
		return NULL;

	setting = (Setting *)malloc(sizeof(Setting));

	/* general setting */
	setting->fontname = g_key_file_get_string(keyfile, "general", "fontname", NULL);

	return setting;
}
