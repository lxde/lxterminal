/**
 *      Copyright 2008 Fred Chien <cfsghost@gmail.com>
 *      Copyright (c) 2010 LxDE Developers, see the file AUTHORS for details.
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
#include <glib/gstdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "setting.h"

/* Save settings to configuration file. */
void setting_save(Setting * setting)
{
    /* Push settings to GKeyFile. */
    g_key_file_set_string(setting->keyfile, "general", "fontname", setting->font_name);
    gchar * p = gdk_color_to_string(&setting->background_color);
    if (p != NULL)
        g_key_file_set_string(setting->keyfile, "general", "bgcolor", p);
    g_free(p);
    g_key_file_set_integer(setting->keyfile, "general", "bgalpha", setting->background_alpha);
    p = gdk_color_to_string(&setting->foreground_color);
    if (p != NULL)
        g_key_file_set_string(setting->keyfile, "general", "fgcolor", p);
    g_free(p);
    g_key_file_set_boolean(setting->keyfile, "general", "disallowbold", setting->disallow_bold);
    g_key_file_set_boolean(setting->keyfile, "general", "cursorblinks", setting->cursor_blink);
    g_key_file_set_boolean(setting->keyfile, "general", "cursorunderline", setting->cursor_underline);
    g_key_file_set_boolean(setting->keyfile, "general", "audiblebell", setting->audible_bell);
    g_key_file_set_string(setting->keyfile, "general", "tabpos", setting->tab_position);
    g_key_file_set_integer(setting->keyfile, "general", "scrollback", setting->scrollback);
    g_key_file_set_boolean(setting->keyfile, "general", "hidescrollbar", setting->hide_scroll_bar);
    g_key_file_set_boolean(setting->keyfile, "general", "hidemenubar", setting->hide_menu_bar);
    g_key_file_set_boolean(setting->keyfile, "general", "hideclosebutton", setting->hide_close_button);
    g_key_file_set_string(setting->keyfile, "general", "selchars", setting->word_selection_characters);
    g_key_file_set_boolean(setting->keyfile, "general", "disablef10", setting->disable_f10);

    /* Convert GKeyFile to text and build path to configuration file. */
    gchar * file_data = g_key_file_to_data(setting->keyfile, NULL, NULL);
    gchar * path = g_build_filename(g_get_user_config_dir(), "lxterminal/lxterminal.conf", NULL);

    if ((file_data != NULL) && (path != NULL))
    {
        /* Create the file if necessary. */
        int fd = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if (fd < 0)
            g_warning("Configuration file create failed: %s\n", g_strerror(errno));
        else
        {
            write(fd, file_data, strlen(file_data));
            close(fd);
        }
    }

    /* Deallocate memory. */
    g_free(file_data);
    g_free(path);
}

/* Load settings from configuration file. */
Setting * load_setting_from_file(const char * filename)
{
    /* Allocate structure. */
    Setting * setting = g_new0(Setting, 1);

    /* Initialize nonzero integer values to defaults. */
    setting->background_alpha = 65535;
    setting->foreground_color.red = setting->foreground_color.green = setting->foreground_color.blue = 0xaaaa;

    /* Load configuration. */
    setting->keyfile = g_key_file_new();
    GError * error = NULL;
    if ((g_file_test(filename, G_FILE_TEST_EXISTS))
    && (g_key_file_load_from_file(setting->keyfile, filename, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error)))
    {
        setting->font_name = g_key_file_get_string(setting->keyfile, "general", "fontname", NULL);
        char * p = g_key_file_get_string(setting->keyfile, "general", "bgcolor", NULL);
        if (p != NULL)
            gdk_color_parse(p, &setting->background_color);
        setting->background_alpha = g_key_file_get_integer(setting->keyfile, "general", "bgalpha", NULL);
        p = g_key_file_get_string(setting->keyfile, "general", "fgcolor", NULL);
        if (p != NULL)
            gdk_color_parse(p, &setting->foreground_color);
        setting->disallow_bold = g_key_file_get_boolean(setting->keyfile, "general", "disallowbold", NULL);
        setting->cursor_blink = g_key_file_get_boolean(setting->keyfile, "general", "cursorblinks", NULL);
        setting->cursor_underline = g_key_file_get_boolean(setting->keyfile, "general", "cursorunderline", NULL);
        setting->audible_bell = g_key_file_get_boolean(setting->keyfile, "general", "audiblebell", NULL);
        setting->tab_position = g_key_file_get_string(setting->keyfile, "general", "tabpos", NULL);
        setting->scrollback = g_key_file_get_integer(setting->keyfile, "general", "scrollback", NULL);
        setting->hide_scroll_bar = g_key_file_get_boolean(setting->keyfile, "general", "hidescrollbar", NULL);
        setting->hide_menu_bar = g_key_file_get_boolean(setting->keyfile, "general", "hidemenubar", NULL);
        setting->hide_close_button = g_key_file_get_boolean(setting->keyfile, "general", "hideclosebutton", NULL);
        setting->word_selection_characters = g_key_file_get_string(setting->keyfile, "general", "selchars", NULL);
        setting->disable_f10 = g_key_file_get_boolean(setting->keyfile, "general", "disablef10", NULL);
    }

    /* Default configuration strings. */
    if (setting->font_name == NULL)
        setting->font_name = g_strdup("monospace 10");
    if (setting->tab_position == NULL)
        setting->tab_position = g_strdup("top");
    if (setting->word_selection_characters == NULL)
        setting->word_selection_characters = g_strdup("-A-Za-z0-9,./?%&#:_");
    return setting;
}
