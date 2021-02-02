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
#include <vte/vte.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "setting.h"


/* Single copy setting*/
Setting * setting;

ColorPreset color_presets[] = {
    {
        .name = "VGA",
        .background_color = "#000000",
        .foreground_color = "#aaaaaa",
        .palette = {
            "#000000", "#aa0000", "#00aa00", "#aa5500",
            "#0000aa", "#aa00aa", "#00aaaa", "#aaaaaa",
            "#555555", "#ff5555", "#55ff55", "#ffff55",
            "#5555ff", "#ff55ff", "#55ffff", "#ffffff"
        }
    },
    {
        .name = "xterm",
        .background_color = "#000000",
        .foreground_color = "#e5e5e5",
        .palette = {
            "#000000", "#cd0000", "#00cd00", "#cdcd00",
            "#0000ee", "#cd00cd", "#00cdcd", "#e5e5e5",
            "#7f7f7f", "#ff0000", "#00ff00", "#ffff00",
            "#5c5cff", "#ff00ff", "#00ffff", "#ffffff"
        }
    },
    {
        .name = "Tango",
        .background_color = "#000000",
        .foreground_color = "#d3d7cf",
        .palette = {
            "#000000", "#cd0000", "#4e9a06", "#c4a000",
            "#3465a4", "#75507b", "#06989a", "#d3d7cf",
            "#555753", "#ef2929", "#8ae234", "#fce94f",
            "#729fcf", "#ad7fa8", "#34e2e2", "#eeeeec"
        }
    },
    {
        .name = "Solarized Dark",
        .background_color = "#002b36",
        .foreground_color = "#839496",
        .palette = {
            "#073642", "#dc322f", "#859900", "#b58900",
            "#268bd2", "#d33682", "#2aa198", "#eee8d5",
            "#002b36", "#cb4b16", "#586e75", "#657b83",
            "#839496", "#6c71c4", "#93a1a1", "#fdf6e3"
        }
    },
    {
        .name = "Solarized Light",
        .background_color = "#fdf6e3",
        .foreground_color = "#657b83",
        .palette = {
            "#073642", "#dc322f", "#859900", "#b58900",
            "#268bd2", "#d33682", "#2aa198", "#eee8d5",
            "#002b36", "#cb4b16", "#586e75", "#657b83",
            "#839496", "#6c71c4", "#93a1a1", "#fdf6e3"
        }
    },
    {
        .name = "Custom"
    }
};

