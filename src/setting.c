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
    g_key_file_set_string(setting->keyfile, "Style", "Font", setting->font_name);

    g_key_file_set_boolean(setting->keyfile, "Style", "AllowBold", setting->allow_bold);

    g_key_file_set_boolean(setting->keyfile, "Style", "CursorBlinks", setting->cursor_blink);

    g_key_file_set_string(setting->keyfile, "Style", "CursorShape", setting->cursor_shape);

    g_key_file_set_boolean(setting->keyfile, "Style", "AudibleBell", setting->audible_bell);

    gchar * p = gdk_color_to_string(&setting->background_color);

    if (p != NULL) {
        g_key_file_set_string(setting->keyfile, "Palette", "BackgroundColor", p);
        g_free(p);
    }

    g_key_file_set_integer(setting->keyfile, "Palette", "BackgroundAlpha", setting->background_alpha);

    p = gdk_color_to_string(&setting->foreground_color);
    if (p != NULL) {
        g_key_file_set_string(setting->keyfile, "Palette", "ForegroundColor", p);
        g_free(p);
    }

    g_key_file_set_string(setting->keyfile, "Display", "TabPosition", setting->tab_position);

    g_key_file_set_integer(setting->keyfile, "Display", "ScrollBack", setting->scrollback);

    g_key_file_set_boolean(setting->keyfile, "Display", "HideScrollbar", setting->hide_scroll_bar);

    g_key_file_set_boolean(setting->keyfile, "Display", "HideMenubar", setting->hide_menu_bar);

    g_key_file_set_boolean(setting->keyfile, "Display", "HideCloseButton", setting->hide_close_button);

    g_key_file_set_string(setting->keyfile, "Advanced", "SelectChars", setting->word_selection_characters);

    g_key_file_set_boolean(setting->keyfile, "Advanced", "DisableF10", setting->disable_f10);

    g_key_file_set_boolean(setting->keyfile, "Advanced", "DisableAlt", setting->disable_alt);

    /* Convert GKeyFile to text and build path to configuration file. */
    gchar * file_data = g_key_file_to_data(setting->keyfile, NULL, NULL);
    gchar * path = g_build_filename(g_get_user_config_dir(), "lxterminal/lxterminal.conf", NULL);

    if ((file_data != NULL) && (path != NULL))
    {
        /* Create the file if necessary. */
        int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
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
    gboolean hasError = FALSE;

    /* Allocate structure. */
    Setting * setting = g_new0(Setting, 1);

    /* Load configuration. */
    setting->keyfile = g_key_file_new();
    GError * error = NULL;
    g_key_file_load_from_file(setting->keyfile, filename, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error);
    if (error != NULL) hasError = TRUE;
    g_clear_error(&error);

    if (hasError == FALSE) {
        char * p;

        if (setting->font_name != NULL) {
            g_free(setting->font_name);
        }
        setting->font_name = g_key_file_get_string(setting->keyfile, "Style", "Font", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->allow_bold = g_key_file_get_boolean(setting->keyfile, "Style", "AllowBold", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->cursor_blink = g_key_file_get_boolean(setting->keyfile, "Style", "CursorBlinks", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        if (setting->cursor_shape != NULL) {
            g_free(setting->cursor_shape);
        }
        setting->cursor_shape = g_key_file_get_string(setting->keyfile, "Style", "CursorShape", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->audible_bell = g_key_file_get_boolean(setting->keyfile, "Style", "AudibleBell", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        p = g_key_file_get_string(setting->keyfile, "Palette", "BackgroundColor", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);
        if (p != NULL) {
            gdk_color_parse(p, &setting->background_color);
            g_free(p);
        }

        setting->background_alpha = g_key_file_get_integer(setting->keyfile, "Palette", "BackgroundAlpha", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        p = g_key_file_get_string(setting->keyfile, "Palette", "ForegroundColor", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);
        if (p != NULL) {
            gdk_color_parse(p, &setting->foreground_color);
            g_free(p);
        }

        if (setting->tab_position != NULL) {
            g_free(setting->tab_position);
        }
        setting->tab_position = g_key_file_get_string(setting->keyfile, "Display", "TabPosition", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->scrollback = g_key_file_get_integer(setting->keyfile, "Display", "ScrollBack", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->hide_scroll_bar = g_key_file_get_boolean(setting->keyfile, "Display", "HideScrollbar", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->hide_menu_bar = g_key_file_get_boolean(setting->keyfile, "Display", "HideMenubar", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->hide_close_button = g_key_file_get_boolean(setting->keyfile, "Display", "HideCloseButton", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        if (setting->word_selection_characters != NULL) {
            g_free(setting->word_selection_characters);
        }
        setting->word_selection_characters = g_key_file_get_string(setting->keyfile, "Advanced", "SelectChars", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);
        
        setting->disable_f10 = g_key_file_get_boolean(setting->keyfile, "Advanced", "DisableF10", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);

        setting->disable_alt = g_key_file_get_boolean(setting->keyfile, "Advanced", "DisableAlt", &error);
        if (error != NULL) hasError = TRUE;
        g_clear_error(&error);
    }

    if (hasError == TRUE) {
        setting->font_name = g_strdup("Monospace 12");
        setting->allow_bold = TRUE;
        setting->cursor_blink = TRUE;
        setting->cursor_shape = g_strdup("block");
        setting->audible_bell = FALSE;
        setting->background_color.red = setting->background_color.green = setting->background_color.blue = 0x0000;
        setting->background_alpha = 65535;
        setting->foreground_color.red = setting->foreground_color.green = setting->foreground_color.blue = 0xaaaa;
        setting->tab_position = g_strdup("top");
        setting->scrollback = 1000;
        setting->hide_scroll_bar = FALSE;
        setting->hide_menu_bar = FALSE;
        setting->hide_close_button = FALSE;
        setting->word_selection_characters = g_strdup("-A-Za-z0-9,./?%&#:_~");
        setting->disable_f10 = FALSE;
        setting->disable_alt = FALSE;
    }
    return setting;
}
