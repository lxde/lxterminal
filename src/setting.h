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

#ifndef LXTERMINAL_SETTING_H
#define LXTERMINAL_SETTING_H

#include <gtk/gtk.h>

/* User preferences. */
typedef struct _setting {

    GKeyFile * keyfile;			/* Pointer to GKeyFile containing settings */
    char * font_name;			/* Font name */
    GdkColor background_color;		/* Background color */
    guint16 background_alpha;		/* Alpha value to go with background color */
    GdkColor foreground_color;		/* Foreground color */
    gboolean disallow_bold;		/* Disallow bolding by VTE */
    gboolean cursor_blink;		/* True if cursor blinks */
    gboolean cursor_underline;		/* True if underline cursor; false if block cursor */
    gboolean audible_bell;		/* True if audible bell */
    char * tab_position;		/* Position of tabs on main window (top, bottom, left, right) */
    gint scrollback;			/* Scrollback buffer size in lines */
    gboolean hide_scroll_bar;		/* True if scroll bar is NOT visible */
    gboolean hide_menu_bar;		/* True if menu bar is NOT visible */
    gboolean hide_close_button;		/* True if close buttons are NOT visible */
    char * word_selection_characters;	/* Characters that act as word breaks during selection by word */
    gboolean disable_f10;		/* True if F10 will be passed to program; false if it brings up File menu */
    gboolean disable_alt;		/* True if Alt-n is passed to shell; false if it is used to switch between tabs */

    gboolean geometry_change;		/* True if there is a geometry change, until it has been acted on */

} Setting;

extern void setting_save(Setting * setting);
extern Setting * load_setting_from_file(const char * filename);

#endif