/* Debug print. */
#if 0
void print_setting()
{
    g_return_if_fail (setting != NULL);

    printf("Font name: %s\n", setting->font_name);
#if VTE_CHECK_VERSION (0, 38, 0)
    gchar * p = gdk_rgba_to_string(&setting->background_color);
#else
    gchar * p = gdk_color_to_string(&setting->background_color);
#endif
    printf("Background color: %s\n", p);
    g_free(p);
#if VTE_CHECK_VERSION (0, 38, 0)
    p = gdk_rgba_to_string(&setting->foreground_color);
#else
    printf("Background Alpha: %i\n", setting->background_alpha);
    p = gdk_color_to_string(&setting->foreground_color);
#endif
    printf("Foreground color: %s\n", p);
    g_free(p);
    printf("Disallow bolding by VTE: %i\n", setting->disallow_bold);
#if VTE_CHECK_VERSION (0, 52, 0)
    printf("Bold is bright: %i\n", setting->bold_bright);
#endif
    printf("Cursor blinks: %i\n", setting->cursor_blink);
    printf("Underline blinks: %i\n", setting->cursor_underline);
    printf("Audible bell: %i\n", setting->audible_bell);
    printf("Tab position: %s\n", setting->tab_position);
    printf("Scrollback buffer size in lines: %i\n", setting->scrollback);
    printf("Hide scrollbar: %i\n", setting->hide_scroll_bar);
    printf("Hide menubar: %i\n", setting->hide_menu_bar);
    printf("Hide Close Button: %i\n", setting->hide_close_button);
    printf("Hide mouse pointer: %i\n", setting->hide_pointer);
    printf("Word selection characters: %s\n", setting->word_selection_characters);
    printf("Disable F10: %i\n", setting->disable_f10);
    printf("Disable Alt: %i\n", setting->disable_alt);
    printf("Disable Confirm: %i\n", setting->disable_confirm);
    printf("Geometry change: %i\n", setting->geometry_change);
    
    /* Shortcut group settings. */
    printf("NEW_WINDOW_ACCEL: %s\n", setting->new_window_accel);
    printf("NEW_TAB_ACCEL: %s\n", setting->new_tab_accel);
    printf("CLOSE_TAB_ACCEL: %s\n", setting->close_tab_accel);
    printf("CLOSE_WINDOW_ACCEL: %s\n", setting->close_window_accel);
    printf("COPY_ACCEL: %s\n", setting->copy_accel);
    printf("PASTE_ACCEL: %s\n", setting->paste_accel);
    printf("NAME_TAB_ACCEL: %s\n", setting->name_tab_accel);
    printf("PREVIOUS_TAB_ACCEL: %s\n", setting->previous_tab_accel);
    printf("NEXT_TAB_ACCEL: %s\n", setting->next_tab_accel);
    printf("MOVE_TAB_LEFT_ACCEL: %s\n", setting->move_tab_left_accel);
    printf("MOVE_TAB_RIGHT_ACCEL: %s\n", setting->move_tab_right_accel);
}
#endif

Setting * get_setting()
{
    return setting;
}

void set_setting(Setting * new_setting)
{
    if (setting != NULL)
    {
        free_setting(&setting);
    }
    setting = new_setting;
}

/* Save settings to configuration file. */
void save_setting()
{
    int i;
    g_return_if_fail (setting != NULL);
    //print_setting();
    
    /* Push settings to GKeyFile. */
    g_key_file_set_string(setting->keyfile, GENERAL_GROUP, FONT_NAME, setting->font_name);
#if VTE_CHECK_VERSION (0, 38, 0)
    gchar * p = gdk_rgba_to_string(&setting->background_color);
#else
    gchar * p = gdk_color_to_string(&setting->background_color);
#endif
    if (p != NULL)
        g_key_file_set_string(setting->keyfile, GENERAL_GROUP, BG_COLOR, p);
    g_free(p);
#if VTE_CHECK_VERSION (0, 38, 0)
    p = gdk_rgba_to_string(&setting->foreground_color);
#else
    g_key_file_set_integer(setting->keyfile, GENERAL_GROUP, BG_ALPHA, setting->background_alpha);
    p = gdk_color_to_string(&setting->foreground_color);
#endif
    if (p != NULL)
        g_key_file_set_string(setting->keyfile, GENERAL_GROUP, FG_COLOR, p);

    /* Save color palette */
    for (i=0; i<16; i++) {
        gchar *palette_color_key = g_strdup_printf(PALETTE_COLOR_PREFIX "%d", i);
#if VTE_CHECK_VERSION (0, 38, 0)
	p = gdk_rgba_to_string(&setting->palette_color[i]);
#else
	p = gdk_color_to_string(&setting->palette_color[i]);
#endif
	if (p != NULL)
	    g_key_file_set_string(setting->keyfile, GENERAL_GROUP, palette_color_key, p);
    }
    g_key_file_set_string(setting->keyfile, GENERAL_GROUP, COLOR_PRESET, setting->color_preset);

    g_free(p);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, DISALLOW_BOLD, setting->disallow_bold);
#if VTE_CHECK_VERSION (0, 52, 0)
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, BOLD_BRIGHT, setting->bold_bright);
#endif
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, CURSOR_BLINKS, setting->cursor_blink);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, CURSOR_UNDERLINE, setting->cursor_underline);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, AUDIBLE_BELL, setting->audible_bell);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, VISUAL_BELL, setting->visual_bell);
    g_key_file_set_string(setting->keyfile, GENERAL_GROUP, TAB_POS, setting->tab_position);
    g_key_file_set_integer(setting->keyfile, GENERAL_GROUP, SCROLLBACK, setting->scrollback);
    g_key_file_set_integer(setting->keyfile, GENERAL_GROUP, GEOMETRY_COLUMNS, setting->geometry_columns);
    g_key_file_set_integer(setting->keyfile, GENERAL_GROUP, GEOMETRY_ROWS, setting->geometry_rows);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, HIDE_SCROLLBAR, setting->hide_scroll_bar);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, HIDE_MENUBAR, setting->hide_menu_bar);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, HIDE_CLOSE_BUTTON, setting->hide_close_button);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, HIDE_POINTER, setting->hide_pointer);
    g_key_file_set_string(setting->keyfile, GENERAL_GROUP, SEL_CHARS, setting->word_selection_characters);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, DISABLE_F10, setting->disable_f10);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, DISABLE_ALT, setting->disable_alt);
    g_key_file_set_boolean(setting->keyfile, GENERAL_GROUP, DISABLE_CONFIRM, setting->disable_confirm);

    /* Shortcut group settings. */
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, NEW_WINDOW_ACCEL, setting->new_window_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, NEW_TAB_ACCEL, setting->new_tab_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, CLOSE_TAB_ACCEL, setting->close_tab_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, CLOSE_WINDOW_ACCEL, setting->close_window_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, COPY_ACCEL, setting->copy_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, PASTE_ACCEL, setting->paste_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, NAME_TAB_ACCEL, setting->name_tab_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, PREVIOUS_TAB_ACCEL, setting->previous_tab_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, NEXT_TAB_ACCEL, setting->next_tab_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, MOVE_TAB_LEFT_ACCEL, setting->move_tab_left_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, MOVE_TAB_RIGHT_ACCEL, setting->move_tab_right_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, ZOOM_IN_ACCEL, setting->zoom_in_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, ZOOM_OUT_ACCEL, setting->zoom_out_accel);
    g_key_file_set_string(setting->keyfile, SHORTCUT_GROUP, ZOOM_RESET_ACCEL, setting->zoom_reset_accel);

    /* Convert GKeyFile to text and build path to configuration file. */
    gchar * file_data = g_key_file_to_data(setting->keyfile, NULL, NULL);
    gchar * config_path = g_build_filename(g_get_user_config_dir(), "lxterminal/lxterminal.conf", NULL);

    if ((file_data != NULL) && (config_path != NULL))
    {
        /* Create the file if necessary. */
        int fd = open(config_path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd < 0)
        {
            g_warning("Configuration file create failed: %s\n", g_strerror(errno));
        }
        else
        {
            if(write(fd, file_data, strlen(file_data)) < 0)
	         g_warning("Configuration file write failed: %s\n", g_strerror(errno));
            close(fd);
        }
    }

    /* Deallocate memory. */
    g_free(file_data);
    g_free(config_path);
}

/* Deep copy settings. */
Setting * copy_setting(Setting * setting)
{
    g_return_val_if_fail (setting != NULL, NULL);

    /* Allocate structure. */
    Setting * new_setting = g_slice_new0(Setting);
    memcpy(new_setting, setting, sizeof(Setting));

    new_setting->font_name = g_strdup(setting->font_name);
    new_setting->tab_position = g_strdup(setting->tab_position);
    new_setting->word_selection_characters = g_strdup(setting->word_selection_characters);
    
    /* Shortcut group settings. */
    new_setting->new_window_accel = g_strdup(setting->new_window_accel);
    new_setting->new_tab_accel = g_strdup(setting->new_tab_accel);
    new_setting->close_tab_accel = g_strdup(setting->close_tab_accel);
    new_setting->close_window_accel = g_strdup(setting->close_window_accel);
    new_setting->copy_accel = g_strdup(setting->copy_accel);
    new_setting->paste_accel = g_strdup(setting->paste_accel);
    new_setting->name_tab_accel = g_strdup(setting->name_tab_accel);
    new_setting->previous_tab_accel = g_strdup(setting->previous_tab_accel);
    new_setting->next_tab_accel = g_strdup(setting->next_tab_accel);
    new_setting->move_tab_left_accel = g_strdup(setting->move_tab_left_accel);
    new_setting->move_tab_right_accel = g_strdup(setting->move_tab_right_accel);
    
    return new_setting;
}

/* Deep free settings. */
void free_setting(Setting ** setting)
{
    Setting *_setting;
    g_return_if_fail (setting != NULL && *setting != NULL);

    _setting = *setting;

    g_free(_setting->font_name);
    g_free(_setting->tab_position);
    g_free(_setting->word_selection_characters);
    g_free(_setting->new_window_accel);
    g_free(_setting->new_tab_accel);
    g_free(_setting->close_tab_accel);
    g_free(_setting->close_window_accel);
    g_free(_setting->copy_accel);
    g_free(_setting->paste_accel);
    g_free(_setting->name_tab_accel);
    g_free(_setting->previous_tab_accel);
    g_free(_setting->next_tab_accel);
    g_free(_setting->move_tab_left_accel);
    g_free(_setting->move_tab_right_accel);

    g_slice_free(Setting, _setting);
    *setting = NULL;
}

/* Load settings from configuration file. */
Setting * load_setting()
{
    int i;
    gchar * dir = g_build_filename(g_get_user_config_dir(), "lxterminal" , NULL);
    g_mkdir_with_parents(dir, S_IRUSR | S_IWUSR | S_IXUSR);
    gchar * user_config_path = g_build_filename(dir, "lxterminal.conf", NULL);
    g_free(dir);
    gchar * system_config_path = g_strdup(PACKAGE_DATA_DIR "/lxterminal/lxterminal.conf");
    gchar * config_path = user_config_path;
    
    gboolean need_save = FALSE;

    if ( ! g_file_test(user_config_path, G_FILE_TEST_EXISTS))
    {
        /* Load system-wide settings. */
        config_path = system_config_path;
        need_save = TRUE;
    }

    /* Allocate structure. */
    setting = g_slice_new0(Setting);

    /* Initialize nonzero integer values to defaults. */
#if VTE_CHECK_VERSION (0, 38, 0)
    setting->background_color.alpha = setting->foreground_color.alpha = 1;
    setting->foreground_color.red = setting->foreground_color.green = setting->foreground_color.blue = (gdouble) 170/255;
#else
    setting->background_alpha = 65535;
    setting->foreground_color.red = setting->foreground_color.green = setting->foreground_color.blue = 0xaaaa;
#endif

    /* Load configuration. */
    setting->keyfile = g_key_file_new();
    GError * error = NULL;
    if ((g_file_test(config_path, G_FILE_TEST_EXISTS))
    && (g_key_file_load_from_file(setting->keyfile, config_path, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error)))
    {
        setting->font_name = g_key_file_get_string(setting->keyfile, GENERAL_GROUP, FONT_NAME, NULL);
        char * p = g_key_file_get_string(setting->keyfile, GENERAL_GROUP, BG_COLOR, NULL);
        if (p != NULL)
        {
#if VTE_CHECK_VERSION (0, 38, 0)
            gdk_rgba_parse(&setting->background_color, p);
#else
            gdk_color_parse(p, &setting->background_color);
        }
        setting->background_alpha = g_key_file_get_integer(setting->keyfile, GENERAL_GROUP, BG_ALPHA, &error);
        if (error && (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND))
        {   
            /* Set default value if key not found! */
            setting->background_alpha = 65535;
#endif
        }
        p = g_key_file_get_string(setting->keyfile, GENERAL_GROUP, FG_COLOR, NULL);
        if (p != NULL)
        {
#if VTE_CHECK_VERSION (0, 38, 0)
            gdk_rgba_parse(&setting->foreground_color, p);
#else
            gdk_color_parse(p, &setting->foreground_color);
#endif
        }

        setting->color_preset = g_key_file_get_string(setting->keyfile, GENERAL_GROUP, COLOR_PRESET, NULL);
        if (setting->color_preset) {
            for (i=0; i<16; i++) {
                gchar *palette_color_key = g_strdup_printf(PALETTE_COLOR_PREFIX "%d", i);
                p = g_key_file_get_string(setting->keyfile, GENERAL_GROUP, palette_color_key, NULL);
                if (p != NULL) {
#if VTE_CHECK_VERSION (0, 38, 0)
                    gdk_rgba_parse(&setting->palette_color[i], p);
#else
                    gdk_color_parse(p, &setting->palette_color[i]);
#endif
                } else {
                    goto color_preset_does_not_exist;
                }
            }
        } else {
color_preset_does_not_exist:
            setting->color_preset = color_presets[0].name;
            for (i=0; i<16; i++) {
#if VTE_CHECK_VERSION (0, 38, 0)
                gdk_rgba_parse(&setting->palette_color[i], color_presets[0].palette[i]);
#else
                gdk_color_parse(color_presets[0].palette[i], &setting->palette_color[i]);
#endif
            }
        }

        setting->disallow_bold = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, DISALLOW_BOLD, NULL);
#if VTE_CHECK_VERSION (0, 52, 0)
        setting->bold_bright = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, BOLD_BRIGHT, NULL);
#endif
        setting->cursor_blink = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, CURSOR_BLINKS, NULL);
        setting->cursor_underline = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, CURSOR_UNDERLINE, NULL);
        setting->audible_bell = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, AUDIBLE_BELL, NULL);
        setting->visual_bell = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, VISUAL_BELL, NULL);
        setting->tab_position = g_key_file_get_string(setting->keyfile, GENERAL_GROUP, TAB_POS, NULL);
        setting->scrollback = g_key_file_get_integer(setting->keyfile, GENERAL_GROUP, SCROLLBACK, &error);
        if (error && (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND))
        {   
            /* Set default value if key not found! */
            setting->scrollback = 1000;
        }
        g_clear_error(&error);
        setting->geometry_columns = g_key_file_get_integer(setting->keyfile, GENERAL_GROUP, GEOMETRY_COLUMNS, &error);
        if (error && (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
            setting->geometry_columns = 80;
        }
        g_clear_error(&error);
        setting->geometry_rows = g_key_file_get_integer(setting->keyfile, GENERAL_GROUP, GEOMETRY_ROWS, &error);
        if (error && (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
            setting->geometry_rows = 24;
        }
        g_clear_error(&error);
        setting->hide_scroll_bar = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, HIDE_SCROLLBAR, NULL);
        setting->hide_menu_bar = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, HIDE_MENUBAR, NULL);
        setting->hide_close_button = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, HIDE_CLOSE_BUTTON, NULL);
        setting->hide_pointer = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, HIDE_POINTER, NULL);
        setting->word_selection_characters = g_key_file_get_string(setting->keyfile, GENERAL_GROUP, SEL_CHARS, NULL);
        setting->disable_f10 = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, DISABLE_F10, NULL);
        setting->disable_alt = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, DISABLE_ALT, NULL);
        setting->disable_confirm = g_key_file_get_boolean(setting->keyfile, GENERAL_GROUP, DISABLE_CONFIRM, NULL);
        
        /* Shortcut group settings. */
        setting->new_window_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, NEW_WINDOW_ACCEL, NULL);
        setting->new_tab_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, NEW_TAB_ACCEL, NULL);
        setting->close_tab_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, CLOSE_TAB_ACCEL, NULL);
        setting->close_window_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, CLOSE_WINDOW_ACCEL, NULL);
        setting->copy_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, COPY_ACCEL, NULL);
        setting->paste_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, PASTE_ACCEL, NULL);
        setting->name_tab_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, NAME_TAB_ACCEL, NULL);
        setting->previous_tab_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, PREVIOUS_TAB_ACCEL, NULL);
        setting->next_tab_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, NEXT_TAB_ACCEL, NULL);
        setting->move_tab_left_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, MOVE_TAB_LEFT_ACCEL, NULL);
        setting->move_tab_right_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, MOVE_TAB_RIGHT_ACCEL, NULL);
        setting->zoom_in_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, ZOOM_IN_ACCEL, NULL);
        setting->zoom_out_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, ZOOM_OUT_ACCEL, NULL);
        setting->zoom_reset_accel = g_key_file_get_string(setting->keyfile, SHORTCUT_GROUP, ZOOM_RESET_ACCEL, NULL);
    }
    g_free(system_config_path);
    g_free(user_config_path);

    /* Default configuration strings. */
    if (setting->font_name == NULL)
    {
        setting->font_name = g_strdup("monospace 10");
    }
    if (setting->tab_position == NULL)
    {
        setting->tab_position = g_strdup("top");
    }
    if (setting->word_selection_characters == NULL)
    {
        setting->word_selection_characters = g_strdup("-A-Za-z0-9,./?%&#:_~");
    }
    
    /* Default configuration for shortcut group settings. */
    if (setting->new_window_accel == NULL)
    {
        setting->new_window_accel = g_strdup(NEW_WINDOW_ACCEL_DEF);
    }
    if (setting->new_tab_accel == NULL)
    {
        setting->new_tab_accel = g_strdup(NEW_TAB_ACCEL_DEF);
    }
    if (setting->close_tab_accel == NULL)
    {
        setting->close_tab_accel = g_strdup(CLOSE_TAB_ACCEL_DEF);
    }
    if (setting->close_window_accel == NULL)
    {
        setting->close_window_accel = g_strdup(CLOSE_WINDOW_ACCEL_DEF);
    }
    if (setting->copy_accel == NULL)
    {
        setting->copy_accel = g_strdup(COPY_ACCEL_DEF);
    }
    if (setting->paste_accel == NULL)
    {
        setting->paste_accel = g_strdup(PASTE_ACCEL_DEF);
    }
    if (setting->name_tab_accel == NULL)
    {
        setting->name_tab_accel = g_strdup(NAME_TAB_ACCEL_DEF);
    }
    if (setting->previous_tab_accel == NULL)
    {
        setting->previous_tab_accel = g_strdup(PREVIOUS_TAB_ACCEL_DEF);
    }
    if (setting->next_tab_accel == NULL)
    {
        setting->next_tab_accel = g_strdup(NEXT_TAB_ACCEL_DEF);
    }
    if (setting->move_tab_left_accel == NULL)
    {
        setting->move_tab_left_accel = g_strdup(MOVE_TAB_LEFT_ACCEL_DEF);
    }
    if (setting->move_tab_right_accel == NULL)
    {
        setting->move_tab_right_accel = g_strdup(MOVE_TAB_RIGHT_ACCEL_DEF);
    }

    if (setting->zoom_in_accel == NULL)
    {
        setting->zoom_in_accel = g_strdup(ZOOM_IN_ACCEL_DEF);
    }

    if (setting->zoom_out_accel == NULL)
    {
        setting->zoom_out_accel = g_strdup(ZOOM_OUT_ACCEL_DEF);
    }

    if (setting->zoom_reset_accel == NULL)
    {
        setting->zoom_reset_accel = g_strdup(ZOOM_RESET_ACCEL_DEF);
    }

    if (need_save)
    {
        save_setting();
    }
    //print_setting();
    return setting;
}

